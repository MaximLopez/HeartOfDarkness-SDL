
#ifndef MENU_H__
#define MENU_H__

#include "intern.h"

struct Game;
struct PafPlayer;
struct Resource;
struct Video;

struct DatSpritesGroup {
	uint32_t currentFrameOffset; // 0
	uint32_t firstFrameOffset; // 4
	uint32_t size; // 8 following this header
	uint16_t count; // 12
	uint16_t num; // 14
} PACKED; // sizeof == 16

struct DatBitmap {
	uint32_t size; // 0
	uint32_t unk4; // 4
	// 8 lzw + 768 palette
} PACKED; // sizeof == 8

struct DatBitmapsGroup {
	uint8_t w;
	uint8_t h;
	uint8_t colors;
	uint8_t data;
	uint32_t offset; // 4
	uint32_t palette; // 8
} PACKED; // sizeof == 12

struct Menu {
	enum {
		kDelayMs = 30,
		kCheckpointLevelsCount = 8,
		kCutsceneIndexesCount = 22, // kPafAnimation_cinema + 1
		kOptionsCount = 19
	};

	Game *_g;
	PafPlayer *_paf;
	Resource *_res;
	Video *_video;

	SetupConfig *_config;
	int _checkpointNum;
	int _levelNum;

	DatSpritesGroup *_titleSprites;
	DatSpritesGroup *_playerSprites;
	const uint8_t *_titleBitmapData;
	uint32_t _titleBitmapSize;
	const uint8_t *_playerBitmapData;
	uint32_t _playerBitmapSize;
	uint32_t _optionsBitmapSize[kOptionsCount];
	const uint8_t *_optionsBitmapData[kOptionsCount];
	DatBitmapsGroup *_cutscenesBitmaps;
	const uint8_t *_cutscenesBitmapsData;
	DatBitmapsGroup *_checkpointsBitmaps[kCheckpointLevelsCount];
	const uint8_t *_checkpointsBitmapsData[kCheckpointLevelsCount];
	DatBitmapsGroup *_levelsBitmaps;
	const uint8_t *_levelsBitmapsData;
	DatSpritesGroup *_iconsSprites;
	const uint8_t *_iconsSpritesData;
	int _optionsButtonSpritesCount;
	const uint8_t *_optionsButtonSpritesData;
	DatSpritesGroup *_currentOptionButtonSprite;
	int _currentOptionButtonSound;

	const uint8_t *_digitsData;
	const uint8_t *_optionData;
	const uint8_t *_soundData;

	uint8_t _paletteBuffer[256 * 3];
	uint8_t _loadLevelButtonState;
	uint8_t _optionNum;
	int _lastLevelNum;
	int _lastLevelCheckpointNum[kCheckpointLevelsCount];
	uint8_t _condMask;
	uint8_t _loadCheckpointButtonState;
	int _bitmapCircularListIndex[3];
	int _cutsceneIndexesCount;
	int _cutsceneNum;
	uint8_t _loadCutsceneButtonState;
	int _cutsceneIndexes[kCutsceneIndexesCount];
	int _settingNum;
	int _controlsNum;
	int _joystickControlsNum;
	int _keyboardControlsNum;
	int _difficultyNum;
	int _soundNum;
	uint8_t _soundVolume;
	int _volumeState;
	int _soundCounter;
	int _soundTestSpriteNum;

	Menu(Game *g, PafPlayer *paf, Resource *res, Video *video);

	void setVolume();

	void loadData();

	int getSoundNum(int num, int index = 0) const;
	SssObject *playSound(int num);

	void drawBitmap(const uint8_t *data, uint32_t size, bool setPalette = false);

	void drawSprite(const DatSpritesGroup *spriteGroup, const uint8_t *ptr, uint32_t num, int x = -1, int y = -1);
	void drawSpriteAnim(DatSpritesGroup *spriteGroup, const uint8_t *ptr, uint32_t num);

	void pafCallback(int frameNum, const uint8_t *frameData);
	void refreshScreen();

	bool mainLoop();

	void drawTitleScreen(int option);
	int handleTitleScreen();
	void drawDigit(int x, int y, int num);
	void drawBitmap(const DatBitmapsGroup *bitmapsGroup, const uint8_t *bitmapData, int x, int y, int w, int h, uint8_t baseColor = 0);
	void setCurrentPlayer(int num);
	void setLevelCheckpoint(int num);
	void drawPlayerProgress(int state, int cursor);
	void handleAssignPlayer();
	void updateBitmapsCircularList(const DatBitmapsGroup *bitmapsGroup, const uint8_t *bitmapData, int num, int count);
	void drawBitmapsCircularList(const DatBitmapsGroup *bitmapsGroup, const uint8_t *bitmapData, int num, int count, bool updatePalette);
	void drawCheckpointScreen();
	void drawLevelScreen();
	void drawCutsceneScreen();
	void drawSettingsScreen();
	void handleSettingsScreen(int num);
	void drawControlsScreen();
	void handleControlsScreen(int num);
	void drawJoystickKeyCode(int num);
	void drawJoystickControlsScreen();
	void handleJoystickControlsScreen(int num);
	void drawKeyboardKeyCode(int num);
	void drawKeyboardControlsScreen();
	void handleKeyboardControlsScreen(int num);
	void drawDifficultyScreen();
	void handleDifficultyScreen(int num);
	void drawSoundScreen();
	void handleSoundScreen(int num);
	void changeToOption(int num);
	void handleLoadLevel(int num);
	void handleLoadCheckpoint(int num);
	void handleLoadCutscene(int num);
	bool handleOptions();
};

#endif // MENU_H__
