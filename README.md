![GITHUB_BANNER](https://user-images.githubusercontent.com/49678526/69384191-f2ca8e00-0cbb-11ea-8049-14c60b72acf1.png)
# HeartOfDarkness-SDL
<p>Release version : 0.2.5</p>
<p><b>We have a Wiki ! Please check it out.</b></p>

* Go see the [Wiki](https://github.com/MaximLopez/HeartOfDarkness-SDL/wiki)
* Go to [INSTALLATION](https://github.com/MaximLopez/HeartOfDarkness-SDL/wiki/Installation) for more informations about the installation.
* Go to [CONTROL](https://github.com/MaximLopez/HeartOfDarkness-SDL/wiki/How-to-control) for the control's scheme.

## About
<p>The objectives of this port is to be 100% accurate to the real game and make it compatible with every devices (like PSP, Nintendo Switch, etc.) or any modern operating system (Linux, Windows, macOS, etc.) that can run SDL2.</p>
<p>HeartOfDarkness-SDL is a reimplementation of the engine of the used by the game "Heart of Darkness" developed by Amazing Studio.</p>
<p>This is a fork of "hode" by Gregory Montoir</p>

## Status 
<p>Menus are currently missing.</p>
<p>PSX datafiles can be used but sound (SPU ADPCM) and background screens (MDEC)
are not handled yet.</p>
<p>No GUI control on Android.</p>

## Installation
### Datafiles
<p>The original datafiles from the Windows release (demo or CD) are required.</p>

> List of files : 
* hod.paf (hod_demo.paf, hod_demo2.paf)
* setup.dat
* *_hod.lvl
* *_hod.sss
* *_hod.mst

### Configuration

<p>By default the engine will try to load the files from the current directory
and start the game from the first level.</p>
<p>These defaults can be changed using command line switches :</p>

    Usage: hode [OPTIONS]...
    --datapath=PATH   Path to data files (default '.')
    --savepath=PATH   Path to save files (default '.')
    --level=NUM       Start at level NUM
    --checkpoint=NUM  Start at checkpoint NUM

<p>Display and engine settings can be configured in the 'hode.ini' file.</p>
<p>Game progress is saved in 'setup.cfg', similar to the original engine.</p>

## Credits 
<p>All the team at Amazing Studio for possiby the best cinematic platformer ever developed.</p>
<p>Gregory Montoir for the hode engine.</p>
<p>Usineur for the Switch and Vita port.</p>

## URLs
* [Mobygames](https://www.mobygames.com/game/heart-of-darkness)
* [Website](http://heartofdarkness.ca/)
