FCEUX TX
===================

FCEUX TX is a fork of FCE Ultra GX: https://github.com/dborth/fceugx
(Under GPL License)

Update History
--------------

---FCEUX TX 1.1.7---
---January 15 2023---

- Updated to the latest FCEUX (git 4dd8943)
- Updated to the latest FCE Ultra GX (git 145490b)
- Added option to turn on/off NTSC color (only for NTSC games with default color palette)
- Added automatic swapping of de-emphasis bits color based on PAL or NTSC region
- Added automatic use of default color palette with VS games (other palettes are not supported)
- Added progressive PAL video mode (576p)
- Removed Triangle/Square 1/Square 2/Noise/PCM volume settings (keep sound volume setting)
  Too messy and requires high sound quality (restored low sound quality by default)
- Fixed clicking with a gamepad on the Game Genie button (thanks InfiniteBlue!)
- Reorganized menu options
- Renamed turbo mode option to "fast forward" (turbo mode name is reminiscent of auto fire)
- Removed catalan and updated all other language files
- Fixed sound effects volume setting and replace enter.ogg audio file
- Updated UI PNG images
- Added DualShock 3 (wired) support based on SickSaxis lib by xerpi (thanks niuus and JabuPL!)
  How to use:
  1. Start up FCEUX TX
  2. Start a game with your Wiimote/GC Controller/etc
  3. Press the PS button on your DualShock 3 and connect it to your Wii with a USB cable
  4. It should connect, one led on the controller should light up
  5. Done, you can play now on a DualShock 3 Controller

---FCEUX TX 1.1.6---
---September 25 2022---

- Updated to the latest FCEUX (git 06b53e9)
- Changed low sound quality to high by default (to take full advantage of new sound settings)
- Added option to choose Sound/Triangle/Square 1/Square 2/Noise/PCM volume in audio settings
- Updated french language file

---FCEUX TX 1.1.5---
---September 4 2022---

- Updated to the latest FCEUX (git 069727c)
  Except git c37f86d (does not compile)
- Updated to the latest FCE Ultra GX (git 2246982)
- Removed exit sound of UI (sometimes causes a small audio crackle when returning to game)
- Removed filtered render and replaced render option by an option to turn on/off 240p output
- Added option to turn on/off bilinear filtering in video setting (works with 240p output)
- Added 2x-Postrender/2x-VBlank PPU overclocking option in emulation hacks settings
- Added new emulation hacks settings button and moved screenshot button to main game menu
- Fixed not loading vertical video shift setting preference
- Updated video code
- Updated all language files
- Updated and changed position of some text and UI PNG images

---FCEUX TX 1.1.4---
---June 23 2022---

- Updated to the latest FCEUX (git 14c2152)
- Updated to the latest FCE Ultra GX 3.5.2
- Added mapper 126 support for Power Joy 84-in-1
- Added option to hide/show "Save RAM" button in save menu (thanks InfiniteBlue!)
- Added credits button in main menu settings (thanks InfiniteBlue!)
- Updated credits
- Updated all language files
- Updated UI PNG images
- Compiled with latest devkitPPC and libfat with UStealth Mod (thanks SaulFabre!)

---FCEUX TX 1.1.3---
---May 17 2022---

- Updated to the latest FCEUX 2.6.4 (git def5768)
- Updated to the latest FCE Ultra GX (git 10b4ff8)
- Added mapper 218 support for Magic Floor
- Restored turbo mode and added button remapping feature and submenu (thanks InfiniteBlue!)
- Added option to choose sound quality (Low/High/Highest) in audio settings
- Added option to turn it on/off low pass filtering in audio settings to simulate NES RF sound
- Added SMB Game & Watch color palette
- Restored PAL 60Hz video mode
- Updated video code
- Reorganized video rendering mode to Original/Unfiltered/Filtered (Unfiltered by default)
- Reorganized menu video settings
- Updated and cleaned up all language files
- Updated UI PNG images
- Compiled with latest devkitPPC

---FCEUX TX 1.1.2---
---September 1 2021---

- Updated to the latest FCEUX 2.4.0 (git f45ba2f)
- Updated to the latest FCE Ultra GX (git d07dc9e)
- Removed Soft and Sharp video filtering (no difference with filtered and unfiltered)
- Reorganized color palettes order
- Removed Wavebeam color palette (Smooth V2 is very similar)
- Added Digital Prime color palette by FBX and rename Smootz to Smooth V2
- Replaced old PAL color palette by that of r57Shell (more accurate)
- Removed rumble function from menu (your batteries will last longer)
- Updated Korean translation (thanks DDinghoya!)
- Updated UI PNG images
- Compiled with latest devkitPPC/libogc
- Updated Forwarder Channel (1.5)

---FCEUX TX 1.1.1---
---April 13 2021---

- Updated to the latest FCEUX (git 029cea5)
- Fixed inverted names of Soft and Sharp video filtering
- Optimized swap duty cycles setting code
- Added new Magnum/Smootz color palettes by FBX
  Magnum is recommended for crt monitors and Smootz for digital displays
- Removed deprecated color palettes
- Removed "Auto" of name of saves and option to enable/disable it
- Updated Forwarder Channel (1.4)

---FCEUX TX 1.1.0---
---March 25 2021---

- Updated to the latest FCEUX (git 4be5045)
- Updated to the latest FCE Ultra GX 3.5.1 (git 3f6ea5b)
- Updated UI PNG images
- Updated French translation
- Updated Forwarder Channel (1.3)
- Added new alternate Forwarder Channel (based on Flo_o's channel)
- Added compiled GameCube version

---FCEUX TX 1.0.9---
---February 22 2021---

- Updated to the latest FCEUX 2.3.0 (git c544c13)
- Updated to the latest FCE Ultra GX 3.5.0 (git 373e12b)
- Removed SMB support (SMB 1.0 has a major security issue due to flaws in the protocol)
- Added Mayflash 2-port Snes USB adapter support (thanks EthanArmbrust!)
- Removed Retrode 2 and Hornet USB controller support (not really useful for Wii)
- Removed PocketNES interoperability
- Removed Dendy support (priority to PAL 576i/288p video mode which does not suit Dendy mode)
- Changed Game Timing setting name to Region
- Changed max game image dimensions to 640x512 to fix screenshots
- Changed UI stripes size
- Reorganized credits
- Reverted moved app version in credits box info
- Reworked UI PNG images (improved colors, shadow effects and logo)
- Added option to turn it on/off swap duty cycles to simulate sound of some NES clones
- Added audio settings in game menu (thanks Bladeoner!)
- Removed menu music (use a bg_music.ogg file in root fceuxtx folder)
- Updated French translation
- Changed root folder name to fceuxtx
- Reorganized color palettes order
- Added original Virtual Console color palette
- Removed experimental Game Boy DMG color palette
- Compiled with latest devkitPPC/libogc
- Updated Forwarder Channel (1.2)

---FCEUX TX 1.0.8---
---June 16 2020---

- Updated to the latest FCEUX (git 8490dd9)
- Updated UI PNG images
- Updated Forwarder Channel (1.1)

---FCEUX TX 1.0.7---
---June 5 2020---

- Updated UI PNG images
- Added new Forwarder Channel (based on qwertymodo's channel)
- Updated to the latest FCE Ultra GX (git 917dae0)
- Added experimental Game Boy DMG color palette
- Moved app version in main menu to credits box info
- Changed fceugx root folder name to fceux
- Changed 1UP name to FCEUX TX and add new logo
- Removed RGBSource's RetronHD color palette
- Updated to the latest FCEUX (git d89ead7)
- Disabled low pass audio filter

---FCEUX TX 1.0.6---
---February 26 2020---

- Updated Wii/vWii Forwarder Channel (1.2)
- Updated UI PNG images

---FCEUX TX 1.0.5---
---February 20 2020---

- Updated UI PNG images
- Enabled low pass audio filter
- Updated to the latest FCE Ultra GX (git a93dc16)

---FCEUX TX 1.0.4---
---February 13 2020---

- Added RGBSource's RetronHD color palette
- Changed Mod name: welcome to FCE Ultra GX 1UP
- Modified credits box PNG image
- Reorganized credits

---FCEUX TX 1.0.3---
---February 10 2020---

- Reorganized credits
- Added Xbox 360/Hornet controller support
- Updated to the latest FCEUX (git 747fba7)
- Removed "Power On" and Insert/Eject disk messages
- Changed audio buffering values
- Updated to the latest FCE Ultra GX (git 82006ec)
- Compiled with latest libraries

---FCEUX TX 1.0.2---
---December 28 2019---

- Updated to the latest FCE Ultra GX (git a8caf23)
- Added Restored Wii VC color palette (thanks SuperrSonic!)
  Restored version adjusts brightness levels significantly
- Updated to the latest FCEUX (git 0b4be4b)
- Updated UI color and added Wii/vWii Forwarder Channel (1.1)
- Removed turbo mode on right joystick

---FCEUX TX 1.0.1---
---November 24 2019---

- Updated to the latest FCEUX (git 88d7f39)
- Added vWii Forwarder Channel for Wii U
- Updated PPU color palettes (libretro fceumm)
- Added A+B+START trigger to go back to menu with Classic Controller
  Work with NES/Snes Classic Mini controllers
- Fixed dialogue box text color to screen position and controller settings
- Corrected PAL audio sample rate

---FCEUX TX 1.0.0---
---October 13 2019---

- Added separate PAL audio sample rate
- Increased sound volume in games
- Added Retrode support (thanks revvv!)
- Changed default mapping of buttons A and B
- Added button mapping for NES Zapper (thanks niuus!)
- Corrected the PAL noise frequency table
- Added all PAL regions detection
- Removed PAL 60Hz video mode
- Corrected the PAL 50Hz video mode resolution to 576i/288p
- Enabled low quality sound
- Updated to the latest FCEUX (git 0fc18be)
- Updated color palettes
- Changed UI and Forwarder Channel colors
- Based on the latest commit of FCE Ultra GX
