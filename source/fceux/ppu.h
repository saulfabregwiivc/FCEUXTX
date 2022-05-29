#ifndef _FCEU_PPU_H
#define _FCEU_PPU_H

void FCEUPPU_Init(void);
void FCEUPPU_Reset(void);
void FCEUPPU_Power(void);
int FCEUPPU_Loop(int skip);

void FCEUPPU_LineUpdate();
void FCEUPPU_SetVideoSystem(int w);

extern void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);
extern void FP_FASTAPASS(1) (*PPU_hook)(uint32 A);

/* For cart.c and banksw.h, mostly */
extern uint8 NTARAM[0x800], *vnapage[4];
extern uint8 NTARAM2[0x800], *vnapage2[4]; //mod: second background
extern uint8 NTARAM3[0x800], *vnapage3[4]; //mod: second background


extern uint8 NTATRIB[0x1000];
extern uint8 NTATRIB2[0x1000];
extern uint8 NTATRIB3[0x1000];
extern uint8 PPUNTARAM;
extern uint8 PPUCHRRAM;
extern int vt03_mode; //mod: 4bpp tiles, onebus and other mappers
extern int wscre;
extern int wss;
extern int wssub;
extern int wscre_32;
extern int wscre_32_2_3;
extern int wscre_new;
void FCEUPPU_SaveState(void);
void FCEUPPU_LoadState(int version);

extern int scanline;
extern uint16 PPU[4]; // mod: increase OAM counter? don't remember...

#endif
