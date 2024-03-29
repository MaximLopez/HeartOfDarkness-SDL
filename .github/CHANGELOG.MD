# CHANGELOG
> release 0.2.9.1
* Clean up repo
* Move to release 0.2.9.F from Gregory Montoir
* Change to README.md
* Activate Git LFS on the repo

> release 0.2.9.0
* Clean up repo
* Add Nintendo Wii and PSP executable
* Add Xbox's port from Modern Vintage Gamer
* Move to release 0.2.9C from Gregory Montoir

> release 0.2.5.1
* First Linux release

> release 0.2.5
* Preparing Linux port
* Preparing macOS port
* Cleaning repo
* Update from GREGORY MONTOIR
* Update Switch and Vita's port into 0.2.5

> release 0.2.4.1
* Add Usineur port's
* Update repo
* Add CONTROL.MD
* Add INSTALLATION.MD
* Delete RELEASE folder

> relase 0.2.4
* Update from GREGORY MONTOIR

> release 0.2
* Update from GREGORY MONTOIR

> release 0.1
* Initial repo

## ORIGINAL CHANGELOG FROM GREGORY MONTOIR
> release 0.2.9c
* added original sound limiter (16 channels)
* fixed checkpoint 7 for 'lar1' screen 19
* fixed some bounding box checks
* fixed sound glitches when resuming the game after a cutscene

> release 0.2.9b
* added PSX hint screens
* added 'volume' submenu
* fixed sounds volume and panning
* fixed walk on chain bridge in 'lar1' screen 3
* fixed gate not closing in 'lar2' screen 8

> release 0.2.9
* added PSP and Wii platforms specific code
* added support for PC demo v1.0 data files
* fixed screen state for 'fort' screens 16 and 17
* fixed checkpoint for 'pwr1' screen 21
* fixed PAF animation glitches with last frame

> release 0.2.8
* added PSX background overlays (MDEC)
* fixed crash playing PAF animation #3 with Italian PC data files

> release 0.2.7
* added 'projection' submenu
* added PSX backgrounds (MDEC)
* fixed menu on big endian platforms

> release 0.2.6
* added initial code for menu
* added PSX sounds (SPU ADPCM)
* fixed skull animation in 'rock' screen 18

> release 0.2.5
* fixed spiders and worms movement
* several fixes for big endian platforms

> release 0.2.4
* fixed Amigo walk path in 'pwr2' screen 7
* fixed glowing stone puzzle in 'lar2'
* fixed missing trigger for switch #0 in 'lar1'

> release 0.2.3
* fixed double spectre fireball animations in 'lar1' and 'lar2'
* fixed level transition from 'pwr1' to 'isld'
* fixed screen state for 'lava' screen 15

> release 0.2.2
* fixed lever in 'lava' level screen 2
* fixed masking with plants in 'pwr1' level
* fixed missing explosion animation with fireballs
* fixed screen mask in 'lar2' level screen 7

> release 0.2.1
* added initial code for PSX data files detection
* fixed sprite infinite looping in 'pwr1' level screen 23
* several fixes to gates and switches in 'lar1' and 'lar2' levels

> release 0.2.0
* added missing postScreenUpdate callbacks for 'lava' level
* fixed crash at beginning of 'lar1' level
* fixed gamma ramp with widescreen borders
* several fixes to gates handling in 'lar1' and 'lar2' levels

> release 0.1.9
* fixed fireballs not hitting Andy
* fixed fishes movement in 'pwr1' and 'pwr2' levels
* fixed several bounding box clipping checks
* fixed sounds panning

> release 0.1.8
* added specific callbacks for 'lar1' and 'lar2' levels
* fixed infinite loop in .mst op27
* several fixes to sound code

> release 0.1.7
* added 16:9 widescreen (blur)
* added missing preScreenUpdate callbacks for 'lar1' level
* fixed .sss version 12 loading (demo v1.4)

> release 0.1.6
* fixed Andy animation glitches when climbing
* fixed Andy apnea animation in 'pwr1' level
* fixed duplicated cutscene playback when switching levels
* fixed postScreenUpdate callback for 'isld' level screen 15
* fixed postScreenUpdate callback for 'pwr1' level screen 6

> release 0.1.5
* added 'firefly' object in 'fort' level
* added collision with monsters
* added missing explosion sprites
* added postScreenUpdate callbacks for 'isld' level
* fixed monsters hug

> release 0.1.4
* added detection for sector files (demo)
* fixed hint screens displaying too often
* fixed monsters initial position
* fixed skull animation in 'rock' level

> release 0.1.3
* added initial code for 'Special Powers'
* implemented .mst opcodes for 'rock' level
* fixed palette fading when dying

> release 0.1.2
* added .mst screen area triggers
* added parsing for .mst files
* added postScreenUpdate callbacks for 'lar1' level

> release 0.1.1
* added audio playback for PAF animations	
* added postScreenUpdate callbacks for 'lava' level
* added preScreenUpdate callbacks for 'isld' level
* added transform screen tables for levels 'pwr1' and 'pwr2'
* Fixed timing for PAF animations (10 fps)

> release 0.1.0
* first release
