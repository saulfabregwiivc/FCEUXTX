/****************************************************************************
 * FCE Ultra
 * Nintendo Wii/GameCube Port
 *
 * Tantric 2008-2022
 * Tanooki 2019-2023
 *
 * fceuxtx.h
 *
 * This file controls overall program flow. Most things start and end here!
 ****************************************************************************/

#ifndef _FCEUXTX_H_
#define _FCEUXTX_H_

#include <unistd.h>

#include "fceux/driver.h"

#define APPNAME 			"FCEUX TX"
#define APPVERSION 			"1.1.7"
#define APPFOLDER 			"fceuxtx"
#define PREF_FILE_NAME		"settings.xml"

#define MAXPATHLEN 1024
#define NOTSILENT 0
#define SILENT 1

const char pathPrefix[9][8] =
{ "", "sd:/", "usb:/", "dvd:/", "carda:/", "cardb:/", "port2:/" };

enum {
	DEVICE_AUTO,
	DEVICE_SD,
	DEVICE_USB,
	DEVICE_DVD,
	DEVICE_SD_SLOTA,
	DEVICE_SD_SLOTB,
	DEVICE_SD_PORT2
};

enum {
	FILE_RAM,
	FILE_STATE,
	FILE_ROM,
	FILE_CHEAT
};

enum 
{
	CTRL_PAD,
	CTRL_ZAPPER,
	CTRL_PAD2,
	CTRL_PAD4,
	CTRL_LENGTH
};

const char ctrlName[6][20] =
{ "NES Controller", "NES Zapper", "NES Controllers (2)", "NES Controllers (4)" };

enum {
	FASTFORWARD_BUTTON_RSTICK = 0,
	FASTFORWARD_BUTTON_A,
	FASTFORWARD_BUTTON_B,
	FASTFORWARD_BUTTON_X,
	FASTFORWARD_BUTTON_Y,
	FASTFORWARD_BUTTON_L,
	FASTFORWARD_BUTTON_R,
	FASTFORWARD_BUTTON_ZL,
	FASTFORWARD_BUTTON_ZR,
	FASTFORWARD_BUTTON_Z,
	FASTFORWARD_BUTTON_C,
	FASTFORWARD_BUTTON_1,
	FASTFORWARD_BUTTON_2,
	FASTFORWARD_BUTTON_PLUS,
	FASTFORWARD_BUTTON_MINUS
};

enum {
	LANG_JAPANESE = 0,
	LANG_ENGLISH,
	LANG_GERMAN,
	LANG_FRENCH,
	LANG_SPANISH,
	LANG_ITALIAN,
	LANG_DUTCH,
	LANG_SIMP_CHINESE,
	LANG_TRAD_CHINESE,
	LANG_KOREAN,
	LANG_PORTUGUESE,
	LANG_BRAZILIAN_PORTUGUESE,
	LANG_TURKISH,
	LANG_LENGTH
};

struct SGCSettings{
	int		AutoLoad;
    int		AutoSave;
    int		LoadMethod; // For ROMS: Auto, SD, DVD, USB
	int		SaveMethod; // For SRAM, Freeze, Prefs: Auto, SD, USB
	char	LoadFolder[MAXPATHLEN];     // Path to game files
	char	LastFileLoaded[MAXPATHLEN]; // Last file loaded filename
	char	SaveFolder[MAXPATHLEN];     // Path to save files
	char	CheatFolder[MAXPATHLEN];    // Path to cheat files
	char	ScreenshotsFolder[MAXPATHLEN]; // Path to screenshots files
	char	CoverFolder[MAXPATHLEN]; 	// Path to cover files
	char	ArtworkFolder[MAXPATHLEN]; 	// Path to artwork files
	int		HideRAMSaving;
	int		AutoloadGame;

	float	zoomHor; // Horizontal zoom amount
	float	zoomVert; // Vertical zoom amount
	int		videomode; // 0 - Automatic, 1 - NTSC (480i), 2 - Progressive (480p), 3 - Progressive (576p), 4 - PAL (50Hz), 5 - PAL (60Hz)
	int		render;		// 0 - Default, 1 - Original (240p)
	int		bilinear;    // Bilinear filtering
	int		hideoverscan; // 0 = None, 1 = Vertical, 2 = Horizontal, 3 = Both
	int		Controller;
	int		FastForward;
	int		FastForwardButton;
	int		currpal;
	int		ntsccolor;
	int		crosshair;
	int		region;
	int		aspect;
	int		xshift;		// Video output shift
	int		yshift;
	int		soundvolume;
	int		soundquality;
	int		lowpass;
	int		swapduty;
	int		overclock;
	int		nospritelimit;
	int		gamegenie;
	int		WiimoteOrientation;
	int		ExitAction;
	int		MusicVolume;
	int		SFXVolume;
	int 	language;
	int		PreviewImage;

};

void ExitApp();
void ShutdownWii();
bool SupportedIOS(u32 ios);
bool SaneIOS(u32 ios);
extern struct SGCSettings GCSettings;
extern int ScreenshotRequested;
extern int ConfigRequested;
extern int ShutdownRequested;
extern int ExitRequested;
extern char appPath[];
extern int frameskip;
extern int fskip;
extern int fskipc;
extern int fastforward;
extern bool romLoaded;
extern bool isWiiVC;
static inline bool IsWiiU(void)
{
	return ((*(vu16*)0xCD8005A0 == 0xCAFE) || isWiiVC);
}
static inline bool IsWiiUFastCPU(void)
{
	return ((*(vu16*)0xCD8005A0 == 0xCAFE) && ((*(vu32*)0xCD8005B0 & 0x20) == 0));
}
#endif
