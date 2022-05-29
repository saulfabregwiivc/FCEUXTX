/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2012 CaH4e3
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

#include "mapinc.h"

static uint8 preg, mirr;
static int isbf9097;
static SFORMAT StateRegs[] =
{
	{ &preg, 1, "PREG" },
	{ &mirr, 1, "MIRR" },
	{ 0 }
};

static void Sync(void) {
	setprg16(0x8000, preg & 0xF);
	
	
	if (isbf9097)
		 setmirror(mirr?MI_1:MI_0);
}

static DECLFW(M71Write) {
// Mirroring = (V >> 4) & 1;
preg = V;
Sync();

	//if ((A & 0xF000) == 0x9000)
	//	mirr = MI_0 + ((V >> 4) & 1);	/* 2-in-1, some carts are normal hardwire V/H mirror, some uses mapper selectable 0/1 mirror */
	//else
	//	preg = V;
	//Sync();
}
static DECLFW(WriteLo) {
// Mirroring = (V >> 4) & 1;
 mirr = (V >> 4) & 1;
 Sync();
	//if ((A & 0xF000) == 0x9000)
	//	mirr = MI_0 + ((V >> 4) & 1);	/* 2-in-1, some carts are normal hardwire V/H mirror, some uses mapper selectable 0/1 mirror */
	//else
	//	preg = V;
	//Sync();
}

static void M71Power(void) {
	mirr = 0;
	preg = 0;
	Sync();
	setprg16(0xC000, 0xF);
	setchr8(0);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);
	SetWriteHandler(0xC000, 0xFFFF, M71Write);
}

static void StateRestore(int version) {
	Sync();
}

void Mapper71_Init(CartInfo *info) {
	info->Power = M71Power;
	GameStateRestore = StateRestore;
isbf9097 = 0;
	AddExState(&StateRegs, ~0, 0, 0);
}

int BIC62_Init(CartInfo *info)
{
 Mapper71_Init(info);

 SetWriteHandler(0x8000, 0xBFFF, WriteLo);
 isbf9097 = 1;
 PRGmask16[0] &= 0x7;
 return(1);
}
