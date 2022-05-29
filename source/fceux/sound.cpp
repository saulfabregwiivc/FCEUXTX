/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "fceu-types.h"
#include "x6502.h"

#include "fceu.h"
#include "sound.h"
#include "filter.h"
#include "state.h"
int nwram;
static uint32 wlookup1[32];
static uint32 wlookup2[203];

int32 Wave[2048 + 512];
int32 WaveHi[40000];
int32 WaveFinal[2048 + 512];

EXPSOUND GameExpSound = { 0, 0, 0 };

static uint8 TriCount = 0;
static uint8 TriMode = 0;

static int32 tristep = 0;

static int32 wlcount[4] = { 0, 0, 0, 0 };	/* Wave length counters.	*/

static uint8 IRQFrameMode = 0;				/* $4017 / xx000000 */
static uint8 PSG[0x10];
static uint8 RawDALatch = 0;				/* $4011 0xxxxxxx */

void(*sfun2[3]) (void);
uint8 vpsg12[8];
uint8 vpsg22[4];
int32 cvbc2[3];
int32 vcount2[3];
int32 dcount2[2];

uint8 EnabledChannels = 0;					/* Byte written to $4015 */

typedef struct {
	uint8 Speed;
	uint8 Mode;	/* Fixed volume(1), and loop(2) */
	uint8 DecCountTo1;
	uint8 decvolume;
	int reloaddec;
} ENVUNIT;

unsigned DMC_7bit = 0; /* used to skip overclocking */
static ENVUNIT EnvUnits[3];

static const int RectDuties[4] = { 1, 2, 4, 6 };

static int32 RectDutyCount[2];
static uint8 sweepon[2];
static int32 curfreq[2];
static uint8 SweepCount[2];

static uint16 nreg = 0;

static uint8 fcnt = 0;
static int32 fhcnt = 0;
static int32 fhinc = 0;

uint32 soundtsoffs = 0;

/* Variables exclusively for low-quality sound. */
int32 nesincsize = 0;
uint32 soundtsinc = 0;
uint32 soundtsi = 0;
static int32 sqacc[2];
/* LQ variables segment ends. */

static int32 lengthcount[4];
static const uint8 lengthtable[0x20]=
{
	10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};


static const uint32 NTSCNoiseFreqTable[0x10] =
{
	4, 8, 16, 32, 64, 96, 128, 160, 202,
	254, 380, 508, 762, 1016, 2034, 4068
};

static const uint32 PALNoiseFreqTable[0x10] =
{
	4, 7, 14, 30, 60, 88, 118, 148, 188,
	236, 354, 472, 708,  944, 1890, 3778
};


static const uint32 NTSCDMCTable[0x10]=
{
 428,380,340,320,286,254,226,214,
 190,160,142,128,106, 84 ,72,54
};

/* Previous values for PAL DMC was value - 1,
 * I am not certain if this is if FCEU handled
 * PAL differently or not, the NTSC values are right,
 * so I am assuming that the current value is handled
 * the same way NTSC is handled. */

static const uint32 PALDMCTable[0x10]=
{
	398, 354, 316, 298, 276, 236, 210, 198,
	176, 148, 132, 118,  98,  78,  66,  50
};


/* $4010  -  Frequency
 * $4011  -  Actual data outputted
 * $4012  -  Address register: $c000 + V*64
 * $4013  -  Size register:  Size in bytes = (V+1)*64
 */

/*static*/ int32 DMCacc = 1;
static int32 DMCPeriod = 0;
/*static*/ uint8 DMCBitCount = 0;

static uint8 DMCAddressLatch = 0, DMCSizeLatch = 0;	/* writes to 4012 and 4013 */
static uint8 DMCFormat = 0;							/* Write to $4010 */

static uint32 DMCAddress = 0;
static int32 DMCSize = 0;
static uint8 DMCShift = 0;
static uint8 SIRQStat = 0;

static char DMCHaveDMA = 0;
static uint8 DMCDMABuf = 0;
static char DMCHaveSample = 0;



int32 DACacc = 1;
static uint32 DACPeriod;
static uint8 DACSizeLo, DACSizeHi;
uint16 DACSize;
uint8 DACAddressLo, DACAddressHi;
uint8 DACFreqLo, DACFreqHi;
uint16 DACFreq;
uint16 DACAddress; /* writes to 4012 and 4013 */
uint8 DACRam[0x10000];
static uint8 DACFormat;	/* Write to $4010 */
static uint8 DACHaveDMA;
static uint8 DACDMABuf = 0;
static uint8 DACHaveSample;

static void Dummyfunc(void) { };
static void (*DoNoise)(void) = Dummyfunc;
static void (*DoTriangle)(void) = Dummyfunc;
static void (*DoPCM)(void) = Dummyfunc;
static void (*DoSQ1)(void) = Dummyfunc;
static void (*DoSQ2)(void) = Dummyfunc;

static uint32 ChannelBC[5];

static void LoadDMCPeriod(uint8 V) {
	if (PAL)
		DMCPeriod = PALDMCTable[V];
	else
		DMCPeriod = NTSCDMCTable[V];
}
static void LoadDACPeriod(uint8 V)
{

	if (PAL)
		DACPeriod = PALDMCTable[V];
	else
		DACPeriod = NTSCDMCTable[V];

	DACFreq = DACFreqLo + (DACFreqHi << 8);
	if (DACFreq)
		DACPeriod = 1789773 / DACFreq;
}
static void PrepDPCM() {
	DMCAddress = 0x4000 + (DMCAddressLatch << 6);
	 if(nwram) DMCAddress += 0x1000;
	DMCSize = (DMCSizeLatch << 4) + 1;
}
static void PrepDAC()
{
	DACAddress = DACAddressLo + (DACAddressHi << 8);

	DACSize = DACSizeLo + (DACSizeHi << 8);
	for (int x = DACAddress; x < (DACAddress + DACSize); x++)
		DACRam[x] = X6502_DMR2(x, 0);
	//printf("Prepare DAC, Address: %04x, Size: %04x\n", DACAddress, DACSize);
}
/* Instantaneous?  Maybe the new freq value is being calculated all of the time... */

static int FASTAPASS(2) CheckFreq(uint32 cf, uint8 sr) {
	uint32 mod;
	if (!(sr & 0x8)) {
		mod = cf >> (sr & 7);
		if ((mod + cf) & 0x800)
			return(0);
	}
	return(1);
}

static void SQReload(int x, uint8 V) {
	if (EnabledChannels & (1 << x)) {
		if (x)
			DoSQ2();
		else
			DoSQ1();
		lengthcount[x] = lengthtable[(V >> 3) & 0x1f];
	}

	sweepon[x] = PSG[(x << 2) | 1] & 0x80;
	curfreq[x] = PSG[(x << 2) | 0x2] | ((V & 7) << 8);
	SweepCount[x] = ((PSG[(x << 2) | 0x1] >> 4) & 7) + 1;

	RectDutyCount[x] = 7;
	EnvUnits[x].reloaddec = 1;
	/* reloadfreq[x]=1; */
}

static DECLFW(Write_PSG) {
/* FCEU_printf("APU1 %04x:%04x\n",A,V); */
	A &= 0x1F;
	switch (A) {
	case 0x0:
		DoSQ1();
		EnvUnits[0].Mode = (V & 0x30) >> 4;
		EnvUnits[0].Speed = (V & 0xF);
		if (swapDuty)
			V = (V & 0x3F) | ((V & 0x80) >> 1) | ((V & 0x40) << 1);
		break;
	case 0x1:
		sweepon[0] = V & 0x80;
		break;
	case 0x2:
		DoSQ1();
		curfreq[0] &= 0xFF00;
		curfreq[0] |= V;
		break;
	case 0x3:
		SQReload(0, V);
		break;
	case 0x4:
		DoSQ2();
		EnvUnits[1].Mode = (V & 0x30) >> 4;
		EnvUnits[1].Speed = (V & 0xF);
		if (swapDuty)
			V = (V & 0x3F) | ((V & 0x80) >> 1) | ((V & 0x40) << 1);
		break;
	case 0x5:
		sweepon[1] = V & 0x80;
		break;
	case 0x6:
		DoSQ2();
		curfreq[1] &= 0xFF00;
		curfreq[1] |= V;
		break;
	case 0x7:
		SQReload(1, V);
		break;
	case 0xa:
		DoTriangle();
		break;
	case 0xb:
		DoTriangle();
		if (EnabledChannels & 0x4)
			lengthcount[2] = lengthtable[(V >> 3) & 0x1f];
		TriMode = 1;	/* Load mode */
		break;
	case 0xC:
		DoNoise();
		EnvUnits[2].Mode = (V & 0x30) >> 4;
		EnvUnits[2].Speed = (V & 0xF);
		break;
	case 0xE:
		DoNoise();
		break;
	case 0xF:
		DoNoise();
		if (EnabledChannels & 0x8)
			lengthcount[3] = lengthtable[(V >> 3) & 0x1f];
		EnvUnits[2].reloaddec = 1;
		break;
	case 0x10:
		DoPCM();
		LoadDMCPeriod(V & 0xF);

		if (SIRQStat & 0x80) {
			if (!(V & 0x80)) {
				X6502_IRQEnd(FCEU_IQDPCM);
				SIRQStat &= ~0x80;
			} else X6502_IRQBegin(FCEU_IQDPCM);
		}
		break;
	}
	PSG[A] = V;
}
int dmc7bfl;
static DECLFW(Write_DMCRegs)
{
	A&=0xF;
	
	switch(A)
	{
	case 0x00:
		DoPCM();
	    LoadDMCPeriod(V&0xF);
	
	    if(SIRQStat&0x80)
	    {
			if(!(V&0x80))
			{
				X6502_IRQEnd(FCEU_IQDPCM);
				SIRQStat&=~0x80;
			}
			else X6502_IRQBegin(FCEU_IQDPCM);
	    }
		DMCFormat=V;
		break;
	case 0x01:
		DoPCM();
		RawDALatch=V&0x7F;
		//RawDALatch=InitialRawDALatch;
		if (RawDALatch) {
			DMC_7bit = 1; totalscanlines = 240;
		}
		break;
	case 0x02:
		DMCAddressLatch=V;
		if (V)
			DMC_7bit = 0;
		break;
	case 0x03:
		DMCSizeLatch=V;
		if (V)
			DMC_7bit = 0;
		break;
	}
}

static DECLFW(Write_DACRegs)		//4100 regs
{
	A &= 0xF;
	//printf("DAC register get wrote, Address: %04x, Value: %04x\n", A, V);
	switch (A)
	{
	case 0x00:DoPCM();
		LoadDACPeriod(V & 0xF);
		if (!(V & 0x10))
			PrepDAC();
		else DACSize = 0;
		if (SIRQStat & 0x80)
		{
			if ((V & 0xC0) != 0x80)
			{
				X6502_IRQEnd(FCEU_IQDPCM);
				SIRQStat &= ~0x80;
			}
		}
		DACFormat = V;
		break;
	case 0x01:DoPCM();
		RawDALatch = V & 0x7F;
		//    if(RawDALatch && !weneedfix)DMC_7bit = 1; 

		//	if(!weneedfix)dendyline=1;}
			//else{razgon=0;}
		break;
	case 0x02:DACAddressLo = V;

		break;
	case 0x03:DACAddressHi = V;
		break;
	case 0x04:DACSizeLo = V;
		break;
	case 0x05:DACSizeHi = V;
		break;
	case 0x06:DACFreqLo = V;
		break;
	case 0x07:DACFreqHi = V;
		break;
	}

}
static DECLFW(StatusWrite) {
	int x;
/*  FCEU_printf("APU1 %04x:%04x\n",A,V); */

	DoSQ1();
	DoSQ2();
	DoTriangle();
	DoNoise();
	DoPCM();
	for (x = 0; x < 4; x++)
		if (!(V & (1 << x))) lengthcount[x] = 0;	/* Force length counters to 0. */

	if (V & 0x10) {
		if (!DMCSize)
			PrepDPCM();
	} else {
		DMCSize = 0;
	}
	SIRQStat &= ~0x80;
	X6502_IRQEnd(FCEU_IQDPCM);
	EnabledChannels = V & 0x1F;
}

static DECLFR(StatusRead) {
	int x;
	uint8 ret;

	ret = SIRQStat;

	for (x = 0; x < 4; x++) ret |= lengthcount[x] ? (1 << x) : 0;
	if (DMCSize) ret |= 0x10;

		#ifdef FCEUDEF_DEBUGGER
	if (!fceuindbg)
		#endif
	{
		SIRQStat &= ~0x40;
		X6502_IRQEnd(FCEU_IQFCOUNT);
	}
	return ret;
}

static void FASTAPASS(1) FrameSoundStuff(int V) {
	int P;

	DoSQ1();
	DoSQ2();
	DoNoise();
	DoTriangle();

	if (!(V & 1)) {	/* Envelope decay, linear counter, length counter, freq sweep */
		if (!(PSG[8] & 0x80))
			if (lengthcount[2] > 0)
				lengthcount[2]--;

		if (!(PSG[0xC] & 0x20))	/* Make sure loop flag is not set. */
			if (lengthcount[3] > 0)
				lengthcount[3]--;

		for (P = 0; P < 2; P++) {
			if (!(PSG[P << 2] & 0x20))	/* Make sure loop flag is not set. */
				if (lengthcount[P] > 0)
					lengthcount[P]--;

			/* Frequency Sweep Code Here */
			/* xxxx 0000 */
			/* xxxx = hz.  120/(x+1)*/
			if (sweepon[P]) {
				int32 mod = 0;

				if (SweepCount[P] > 0) SweepCount[P]--;
				if (SweepCount[P] <= 0) {
					SweepCount[P] = ((PSG[(P << 2) + 0x1] >> 4) & 7) + 1;	/* +1; */
					if (PSG[(P << 2) + 0x1] & 0x8) {
						mod -= (P ^ 1) + ((curfreq[P]) >> (PSG[(P << 2) + 0x1] & 7));
						if (curfreq[P] && (PSG[(P << 2) + 0x1] & 7)	/* && sweepon[P]&0x80*/) {
							curfreq[P] += mod;
						}
					} else {
						mod = curfreq[P] >> (PSG[(P << 2) + 0x1] & 7);
						if ((mod + curfreq[P]) & 0x800) {
							sweepon[P] = 0;
							curfreq[P] = 0;
						} else {
							if (curfreq[P] && (PSG[(P << 2) + 0x1] & 7)	/* && sweepon[P]&0x80*/) {
								curfreq[P] += mod;
							}
						}
					}
				}
			} else {/* Sweeping is disabled: */
#if 0
				curfreq[P]&=0xFF00;
				curfreq[P]|=PSG[(P<<2)|0x2]; /* |((PSG[(P<<2)|3]&7)<<8); */
#endif
			}
		}
	}

	/* Now do envelope decay + linear counter. */

	if (TriMode)/* In load mode? */
		TriCount = PSG[0x8] & 0x7F;
	else if (TriCount)
		TriCount--;

	if (!(PSG[0x8] & 0x80))
		TriMode = 0;

	for (P = 0; P < 3; P++) {
		if (EnvUnits[P].reloaddec) {
			EnvUnits[P].decvolume = 0xF;
			EnvUnits[P].DecCountTo1 = EnvUnits[P].Speed + 1;
			EnvUnits[P].reloaddec = 0;
			continue;
		}

		if (EnvUnits[P].DecCountTo1 > 0) EnvUnits[P].DecCountTo1--;
		if (EnvUnits[P].DecCountTo1 == 0) {
			EnvUnits[P].DecCountTo1 = EnvUnits[P].Speed + 1;
			if (EnvUnits[P].decvolume || (EnvUnits[P].Mode & 0x2)) {
				EnvUnits[P].decvolume--;
				EnvUnits[P].decvolume &= 0xF;
			}
		}
	}
}

void FrameSoundUpdate(void) {
	/* Linear counter:  Bit 0-6 of $4008
	 * Length counter:  Bit 4-7 of $4003, $4007, $400b, $400f
	 */

	if (!fcnt && !(IRQFrameMode & 0x3)) {
		SIRQStat |= 0x40;
		X6502_IRQBegin(FCEU_IQFCOUNT);
	}

	if (fcnt == 3) {
		if (IRQFrameMode & 0x2)
			fhcnt += fhinc;
	}
	FrameSoundStuff(fcnt);
	fcnt = (fcnt + 1) & 3;
}


static INLINE void tester(void) {
	if (DMCBitCount == 0) {
		if (!DMCHaveDMA)
			DMCHaveSample = 0;
		else {
			DMCHaveSample = 1;
			DMCShift = DMCDMABuf;
			DMCHaveDMA = 0;
		}
	}
}

static INLINE void DMCDMA(void)
{
  if(DMCSize && !DMCHaveDMA)
  {
	  if(!nwram)
	  {
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   X6502_DMR(0x8000+DMCAddress);
   DMCDMABuf=X6502_DMR(0x8000+DMCAddress);
   DMCHaveDMA=1;
   DMCAddress=(DMCAddress+1)&0x7fff;
	  }
	  else
	  {
   X6502_DMR2(DMCAddress, 0);
   X6502_DMR2(DMCAddress, 0);
   X6502_DMR2(DMCAddress, 0);
   DMCDMABuf=X6502_DMR2(DMCAddress, 0);
   DMCHaveDMA=1;
   DMCAddress=(DMCAddress+1);
	  }
   DMCSize--;
   if(!DMCSize)
   {
    if(DMCFormat&0x40)
     PrepDPCM();
    else
    {
       SIRQStat|=0x80;
     if(DMCFormat&0x80)
	  {
	
      X6502_IRQBegin(FCEU_IQDPCM);
    }
	}
   }
 }
}
static INLINE void DACDMA(void)
{
	if (DACSize && !DACHaveDMA)
	{


		DACDMABuf = DACRam[DACAddress];

		//  DACDMABuf=X6502_DMR(DACAddress);
		//  DACDMABuf=X6502_DMR(DACAddress);
		//  DACDMABuf=X6502_DMR(DACAddress);
		//  DACDMABuf=X6502_DMR(DACAddress);
		 //  printf("Dac buffer, Address: %04x, Value: %02x\n", DACAddress, DACDMABuf);
		
		DACHaveDMA = 1;
		DACAddress++;

		DACSize--;
		if (!DACSize)
		{
			if (DACFormat & 0x40)
				PrepDAC();
			else
			{
				if (DACFormat & 0x80)
				{
					SIRQStat |= 0x80;
					X6502_IRQBegin(FCEU_IQDPCM);
				}
			}
		}
	}
}
void FASTAPASS(1) FCEU_SoundCPUHook(int cycles) {
	fhcnt -= cycles * 48;
	if (fhcnt <= 0) {
		FrameSoundUpdate();
		fhcnt += fhinc;
	}

	DMCDMA();
	DMCacc -= cycles;

	while (DMCacc <= 0) {
		if (DMCHaveSample) {
			uint8 bah = RawDALatch;
			int t = ((DMCShift & 1) << 2) - 2;
			/* Unbelievably ugly hack */
			if (FSettings.SndRate) {
				soundtsoffs += DMCacc;
				DoPCM();
				soundtsoffs -= DMCacc;
			}
			RawDALatch += t;
			if (RawDALatch & 0x80)
				RawDALatch = bah;
		}

		DMCacc += DMCPeriod;
		DMCBitCount = (DMCBitCount + 1) & 7;
		DMCShift >>= 1;
		tester();
	}
	 if (vrc6_snd)
 {
	 DACDMA();
	 DACacc -= cycles;
	 while (DACacc <= 0)
	 {
		 if (DACHaveDMA)
		 {
			 //   soundtsoffs+=DACacc;
			 DoPCM();
			 //   soundtsoffs-=DACacc;
			 RawDALatch = DACDMABuf & 0x7F;
			 DACHaveDMA = 0;
			 // printf("DAC, data get for channel: %02x\n", RawDALatch);
		 }

		 DACacc += DACPeriod;
		 //  testerDAC();
	 }
 }
}

void RDoPCM(void) {
	uint32 V;

	for (V = ChannelBC[4]; V < SOUNDTS; V++)
		WaveHi[V] += RawDALatch << 16;

	ChannelBC[4] = SOUNDTS;
}

/* This has the correct phase.  Don't mess with it. */
static INLINE void RDoSQ(int x) {
	int32 V;
	int32 amp;
	int32 rthresh;
	int32 *D;
	int32 currdc;
	int32 cf;
	int32 rc;

	if (curfreq[x] < 8 || curfreq[x] > 0x7ff)
		goto endit;
	if (!CheckFreq(curfreq[x], PSG[(x << 2) | 0x1]))
		goto endit;
	if (!lengthcount[x])
		goto endit;

	if (EnvUnits[x].Mode & 0x1)
		amp = EnvUnits[x].Speed;
	else
		amp = EnvUnits[x].decvolume;
/*   printf("%d\n",amp); */
	amp <<= 24;

	rthresh = RectDuties[(PSG[(x << 2)] & 0xC0) >> 6];

	D = &WaveHi[ChannelBC[x]];
	V = SOUNDTS - ChannelBC[x];

	currdc = RectDutyCount[x];
	cf = (curfreq[x] + 1) * 2;
	rc = wlcount[x];

	while (V > 0) {
		if (currdc < rthresh)
			*D += amp;
		rc--;
		if (!rc) {
			rc = cf;
			currdc = (currdc + 1) & 7;
		}
		V--;
		D++;
	}

	RectDutyCount[x] = currdc;
	wlcount[x] = rc;

 endit:
	ChannelBC[x] = SOUNDTS;
}

static void RDoSQ1(void) {
	RDoSQ(0);
}

static void RDoSQ2(void) {
	RDoSQ(1);
}

static void RDoSQLQ(void) {
	int32 start, end;
	int32 V;
	int32 amp[2];
	int32 rthresh[2];
	int32 freq[2];
	int x;
	int32 inie[2];

	int32 ttable[2][8];
	int32 totalout;

	start = ChannelBC[0];
	end = (SOUNDTS << 16) / soundtsinc;
	if (end <= start) return;
	ChannelBC[0] = end;

	for (x = 0; x < 2; x++) {
		int y;

		inie[x] = nesincsize;
		if (curfreq[x] < 8 || curfreq[x] > 0x7ff)
			inie[x] = 0;
		if (!CheckFreq(curfreq[x], PSG[(x << 2) | 0x1]))
			inie[x] = 0;
		if (!lengthcount[x])
			inie[x] = 0;

		if (EnvUnits[x].Mode & 0x1)
			amp[x] = EnvUnits[x].Speed;
		else
			amp[x] = EnvUnits[x].decvolume;

		if (!inie[x]) amp[x] = 0;	/* Correct? Buzzing in MM2, others otherwise... */

		rthresh[x] = RectDuties[(PSG[x * 4] & 0xC0) >> 6];

		for (y = 0; y < 8; y++) {
			if (y < rthresh[x])
				ttable[x][y] = amp[x];
			else
				ttable[x][y] = 0;
		}
		freq[x] = (curfreq[x] + 1) << 1;
		freq[x] <<= 17;
	}

	totalout = wlookup1[ ttable[0][RectDutyCount[0]] + ttable[1][RectDutyCount[1]] ];

	if (!inie[0] && !inie[1]) {
		for (V = start; V < end; V++)
			Wave[V >> 4] += totalout;
	} else
		for (V = start; V < end; V++) {
			/* int tmpamp=0;
			if(RectDutyCount[0]<rthresh[0])
			 tmpamp=amp[0];
			if(RectDutyCount[1]<rthresh[1])
			 tmpamp+=amp[1];
			tmpamp=wlookup1[tmpamp];
			tmpamp = wlookup1[ ttable[0][RectDutyCount[0]] + ttable[1][RectDutyCount[1]] ];
			*/

			Wave[V >> 4] += totalout;	/* tmpamp; */

			sqacc[0] -= inie[0];
			sqacc[1] -= inie[1];

			if (sqacc[0] <= 0) {
 rea:
				sqacc[0] += freq[0];
				RectDutyCount[0] = (RectDutyCount[0] + 1) & 7;
				if (sqacc[0] <= 0) goto rea;
				totalout = wlookup1[ ttable[0][RectDutyCount[0]] + ttable[1][RectDutyCount[1]] ];
			}

			if (sqacc[1] <= 0) {
 rea2:
				sqacc[1] += freq[1];
				RectDutyCount[1] = (RectDutyCount[1] + 1) & 7;
				if (sqacc[1] <= 0) goto rea2;
				totalout = wlookup1[ ttable[0][RectDutyCount[0]] + ttable[1][RectDutyCount[1]] ];
			}
		}
}

static void RDoTriangle(void) {
	int32 V;
	int32 tcout;

	tcout = (tristep & 0xF);
	if (!(tristep & 0x10)) tcout ^= 0xF;
	tcout = (tcout * 3) << 16;	/* (tcout<<1); */

	if (!lengthcount[2] || !TriCount) {	/* Counter is halted, but we still need to output. */
		int32 *start = &WaveHi[ChannelBC[2]];
		int32 count = SOUNDTS - ChannelBC[2];
		while (count--) {
			*start += tcout;
			start++;
		}
#if 0
		for(V = ChannelBC[2]; V < SOUNDTS; V++)
			WaveHi[V] += tcout;
#endif
	} else
		for (V = ChannelBC[2]; V < SOUNDTS; V++) {
			WaveHi[V] += tcout;
			wlcount[2]--;
			if (!wlcount[2]) {
				wlcount[2] = (PSG[0xa] | ((PSG[0xb] & 7) << 8)) + 1;
				tristep++;
				tcout = (tristep & 0xF);
				if (!(tristep & 0x10)) tcout ^= 0xF;
				tcout = (tcout * 3) << 16;
			}
		}

	ChannelBC[2] = SOUNDTS;
}

uint32 lq_tcout = 0;
int32 lq_triacc = 0;
int32 lq_noiseacc = 0;

static void RDoTriangleNoisePCMLQ(void) {
	int32 V;
	int32 start, end;
	int32 freq[2];
	int32 inie[2];
	uint32 amptab[2];
	uint32 noiseout;
	int nshift;

	int32 totalout;

	start = ChannelBC[2];
	end = (SOUNDTS << 16) / soundtsinc;
	if (end <= start) return;
	ChannelBC[2] = end;

	inie[0] = inie[1] = nesincsize;

	freq[0] = (((PSG[0xa] | ((PSG[0xb] & 7) << 8)) + 1));

	if (!lengthcount[2] || !TriCount || freq[0] <= 4)
		inie[0] = 0;

	freq[0] <<= 17;
	if (EnvUnits[2].Mode & 0x1)
		amptab[0] = EnvUnits[2].Speed;
	else
		amptab[0] = EnvUnits[2].decvolume;
	amptab[1] = 0;
	amptab[0] <<= 1;

	if (!lengthcount[3])
		amptab[0] = inie[1] = 0;	/* Quick hack speedup, set inie[1] to 0 */

	noiseout = amptab[(nreg >> 0xe) & 1];

	if (PSG[0xE] & 0x80)
		nshift = 8;
	else
		nshift = 13;


	totalout = wlookup2[lq_tcout + noiseout + RawDALatch];

	if (inie[0] && inie[1]) {
		for (V = start; V < end; V++) {
			Wave[V >> 4] += totalout;

			lq_triacc -= inie[0];
			lq_noiseacc -= inie[1];

			if (lq_triacc <= 0) {
 rea:
				lq_triacc += freq[0];	/* t; */
				tristep = (tristep + 1) & 0x1F;
				if (lq_triacc <= 0) goto rea;
				lq_tcout = (tristep & 0xF);
				if (!(tristep & 0x10)) lq_tcout ^= 0xF;
				lq_tcout = lq_tcout * 3;
				totalout = wlookup2[lq_tcout + noiseout + RawDALatch];
			}

			if (lq_noiseacc <= 0) {
 rea2:
				/* used to added <<(16+2) when the noise table
				 * values were half.
				 */
				if (PAL)
					lq_noiseacc += PALNoiseFreqTable[PSG[0xE] & 0xF] << (16 + 1);
				else
					lq_noiseacc += NTSCNoiseFreqTable[PSG[0xE] & 0xF] << (16 + 1);
				nreg = (nreg << 1) + (((nreg >> nshift) ^ (nreg >> 14)) & 1);
				nreg &= 0x7fff;
				noiseout = amptab[(nreg >> 0xe) & 1];
				if (lq_noiseacc <= 0) goto rea2;
				totalout = wlookup2[lq_tcout + noiseout + RawDALatch];
			}	/* noiseacc<=0 */
		}	/* for(V=... */
	} else if (inie[0]) {
		for (V = start; V < end; V++) {
			Wave[V >> 4] += totalout;

			lq_triacc -= inie[0];

			if (lq_triacc <= 0) {
 area:
				lq_triacc += freq[0];	/* t; */
				tristep = (tristep + 1) & 0x1F;
				if (lq_triacc <= 0) goto area;
				lq_tcout = (tristep & 0xF);
				if (!(tristep & 0x10)) lq_tcout ^= 0xF;
				lq_tcout = lq_tcout * 3;
				totalout = wlookup2[lq_tcout + noiseout + RawDALatch];
			}
		}
	} else if (inie[1]) {
		for (V = start; V < end; V++) {
			Wave[V >> 4] += totalout;
			lq_noiseacc -= inie[1];
			if (lq_noiseacc <= 0) {
 area2:
				/* used to be added <<(16+2) when the noise table
				 * values were half.
				 */
				if (PAL)
					lq_noiseacc += PALNoiseFreqTable[PSG[0xE] & 0xF] << (16 + 1);
				else
					lq_noiseacc += NTSCNoiseFreqTable[PSG[0xE] & 0xF] << (16 + 1);
				nreg = (nreg << 1) + (((nreg >> nshift) ^ (nreg >> 14)) & 1);
				nreg &= 0x7fff;
				noiseout = amptab[(nreg >> 0xe) & 1];
				if (lq_noiseacc <= 0) goto area2;
				totalout = wlookup2[lq_tcout + noiseout + RawDALatch];
			}	/* noiseacc<=0 */
		}
	} else {
		for (V = start; V < end; V++)
			Wave[V >> 4] += totalout;
	}
}


static void RDoNoise(void) {
	uint32 V;
	int32 outo;
	uint32 amptab[2];

	if (EnvUnits[2].Mode & 0x1)
		amptab[0] = EnvUnits[2].Speed;
	else
		amptab[0] = EnvUnits[2].decvolume;

	amptab[0] <<= 16;
	amptab[1] = 0;

	amptab[0] <<= 1;

	outo = amptab[(nreg >> 0xe) & 1];

	if (!lengthcount[3]) {
		outo = amptab[0] = 0;
	}

	if (PSG[0xE] & 0x80)/* "short" noise */
		for (V = ChannelBC[3]; V < SOUNDTS; V++) {
			WaveHi[V] += outo;
			wlcount[3]--;
			if (!wlcount[3]) {
				uint8 feedback;
				if (PAL)
					wlcount[3] = PALNoiseFreqTable[PSG[0xE] & 0xF];
				else
					wlcount[3] = NTSCNoiseFreqTable[PSG[0xE] & 0xF];
				feedback = ((nreg >> 8) & 1) ^ ((nreg >> 14) & 1);
				nreg = (nreg << 1) + feedback;
				nreg &= 0x7fff;
				outo = amptab[(nreg >> 0xe) & 1];
			}
		}
	else
		for (V = ChannelBC[3]; V < SOUNDTS; V++) {
			WaveHi[V] += outo;
			wlcount[3]--;
			if (!wlcount[3]) {
				uint8 feedback;
				if (PAL)
					wlcount[3] = PALNoiseFreqTable[PSG[0xE] & 0xF];
				else
					wlcount[3] = NTSCNoiseFreqTable[PSG[0xE] & 0xF];
				feedback = ((nreg >> 13) & 1) ^ ((nreg >> 14) & 1);
				nreg = (nreg << 1) + feedback;
				nreg &= 0x7fff;
				outo = amptab[(nreg >> 0xe) & 1];
			}
		}
	ChannelBC[3] = SOUNDTS;
}

DECLFW(Write_IRQFM) {
	V = (V & 0xC0) >> 6;
	fcnt = 0;
	if (V & 2)
		FrameSoundUpdate();
	fcnt = 1;
	fhcnt = fhinc;
	X6502_IRQEnd(FCEU_IQFCOUNT);
	SIRQStat &= ~0x40;
	IRQFrameMode = V;
}



static int32 inbuf = 0;
int FlushEmulateSound(void) {
	int x;
	int32 end, left;

	if (!sound_timestamp) return(0);

	if (!FSettings.SndRate) {
		left = 0;
		end = 0;
		goto nosoundo;
	}

	DoSQ1();
	DoSQ2();
	DoTriangle();
	DoNoise();
	DoPCM();

	if (FSettings.soundq >= 1) {
		int32 *tmpo = &WaveHi[soundtsoffs];

		if (GameExpSound.HiFill) GameExpSound.HiFill();

		for (x = sound_timestamp; x; x--) {
			uint32 b = *tmpo;
			*tmpo = (b & 65535) + wlookup2[(b >> 16) & 255] + wlookup1[b >> 24];
			tmpo++;
		}
		end = NeoFilterSound(WaveHi, WaveFinal, SOUNDTS, &left);

		memmove(WaveHi, WaveHi + SOUNDTS - left, left * sizeof(uint32));
		memset(WaveHi + left, 0, sizeof(WaveHi) - left * sizeof(uint32));

		if (GameExpSound.HiSync) GameExpSound.HiSync(left);
		for (x = 0; x < 5; x++)
			ChannelBC[x] = left;
	} else {
		end = (SOUNDTS << 16) / soundtsinc;
		if (GameExpSound.Fill)
			GameExpSound.Fill(end & 0xF);

		SexyFilter(Wave, WaveFinal, end >> 4);

#if 0
		if (FSettings.lowpass)
			SexyFilter2(WaveFinal, end >> 4);
#endif
		if (end & 0xF)
			Wave[0] = Wave[(end >> 4)];
		Wave[end >> 4] = 0;
	}
 nosoundo:

	if (FSettings.soundq >= 1) {
		soundtsoffs = left;
	} else {
		for (x = 0; x < 5; x++)
			ChannelBC[x] = end & 0xF;
		soundtsoffs = (soundtsinc * (end & 0xF)) >> 16;
		end >>= 4;
	}
	inbuf = end;

	/* FCEU_WriteWaveData(WaveFinal, end);	 This function will just return
										if sound recording is off. */
	return(end);
}

int GetSoundBuffer(int32 **W) {
	*W = WaveFinal;
	return(inbuf);
}

/* FIXME:  Find out what sound registers get reset on reset.  I know $4001/$4005 don't,
due to that whole MegaMan 2 Game Genie thing.
*/

void FCEUSND_Reset(void) {
	int x;

	IRQFrameMode = 0x0;
	fhcnt = fhinc;
	fcnt = 0;
	nreg = 1;

	for (x = 0; x < 2; x++) {
		wlcount[x] = 2048;
		if (nesincsize)	/* lq mode */
			sqacc[x] = ((uint32)2048 << 17) / nesincsize;
		else
			sqacc[x] = 1;
		sweepon[x] = 0;
		curfreq[x] = 0;
	}
	wlcount[2] = 1;	/* 2048; */
	wlcount[3] = 2048;
	DMCHaveDMA = DMCHaveSample = 0;
	SIRQStat = 0x00;

	RawDALatch = 0x00;
	TriCount = 0;
	TriMode = 0;
	tristep = 0;
	EnabledChannels = 0;
	for (x = 0; x < 4; x++)
		lengthcount[x] = 0;

	DMCAddressLatch = 0;
	DMCSizeLatch = 0;
	DMCFormat = 0;
	DMCAddress = 0;
	DMCSize = 0;
	DMCShift = 0;
	
	DACAddressLo = 0;
	DACAddressHi = 0;
	DACSizeLo = 0;
	DACSizeHi = 0;
	DACSize = 0;
	DACacc = 1;
	DACPeriod = 0;
	DACFormat = 0;
	DACHaveDMA = 0;
	DACDMABuf = 0;
	DACHaveSample = 0;
	LoadDACPeriod(DACFormat & 0xF);
}

void FCEUSND_Power(void) {
	int x;

	SetNESSoundMap();
	memset(PSG, 0x00, sizeof(PSG));
	FCEUSND_Reset();

	memset(Wave, 0, sizeof(Wave));
	memset(WaveHi, 0, sizeof(WaveHi));
	memset(&EnvUnits, 0, sizeof(EnvUnits));

	for (x = 0; x < 5; x++)
		ChannelBC[x] = 0;
	soundtsoffs = 0;
	LoadDMCPeriod(DMCFormat & 0xF);
}


void SetSoundVariables(void) {
	int x;

	fhinc = PAL ? 16626 : 14915;	/* *2 CPU clock rate */
	fhinc *= 24;

	if (FSettings.SndRate) {
		wlookup1[0] = 0;
		for (x = 1; x < 32; x++) {
			wlookup1[x] = (double)16 * 16 * 16 * 4 * 95.52 / ((double)8128 / (double)x + 100);
			if (!FSettings.soundq) wlookup1[x] >>= 4;
		}
		wlookup2[0] = 0;
		for (x = 1; x < 203; x++) {
			wlookup2[x] = (double)16 * 16 * 16 * 4 * 163.67 / ((double)24329 / (double)x + 100);
			if (!FSettings.soundq) wlookup2[x] >>= 4;
		}
		if (FSettings.soundq >= 1) {
			DoNoise = RDoNoise;
			DoTriangle = RDoTriangle;
			DoPCM = RDoPCM;
			DoSQ1 = RDoSQ1;
			DoSQ2 = RDoSQ2;
		} else {
			DoNoise = DoTriangle = DoPCM = DoSQ1 = DoSQ2 = Dummyfunc;
			DoSQ1 = RDoSQLQ;
			DoSQ2 = RDoSQLQ;
			DoTriangle = RDoTriangleNoisePCMLQ;
			DoNoise = RDoTriangleNoisePCMLQ;
			DoPCM = RDoTriangleNoisePCMLQ;
		}
	} else {
		DoNoise = DoTriangle = DoPCM = DoSQ1 = DoSQ2 = Dummyfunc;
		return;
	}

	MakeFilters(FSettings.SndRate);

	if (GameExpSound.RChange)
		GameExpSound.RChange();

	nesincsize = (int64)(((int64)1 << 17) * (double)(PAL ? PAL_CPU : NTSC_CPU) / (FSettings.SndRate * 16));
	memset(sqacc, 0, sizeof(sqacc));
	memset(ChannelBC, 0, sizeof(ChannelBC));

	LoadDMCPeriod(DMCFormat & 0xF);	/* For changing from PAL to NTSC */

	soundtsinc = (uint32)((uint64)(PAL ? (long double)PAL_CPU * 65536 : (long double)NTSC_CPU * 65536) / (FSettings.SndRate * 16));
}

void FCEUI_Sound(int Rate) {
	FSettings.SndRate = Rate;
	SetSoundVariables();
}

void FCEUI_SetLowPass(int q) {
	FSettings.lowpass = q;
}

void FCEUI_SetSoundQuality(int quality) {
	FSettings.soundq = quality;
	SetSoundVariables();
}

void FCEUI_SetSoundVolume(uint32 volume) {
	FSettings.SoundVolume = volume;
}

static void DoSQV1(void);
static void DoSQV2(void);
static void DoSawV(void);

static INLINE void DoSQV(int x) {
	int32 V;
	int32 amp = (((vpsg12[x << 2] & 15) << 8) * 6 / 8) >> 4;
	int32 start, end;

	start = cvbc2[x];
	end = (SOUNDTS << 16) / soundtsinc;
	if (end <= start) return;
	cvbc2[x] = end;

	if (vpsg12[(x << 2) | 0x2] & 0x80) {
		if (vpsg12[x << 2] & 0x80) {
			for (V = start; V < end; V++)
				Wave[V >> 4] += amp;
		} else {
			int32 thresh = (vpsg12[x << 2] >> 4) & 7;
			int32 freq = ((vpsg12[(x << 2) | 0x1] | ((vpsg12[(x << 2) | 0x2] & 15) << 8)) + 1) << 17;
			for (V = start; V < end; V++) {
				if (dcount2[x] > thresh)
					Wave[V >> 4] += amp;
				vcount2[x] -= nesincsize;
				while (vcount2[x] <= 0) {
					vcount2[x] += freq;
					dcount2[x] = (dcount2[x] + 1) & 15;
				}
			}
		}
	}
}

static void DoSQV1(void) {
	DoSQV(0);
}

static void DoSQV2(void) {
	DoSQV(1);
}

static void DoSawV(void) {
	int V;
	int32 start, end;

	start = cvbc2[2];
	end = (SOUNDTS << 16) / soundtsinc;
	if (end <= start) return;
	cvbc2[2] = end;

	if (vpsg22[2] & 0x80) {
		static int32 saw1phaseacc = 0;
		uint32 freq3;
		static uint8 b3 = 0;
		static int32 phaseacc = 0;
		static uint32 duff = 0;

		freq3 = (vpsg22[1] + ((vpsg22[2] & 15) << 8) + 1);

		for (V = start; V < end; V++) {
			saw1phaseacc -= nesincsize;
			if (saw1phaseacc <= 0) {
				int32 t;
 rea:
				t = freq3;
				t <<= 18;
				saw1phaseacc += t;
				phaseacc += vpsg22[0] & 0x3f;
				b3++;
				if (b3 == 7) {
					b3 = 0;
					phaseacc = 0;
				}
				if (saw1phaseacc <= 0)
					goto rea;
				duff = (((phaseacc >> 3) & 0x1f) << 4) * 6 / 8;
			}
			Wave[V >> 4] += duff;
		}
	}
}

static INLINE void DoSQVHQ(int x) {
	int32 V;
	int32 amp = ((vpsg12[x << 2] & 15) << 8) * 6 / 8;

	if (vpsg12[(x << 2) | 0x2] & 0x80) {
		if (vpsg12[x << 2] & 0x80) {
			for (V = cvbc2[x]; V < (int)SOUNDTS; V++)
				WaveHi[V] += amp;
		} else {
			int32 thresh = (vpsg12[x << 2] >> 4) & 7;
			for (V = cvbc2[x]; V < (int)SOUNDTS; V++) {
				if (dcount2[x] > thresh)
					WaveHi[V] += amp;
				vcount2[x]--;
				if (vcount2[x] <= 0) {
					vcount2[x] = (vpsg12[(x << 2) | 0x1] | ((vpsg12[(x << 2) | 0x2] & 15) << 8)) + 1;
					dcount2[x] = (dcount2[x] + 1) & 15;
				}
			}
		}
	}
	cvbc2[x] = SOUNDTS;
}

static void DoSQV1HQ(void) {
	DoSQVHQ(0);
}

static void DoSQV2HQ(void) {
	DoSQVHQ(1);
}

static void DoSawVHQ(void) {
	static uint8 b3 = 0;
	static int32 phaseacc = 0;
	int32 V;

	if (vpsg22[2] & 0x80) {
		for (V = cvbc2[2]; V < (int)SOUNDTS; V++) {
			WaveHi[V] += (((phaseacc >> 3) & 0x1f) << 8) * 6 / 8;
			vcount2[2]--;
			if (vcount2[2] <= 0) {
				vcount2[2] = (vpsg22[1] + ((vpsg22[2] & 15) << 8) + 1) << 1;
				phaseacc += vpsg22[0] & 0x3f;
				b3++;
				if (b3 == 7) {
					b3 = 0;
					phaseacc = 0;
				}
			}
		}
	}
	cvbc2[2] = SOUNDTS;
}

static SFORMAT SStateRegs[] =
{
	{ vpsg12, 8, "PSG1" },
	{ vpsg22, 4, "PSG2" },
	{ 0 }
};
void VRC6Sound2(int Count) {
	int x;

	DoSQV1();
	DoSQV2();
	DoSawV();
	for (x = 0; x < 3; x++)
		cvbc2[x] = Count;
}

void VRC6Sound2HQ2(void) {
	DoSQV1HQ();
	DoSQV2HQ();
	DoSawVHQ();
}

void VRC6SyncHQ2(int32 ts) {
	int x;
	for (x = 0; x < 3; x++) cvbc2[x] = ts;
}
void VRC6_ESI(void) {
	GameExpSound.RChange = VRC6_ESI;
	GameExpSound.Fill = VRC6Sound2;
	GameExpSound.HiFill = VRC6Sound2HQ2;
	GameExpSound.HiSync = VRC6SyncHQ2;

	memset(cvbc2, 0, sizeof(cvbc2));
	memset(vcount2, 0, sizeof(vcount2));
	memset(dcount2, 0, sizeof(dcount2));
//	if (FSettings.SndRate) {
		if (FSettings.soundq >= 1) {
			sfun2[0] = DoSQV1HQ;
			sfun2[1] = DoSQV2HQ;
			sfun2[2] = DoSawVHQ;
		} else {
			sfun2[0] = DoSQV1;
			sfun2[1] = DoSQV2;
			sfun2[2] = DoSawV;
		}
//	} else
//		memset(sfun2, 0, sizeof(sfun2));
	AddExState(&SStateRegs, ~0, 0, 0);
}
static DECLFW(VRC6SW) {
	//A &= 0xF003;
	 if(A>=0x4040 && A<=0x4042)
        {
		vpsg12[A & 3] = V;
		if (sfun2[0]) sfun2[0]();
	}  else if(A>=0x4050 && A<=0x4052) {
		vpsg12[4 | (A & 3)] = V;
		if (sfun2[1]) sfun2[1]();
	} else if(A>=0x4060 && A<=0x4062) {
		vpsg22[A & 3] = V;
		if (sfun2[2]) sfun2[2]();
	}
}
void SetNESSoundMap(void) {
	SetWriteHandler(0x4000, 0x400F, Write_PSG);
	SetWriteHandler(0x4010, 0x4013, Write_DMCRegs);
	SetWriteHandler(0x4017, 0x4017, Write_IRQFM);
  if (vrc6_snd)SetWriteHandler(0x4110, 0x4117, Write_DACRegs);
	SetWriteHandler(0x4015, 0x4015, StatusWrite);
	SetReadHandler(0x4015, 0x4015, StatusRead);
	  if(vrc6_snd)
		{
		  SetWriteHandler(0x4040, 0x4070, VRC6SW);
		  VRC6_ESI();
		}
}
SFORMAT FCEUSND_STATEINFO[] = {
	{ &fhcnt, 4 | FCEUSTATE_RLSB, "FHCN" },
	{ &fcnt, 1, "FCNT" },
	{ PSG, 0x10, "PSG" },
	{ &EnabledChannels, 1, "ENCH" },
	{ &IRQFrameMode, 1, "IQFM" },
	{ &nreg, 2 | FCEUSTATE_RLSB, "NREG" },
	{ &TriMode, 1, "TRIM" },
	{ &TriCount, 1, "TRIC" },

	{ &EnvUnits[0].Speed, 1, "E0SP" },
	{ &EnvUnits[1].Speed, 1, "E1SP" },
	{ &EnvUnits[2].Speed, 1, "E2SP" },

	{ &EnvUnits[0].Mode, 1, "E0MO" },
	{ &EnvUnits[1].Mode, 1, "E1MO" },
	{ &EnvUnits[2].Mode, 1, "E2MO" },

	{ &EnvUnits[0].DecCountTo1, 1, "E0D1" },
	{ &EnvUnits[1].DecCountTo1, 1, "E1D1" },
	{ &EnvUnits[2].DecCountTo1, 1, "E2D1" },

	{ &EnvUnits[0].decvolume, 1, "E0DV" },
	{ &EnvUnits[1].decvolume, 1, "E1DV" },
	{ &EnvUnits[2].decvolume, 1, "E2DV" },

	{ &lengthcount[0], 4 | FCEUSTATE_RLSB, "LEN0" },
	{ &lengthcount[1], 4 | FCEUSTATE_RLSB, "LEN1" },
	{ &lengthcount[2], 4 | FCEUSTATE_RLSB, "LEN2" },
	{ &lengthcount[3], 4 | FCEUSTATE_RLSB, "LEN3" },
	{ sweepon, 2, "SWEE" },
	{ &curfreq[0], 4 | FCEUSTATE_RLSB, "CRF1" },
	{ &curfreq[1], 4 | FCEUSTATE_RLSB, "CRF2" },
	{ SweepCount, 2, "SWCT" },

	{ &SIRQStat, 1, "SIRQ" },

	{ &DMCacc, 4 | FCEUSTATE_RLSB, "5ACC" },
	{ &DMCBitCount, 1, "5BIT" },
	{ &DMCAddress, 4 | FCEUSTATE_RLSB, "5ADD" },
	{ &DMCSize, 4 | FCEUSTATE_RLSB, "5SIZ" },
	{ &DMCShift, 1, "5SHF" },

	{ &DMCHaveDMA, 1, "5VDM" },
	{ &DMCHaveSample, 1, "5VSP" },

	{ &DMCSizeLatch, 1, "5SZL" },
	{ &DMCAddressLatch, 1, "5ADL" },
	{ &DMCFormat, 1, "5FMT" },
	{ &RawDALatch, 1, "RWDA" },
	
	 {DACRam, 0x10000, "6DCR"},
  {&DACacc, 4 | FCEUSTATE_RLSB, "6ACC"},
  {&DACAddress, 1, "6ADD"},
  {&DACSize, 1, "6SIZ"},
  {&DACSizeLo, 1, "6SHF"},
  {&DACSizeHi, 1, "6SHI"},

  {&DACFreq, 1, "6FRQ"},
  {&DACFreqLo, 1, "6FRL"},
  {&DACFreqHi, 1, "6FRH"},

  {&DACHaveDMA, 1, "6HVDM"},
  {&DACDMABuf, 1, "DACDB"},
  {&DACHaveSample, 1, "7HVSP"},

  {&DACAddressLo, 1, "6SZL"},
  {&DACAddressHi, 1, "6ADL"},
  {&DACFormat, 1, "6FMT"},
  
	//these are important for smooth sound after loading state
	{ &sqacc[0], sizeof(sqacc[0]) | FCEUSTATE_RLSB, "SAC1" },
	{ &sqacc[1], sizeof(sqacc[1]) | FCEUSTATE_RLSB, "SAC2" },
	{ &RectDutyCount[0], sizeof(RectDutyCount[0]) | FCEUSTATE_RLSB, "RCD1"},
	{ &RectDutyCount[1], sizeof(RectDutyCount[1]) | FCEUSTATE_RLSB, "RCD2"},
	{ &tristep, sizeof(tristep) | FCEUSTATE_RLSB, "TRIS"},
	{ &lq_triacc, sizeof(lq_triacc) | FCEUSTATE_RLSB, "TACC" },
	{ &lq_noiseacc, sizeof(lq_noiseacc) | FCEUSTATE_RLSB, "NACC" },
	
	//less important but still necessary
	{ &ChannelBC[0], sizeof(ChannelBC[0]) | FCEUSTATE_RLSB, "CBC1" },
	{ &ChannelBC[1], sizeof(ChannelBC[1]) | FCEUSTATE_RLSB, "CBC2" },
	{ &ChannelBC[2], sizeof(ChannelBC[2]) | FCEUSTATE_RLSB, "CBC3" },
	{ &ChannelBC[3], sizeof(ChannelBC[3]) | FCEUSTATE_RLSB, "CBC4" },
	{ &ChannelBC[4], sizeof(ChannelBC[4]) | FCEUSTATE_RLSB, "CBC5" },
	{ &sound_timestamp, sizeof(sound_timestamp) | FCEUSTATE_RLSB, "SNTS" },
	{ &soundtsoffs, sizeof(soundtsoffs) | FCEUSTATE_RLSB, "TSOF"},
	{ &wlcount[0], sizeof(wlcount[0]) | FCEUSTATE_RLSB, "WLC1" },
	{ &wlcount[1], sizeof(wlcount[1]) | FCEUSTATE_RLSB, "WLC2" },
	{ &wlcount[2], sizeof(wlcount[2]) | FCEUSTATE_RLSB, "WLC3" },
	{ &wlcount[3], sizeof(wlcount[3]) | FCEUSTATE_RLSB, "WLC4" },
	{ &sexyfilter_acc1, sizeof(sexyfilter_acc1) | FCEUSTATE_RLSB, "FAC1" },
	{ &sexyfilter_acc2, sizeof(sexyfilter_acc2) | FCEUSTATE_RLSB, "FAC2" },
	{ &lq_tcout, sizeof(lq_tcout) | FCEUSTATE_RLSB, "TCOU"},
	
	//wave buffer is used for filtering, only need first 17 values from it
	{ &Wave, 32 * sizeof(int32), "WAVE"},
{ 0 }
};

void FCEUSND_SaveState(void) {
}

void FCEUSND_LoadState(int version) {
	int i;
	LoadDMCPeriod(DMCFormat & 0xF);
	RawDALatch &= 0x7F;
	DMCAddress &= 0x7FFF;
 if (vrc6_snd)
 {
	 LoadDACPeriod(DACFormat & 0xF);
	 //  RawDALatch&=0x7F;
	 //  if(!exrams)
	 //  DACAddress&=0x7FFF;


	 if (DACacc <= 0)
		 DACacc = 1;
 }
	//minimal validation
	for (i = 0; i < 5; i++)
	{
		if (ChannelBC[i] < 0 || ChannelBC[i] > 15)
		{
			ChannelBC[i] = 0;
		}
	}
	for (i = 0; i < 4; i++)
	{
		if (wlcount[i] < 0 || wlcount[i] > 2048)
		{
			wlcount[i] = 2048;
		}
	}
	for (i = 0; i < 2; i++)
	{
		if (RectDutyCount[i] < 0 || RectDutyCount[i] > 7)
		{
			RectDutyCount[i] = 7;
		}
	}
	if (sound_timestamp < 0)
	{
		sound_timestamp = 0;
	}
	if (soundtsoffs < 0)
	{
		soundtsoffs = 0;
	}
	if (soundtsoffs + sound_timestamp >= soundtsinc)
	{
		soundtsoffs = 0;
		sound_timestamp = 0;
	}
	if (tristep > 32)
	{
		tristep &= 0x1F;
	}
}
