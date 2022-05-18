/****************************************************************************
 * FCE Ultra
 * Nintendo Wii/GameCube Port
 *
 * Tantric 2008-2022
 *
 * pad.h
 *
 * Controller management
 ****************************************************************************/

#ifndef _PAD_H_
#define _PAD_H_

#include <gctypes.h>
#include <wiiuse/wpad.h>

#include "utils/wiidrc.h"

#define PI 				3.14159265f
#define PADCAL			50
#define WIIDRCCAL		20
#define MAXJP 			11
#define RAPID_A 		256
#define RAPID_B			512

extern int playerMapping[4];
extern u32 btnmap[2][6][12];

void SetControllers();
void ResetControls(int cc = -1, int wc = -1);
void GetJoy();
bool MenuRequested();
void SetupPads();
void UpdatePads();
#ifdef HW_RVL
char* GetUSBControllerInfo();
#endif

#endif
