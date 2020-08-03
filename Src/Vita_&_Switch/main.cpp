/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#if !defined(PSP) && !defined(WII)
#include <SDL.h>
#endif
#if defined(WII)
#include <fat.h>
#endif
#include <getopt.h>
#include <sys/stat.h>

#include "3p/inih/ini.h"

#include "game.h"
#include "menu.h"
#include "mixer.h"
#include "paf.h"
#include "util.h"
#include "resource.h"
#include "system.h"
#include "video.h"

#ifdef __SWITCH__
#include <switch.h>
#endif

static const char *_title = "Heart of Darkness";

#ifdef __vita__
static const char *_configIni = "ux0:data/hode/hode.ini";
#else
static const char *_configIni = "hode.ini";
#endif

static const char *_usage =
	"hode - Heart of Darkness Interpreter\n"
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default '.')\n"
	"  --savepath=PATH   Path to save files (default '.')\n"
	"  --level=NUM       Start at level NUM\n"
	"  --checkpoint=NUM  Start at checkpoint NUM\n"
;

static bool _fullscreen = false;
static bool _widescreen = false;

static const bool _runBenchmark = false;
static bool _runMenu = true;

static void lockAudio(int flag) {
	if (flag) {
		g_system->lockAudio();
	} else {
		g_system->unlockAudio();
	}
}

static void mixAudio(void *userdata, int16_t *buf, int len) {
	((Game *)userdata)->mixAudio(buf, len);
}

static void setupAudio(Game *g) {
	g->_mix._lock = lockAudio;
	AudioCallback cb;
	cb.proc = mixAudio;
	cb.userdata = g;
	g_system->startAudio(cb);
}

static const char *_defaultDataPath = ".";

static const char *_defaultSavePath = ".";

static const char *_levelNames[] = {
	"rock",
	"fort",
	"pwr1",
	"isld",
	"lava",
	"pwr2",
	"lar1",
	"lar2",
	"dark",
	0
};

static bool configBool(const char *value) {
	return strcasecmp(value, "true") == 0 || (strlen(value) == 2 && (value[0] == 't' || value[0] == '1'));
}

static int handleConfigIni(void *userdata, const char *section, const char *name, const char *value) {
	Game *g = (Game *)userdata;
	// fprintf(stdout, "config.ini: section '%s' name '%s' value '%s'\n", section, name, value);
	if (strcmp(section, "engine") == 0) {
		if (strcmp(name, "disable_paf") == 0) {
			if (!g->_paf->_skipCutscenes) { // .paf file not found
				g->_paf->_skipCutscenes = configBool(value);
			}
		} else if (strcmp(name, "disable_mst") == 0) {
			g->_mstDisabled = configBool(value);
		} else if (strcmp(name, "disable_sss") == 0) {
			g->_sssDisabled = configBool(value);
		} else if (strcmp(name, "disable_menu") == 0) {
			_runMenu = !configBool(value);
		} else if (strcmp(name, "max_active_sounds") == 0) {
			g->_playingSssObjectsMax = atoi(value);
		} else if (strcmp(name, "difficulty") == 0) {
			g->_difficulty = atoi(value);
		} else if (strcmp(name, "frame_duration") == 0) {
			g->_frameMs = atoi(value);
		} else if (strcmp(name, "loading_screen") == 0) {
			g->_loadingScreenEnabled = configBool(value);
		}
	} else if (strcmp(section, "display") == 0) {
		if (strcmp(name, "scale_factor") == 0) {
			const int scale = atoi(value);
			g_system->setScaler(0, scale);
		} else if (strcmp(name, "scale_algorithm") == 0) {
			g_system->setScaler(value, 0);
		} else if (strcmp(name, "gamma") == 0) {
			g_system->setGamma(atof(value));
		} else if (strcmp(name, "fullscreen") == 0) {
			_fullscreen = configBool(value);
		} else if (strcmp(name, "widescreen") == 0) {
			_widescreen = configBool(value);
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
#ifdef __SWITCH__
	socketInitializeDefault();
	nxlinkStdio();
#endif
#ifdef __vita__
	const char *dataPath = "ux0:data/hode";
	const char *savePath = "ux0:data/hode";
#else
	char *dataPath = 0;
	char *savePath = 0;
#endif
	int level = 0;
	int checkpoint = 0;
	bool resume = true; // resume game from 'setup.cfg'

	g_debugMask = 0; //kDebug_GAME | kDebug_RESOURCE | kDebug_SOUND | kDebug_MONSTER;
	int cheats = 0;

#ifdef WII
	fatInitDefault();
	static const char *pathsWII[] = {
		"sd:/hode",
		"usb:/hode",
		0
	};
	for (int i = 0; pathsWII[i]; ++i) {
		struct stat st;
		if (stat(pathsWII[i], &st) == 0 && S_ISDIR(st.st_mode)) {
			dataPath = strdup(pathsWII[i]);
			savePath = strdup(pathsWII[i]);
			break;
		}
	}
#else
#if !defined(PSP)
	if (argc == 2) {
		// data path as the only command line argument
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISDIR(st.st_mode)) {
			dataPath = strdup(argv[1]);
		}
	}
	while (1) {
		static struct option options[] = {
			{ "datapath",   required_argument, 0, 1 },
			{ "savepath",   required_argument, 0, 2 },
			{ "level",      required_argument, 0, 3 },
			{ "checkpoint", required_argument, 0, 4 },
			{ "debug",      required_argument, 0, 5 },
			{ "cheats",     required_argument, 0, 6 },
			{ 0, 0, 0, 0 },
		};
		int index;
		const int c = getopt_long(argc, argv, "", options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 1:
			dataPath = strdup(optarg);
			break;
		case 2:
			savePath = strdup(optarg);
			break;
		case 3:
			if (optarg[0] >= '0' && optarg[0] <= '9') {
				level = atoi(optarg);
			} else {
				for (int i = 0; _levelNames[i]; ++i) {
					if (strcmp(_levelNames[i], optarg) == 0) {
						level = i;
						break;
					}
				}
			}
			resume = false;
			break;
		case 4:
			checkpoint = atoi(optarg);
			resume = false;
			break;
		case 5:
			g_debugMask |= atoi(optarg);
			break;
		case 6:
			cheats |= atoi(optarg);
			break;
		default:
			fprintf(stdout, "%s\n", _usage);
			return -1;
		}
	}
#endif
#endif
	Game *g = new Game(dataPath ? dataPath : _defaultDataPath, savePath ? savePath : _defaultSavePath, cheats);
	ini_parse(_configIni, handleConfigIni, g);
	if (_runBenchmark) {
		g->benchmarkCpu();
	}
	// load setup.dat and detects if these are PC or PSX datafiles
	g->_res->loadSetupDat();
	const bool isPsx = g->_res->_isPsx;
	g_system->init(_title, Video::W, Video::H, _fullscreen, _widescreen, isPsx);
	setupAudio(g);
	g->loadSetupCfg(resume);
	bool runGame = true;
	g->_video->init(isPsx);
	g->displayLoadingScreen();
	if (_runMenu && resume && !isPsx) {
		Menu *m = new Menu(g, g->_paf, g->_res, g->_video);
		runGame = m->mainLoop();
		delete m;
	}
	if (runGame && !g_system->inp.quit) {
		bool levelChanged = false;
		do {
			g->displayLoadingScreen();
			g->mainLoop(level, checkpoint, levelChanged);
			// do not save progress when game is started from a specific level/checkpoint
			if (resume) {
				g->saveSetupCfg();
			}
			if (g->_res->_isDemo) {
				break;
			}
			level = g->_currentLevel + 1;
			checkpoint = 0;
			levelChanged = true;
		} while (!g_system->inp.quit && level < kLvl_test);
	}
	g_system->stopAudio();
	g_system->destroy();
	delete g;
#ifndef __vita__
	free(dataPath);
	free(savePath);
#endif
#ifdef WII
	fatUnmount("sd:/");
	fatUnmount("usb:/");
#endif
#ifdef __SWITCH__
	socketExit();
#endif
	return 0;
}
