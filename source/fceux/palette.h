#ifndef _FCEU_PALETTE_H
#define _FCEU_PALETTE_H

typedef struct {
	uint8 r, g, b;
} pal;

extern pal *palo;
extern pal *pal512;				 //mod: control RGB by register $401B
extern pal palette_user[64*8*4]; //mod: is it working?
extern int ipalette;
void FCEU_ResetPalette(void);
extern void ApplyDeemphasisComplete(pal* pal512);
extern void WritePalette(void);
void FCEU_ResetPalette(void);
void FCEU_ResetMessages();
void FCEU_LoadGamePalette(void);
void FCEU_DrawNTSCControlBars(uint8 *XBuf);

#endif
