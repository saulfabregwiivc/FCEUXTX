


uint8 *S2 = PALRAM;
static uint32 pixdata2 = 0;
static uint32 attl = 0;
uint8 *C2;
register uint8 cc2 = 0;
uint32 vadr2 = 0;
	uint8 *S3 = PALRAM;
static	uint32 pixdata3 = 0;
static	uint32 prior3 = 0;
static uint32 attl3 = 0;
uint8 *C3;
register uint8 cc3 = 0;
register uint8 prior_flag = 0;
uint32 vadr3;

	register uint8 zz2;
	register uint8 zz3;
	int XOC =  16 - XOffset;
	int XOC2 = 16 - XOffset2;
	int XOC3 = 16 - XOffset3;

uint32 pixdata_ex  = 0;
uint32 pixdata2_ex = 0;
uint32 pixdata3_ex = 0;
if (X1 >= 2)
	{


	

if(is_2_bg)pixdata2 = ppulut1[(pshift2[0] >> (XOC2)) & 0xFF] | ppulut2[(pshift2[1] >> (XOC2)) & 0xFF] | ppulut4[(pshift2[2] >> (XOC2)) & 0xFF] | ppulut5[(pshift2[3] >> (XOC2)) & 0xFF];
if(is_3_bg)pixdata3 = ppulut1[(pshift3[0] >> (XOC3)) & 0xFF] | ppulut2[(pshift3[1] >> (XOC3)) & 0xFF] | ppulut4[(pshift3[2] >> (XOC3)) & 0xFF] | ppulut5[(pshift3[3] >> (XOC3)) & 0xFF];





if(is_2_bg)pixdata2_ex = ppulut6[XOffset2| ((atlatch2&0xFF) << 3)];
if(is_3_bg)pixdata3_ex = ppulut6[XOffset3| ((atlatch3&0xFF) << 3)];


}

uint8 *C;
register uint8 cc;
uint16 vadr;

#ifndef PPUT_MMC5SP
	register uint8 zz;
#else
	uint8 xs, ys;
	xs = X1;
	ys = ((scanline >> 3) + MMC5HackSPScroll) & 0x1F;
	if (ys >= 0x1E) ys -= 0x1E;
#endif

if (X1 >= 2) {
	uint8 *S = PALRAM;
	uint32 pixdata;

	
if(vt03_mode)
	pixdata = ppulut1[(pshift[0] >> (XOC)) & 0xFF] | ppulut2[(pshift[1] >> (XOC)) & 0xFF] | ppulut4[(pshift[2] >> (XOC)) & 0xFF] | ppulut5[(pshift[3] >> (XOC)) & 0xFF];
else 
	pixdata = ppulut1[(pshift[0] >> (XOC)) & 0xFF] | ppulut2[(pshift[1] >> (XOC)) & 0xFF];

pixdata_ex = ppulut6[XOffset | ((atlatch&0xFF) << 3)];
for(int x =(X1*8-16); x<(X1*8-16+8); x++)
{
	
	priora_bg[x] = 0;
	priora_bg_3rd[x] = 0;
}



uint8 color_flag = vt03_mode ? 0xF : 0x3;
	if(wscre) prior3 = ppulut3[XOffset3| (prior_flag << 3)];

for(int x = 0; x<8; x++)
{
	
if(pixdata3&0xF)
{
P[x] = S[pixdata3 & 0xF | ((pixdata3_ex&shift_bg_2)<<shift_bg_1)]; 
if(!(prior3&0xF))priora_bg_3rd[X1*8-16+x] = 1;
}
else if(pixdata & color_flag)
{
P[x] = S[pixdata & color_flag | ((pixdata_ex&shift_bg_2)<<shift_bg_1)];
priora_bg[X1*8-16+x] = 1  | ((atrib&0x20)>>4); 
}
else if(pixdata2 & 0xF)
P[x] =  S[(pixdata2 & 0xF) | ((pixdata2_ex&shift_bg_2)<<shift_bg_1)];
else P[x] = S[0];
	prior3 >>= 4;
	pixdata >>= 4;
	pixdata2 >>= 4;
	pixdata3 >>= 4;
	pixdata_ex >>= 4;
	pixdata2_ex >>=4;
	pixdata3_ex >>=4;
	
}



}

if (X1 >= 2)
	P +=8;

if(vt03_mode)
{

	zz2 = RefreshAddr2 & 0x1F;
	C2 = vnapage2[(RefreshAddr2 >> 10) & 3];
	vadr2 = (C2[RefreshAddr2 & 0x3ff] << 5) + vofs2;	// Fetch name table byte.
	if((wscre)&&(NTATRIB2[RefreshAddr2&wscre_32_2_3]&0x10))vadr2 = ((C2[RefreshAddr2 & 0x3ff] << 5) + 0xC000)  | ((RefreshAddr2 >> 12) & 7);	// Fetch name table byte.

#ifdef PPUT_HOOK
	PPU_hook(0x2000 | (RefreshAddr2 & 0xfff));
#endif


		cc2 = C2[0x3c0 + (zz2 >> 2) + ((RefreshAddr2 & 0x380) >> 4)];	// Fetch attribute table byte.
		cc2 = ((cc2 >> ((zz2 & 2) + ((RefreshAddr2 & 0x40) >> 4))) & 3);
		if(wscre)cc2 = NTATRIB2[RefreshAddr2&wscre_32_2_3]&0x0F;



		atlatch2|=cc2<<8;




		C2 = &extra_ppu2[vadr2];


#ifdef PPUT_HOOK
	PPU_hook(vadr2);
#endif

        pshift2[0]|=C2[0];
        pshift2[1]|=C2[8];
		pshift2[2]|=C2[16];
		pshift2[3]|=C2[24];
        pshift2[0]<<=8;
        pshift2[1]<<=8;
		pshift2[2]<<=8;
		pshift2[3]<<=8;
	    atlatch2 >>= 4;
/*test some vars for armv8*/
static uint8 C3_extra[8];
	zz3 = RefreshAddr3 & 0x1F;
	C3 = vnapage3[(RefreshAddr3 >> 10) & 3];
	vadr3 = (C3[RefreshAddr3 & 0x3ff] << 5) + vofs3;	// Fetch name table byte.
	if((wscre)&&(NTATRIB3[RefreshAddr3&wscre_32_2_3]&0x10))vadr3 = ((C3[RefreshAddr3 & 0x3ff] << 5) + 0xC000)  | ((RefreshAddr3 >> 12) & 7);;	// Fetch name table byte.


   
	  
#ifdef PPUT_HOOK
	PPU_hook(0x2000 | (RefreshAddr3 & 0xfff));
#endif

		cc3 = C3[0x3c0 + (zz3 >> 2) + ((RefreshAddr3 & 0x380) >> 4)];	// Fetch attribute table byte.
		cc3 = ((cc3 >> ((zz3 & 2) + ((RefreshAddr3 & 0x40) >> 4))) & 3);
		if(wscre)
        {
        cc3 = NTATRIB3[RefreshAddr3&wscre_32_2_3]&0x0F;
		prior_flag =  (NTATRIB3[RefreshAddr3&wscre_32_2_3]&0x20)>>5;
        }


atlatch3|=cc3<<8;



		C3 = &extra_ppu3[vadr3];

#ifdef PPUT_HOOK
	PPU_hook(vadr3);
#endif

        pshift3[0]|=C3[0];
        pshift3[1]|=C3[8];
		pshift3[2]|=C3[16];
		pshift3[3]|=C3[24];
        pshift3[0]<<=8;
        pshift3[1]<<=8;
		pshift3[2]<<=8;
		pshift3[3]<<=8;
	    atlatch3 >>= 4;
		

}

//#ifdef PPUT_HOOK
//	PPU_hook(0x2000 | (RefreshAddr2 & 0xfff));
//#endif
#ifdef PPUT_MMC5SP
	vadr = (MMC5HackExNTARAMPtr[xs | (ys << 5)] << 4) + (vofs & 7);
#else
	zz = RefreshAddr & 0x1F;
	C = vnapage[(RefreshAddr >> 10) & 3];
	vadr = (C[RefreshAddr & 0x3ff] << 4) + vofs;	// Fetch name table byte.
	if(vt03_mode)vadr = (C[RefreshAddr & 0x3ff] << 5) + vofs;	// Fetch name table byte.
	if((wscre)&&(NTATRIB[RefreshAddr&wscre_32]&0x10))vadr = ((C[RefreshAddr & 0x3ff] << 5) + 0xC000)  | ((RefreshAddr >> 12) & 7);	// Fetch name table byte.
#endif

#ifdef PPUT_HOOK
	PPU_hook(0x2000 | (RefreshAddr & 0xfff));
#endif

#ifdef PPUT_MMC5SP
	cc = MMC5HackExNTARAMPtr[0x3c0 + (xs >> 2) + ((ys & 0x1C) << 1)];
	cc = ((cc >> ((xs & 2) + ((ys & 0x2) << 1))) & 3);
#else
	#ifdef PPUT_MMC5CHR1
		cc = (MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff] & 0xC0) >> 6;
	#else
		cc = C[0x3c0 + (zz >> 2) + ((RefreshAddr & 0x380) >> 4)];	// Fetch attribute table byte.
		cc = ((cc >> ((zz & 2) + ((RefreshAddr & 0x40) >> 4))) & 3);
		if(wscre)cc = NTATRIB[RefreshAddr&wscre_32]&0x0F;
	#endif
#endif
 atlatch|=cc<<8;
atrib = NTATRIB[RefreshAddr&wscre_32];


#ifdef PPUT_MMC5SP
	C = MMC5HackVROMPTR + vadr;
	C += ((MMC5HackSPPage & 0x3f & MMC5HackVROMMask) << 12);
#else
	#ifdef PPUT_MMC5CHR1
		C = MMC5HackVROMPTR;
		C += (((MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff]) & 0x3f & MMC5HackVROMMask) << 12) + (vadr & 0xfff);
		C += (MMC50x5130 & 0x3) << 18; //11-jun-2009 for kuja_killer
	#elif defined(PPUT_MMC5)
		C = MMC5BGVRAMADR(vadr);
	#else
		if(wscre)recalculate_hv(atrib, vadr,RefreshAddr );
				if(vadr<0x4000)
				{
						if(chrambank_V[vadr>>8])
						C = &chrramm[(chrambank_V[vadr>>8]<<8)|(vadr&0xFF)];
							else if(!wscre)
				C = VRAMADR(vadr);
				else C = &C_HV[0];
				}
				else
				{
				if(vt03_mmc3_flag && vadr <0xA000) C = VRAMADR(vadr);
				else if(wscre)C = &C_HV[0];
				else C = &extra_ppu[vadr];
				}
	#endif
#endif

#ifdef PPUT_HOOK
	PPU_hook(vadr);
#endif

        pshift[0]|=C[0];
        pshift[1]|=C[8];
	if(vt03_mode)	pshift[2]|=C[16];
	if(vt03_mode)	pshift[3]|=C[24];
        pshift[0]<<=8;
        pshift[1]<<=8;
	if(vt03_mode)	pshift[2]<<=8;
    if(vt03_mode)   pshift[3]<<=8;
	   atlatch >>= 4;

if ((RefreshAddr & 0x1f) == 0x1f)
	RefreshAddr ^= 0x41F;
else
	RefreshAddr++;

if ((RefreshAddr2 & 0x1f) == 0x1f)
	RefreshAddr2 ^= 0x41F;
else
	RefreshAddr2++;

if ((RefreshAddr3 & 0x1f) == 0x1f)
	RefreshAddr3 ^= 0x41F;
else
	RefreshAddr3++;
#ifdef PPUT_HOOK

	PPU_hook(0x2000 | (RefreshAddr & 0xfff));
#endif

