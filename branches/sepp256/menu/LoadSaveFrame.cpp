#include "MenuContext.h"
#include "LoadSaveFrame.h"
#include "../libgui/Button.h"
#include "../libgui/resources.h"
#include "../libgui/FocusManager.h"
#include "../libgui/CursorManager.h"


extern "C" {
#include "../gc_memory/Saves.h"
#include "../fileBrowser/fileBrowser.h"
#include "../fileBrowser/fileBrowser-libfat.h"
#include "../fileBrowser/fileBrowser-CARD.h"
#ifdef WII
#include "../fileBrowser/fileBrowser-WiiFS.h"
#endif
}

void Func_LoadSaveCardA();
void Func_LoadSaveCardB();
void Func_LoadSaveSD();
void Func_LoadSaveWiiFS();
void Func_ReturnFromLoadSaveFrame();

#define NUM_FRAME_BUTTONS 4
#define FRAME_BUTTONS loadSaveFrameButtons
#define FRAME_STRINGS loadSaveFrameStrings

static char FRAME_STRINGS[4][22] =
	{ "Memory Card in Slot A",
	  "Memory Card in Slot B",
	  "SD Card",
	  "Wii Filesystem"};

struct ButtonInfo
{
	menu::Button	*button;
	char*			buttonString;
	float			x;
	float			y;
	float			width;
	float			height;
	int				focusUp;
	int				focusDown;
	int				focusLeft;
	int				focusRight;
	ButtonFunc		clickedFunc;
	ButtonFunc		returnFunc;
} FRAME_BUTTONS[NUM_FRAME_BUTTONS] =
{ //	button	buttonString		x		y		width	height	Up	Dwn	Lft	Rt	clickFunc			returnFunc
	{	NULL,	FRAME_STRINGS[0],	150.0,	 50.0,	340.0,	40.0,	 3,	 1,	-1,	-1,	Func_LoadSaveCardA,	Func_ReturnFromLoadSaveFrame }, // Load From Card A
	{	NULL,	FRAME_STRINGS[1],	150.0,	150.0,	340.0,	40.0,	 0,	 2,	-1,	-1,	Func_LoadSaveCardB,	Func_ReturnFromLoadSaveFrame }, // Load From Card B
	{	NULL,	FRAME_STRINGS[2],	150.0,	250.0,	340.0,	40.0,	 1,	 3,	-1,	-1,	Func_LoadSaveSD,	Func_ReturnFromLoadSaveFrame }, // Load From SD
	{	NULL,	FRAME_STRINGS[3],	150.0,	350.0,	340.0,	40.0,	 2,	 0,	-1,	-1,	Func_LoadSaveWiiFS,	Func_ReturnFromLoadSaveFrame }, // Load From Wii FS
};

LoadSaveFrame::LoadSaveFrame()/*(MenuContext *menuContext)
		: menuContext(menuContext)*/
{
//	printf("MainFrame constructor\n");

	buttonImage = new menu::Image(ButtonTexture, 16, 16, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	buttonFocusImage = new menu::Image(ButtonFocusTexture, 16, 16, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	for (int i = 0; i < NUM_FRAME_BUTTONS; i++)
		FRAME_BUTTONS[i].button = new menu::Button(buttonImage, &FRAME_BUTTONS[i].buttonString, 
										FRAME_BUTTONS[i].x, FRAME_BUTTONS[i].y, 
										FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].height);

	for (int i = 0; i < NUM_FRAME_BUTTONS; i++)
	{
		FRAME_BUTTONS[i].button->setFocusImage(buttonFocusImage);
		if (FRAME_BUTTONS[i].focusUp != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_UP, FRAME_BUTTONS[FRAME_BUTTONS[i].focusUp].button);
		if (FRAME_BUTTONS[i].focusDown != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_DOWN, FRAME_BUTTONS[FRAME_BUTTONS[i].focusDown].button);
		if (FRAME_BUTTONS[i].focusLeft != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_LEFT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusLeft].button);
		if (FRAME_BUTTONS[i].focusRight != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_RIGHT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusRight].button);
		FRAME_BUTTONS[i].button->setActive(true);
		if (FRAME_BUTTONS[i].clickedFunc) FRAME_BUTTONS[i].button->setClicked(FRAME_BUTTONS[i].clickedFunc);
		if (FRAME_BUTTONS[i].returnFunc) FRAME_BUTTONS[i].button->setReturn(FRAME_BUTTONS[i].returnFunc);
		add(FRAME_BUTTONS[i].button);
		menu::Cursor::getInstance().addComponent(this, FRAME_BUTTONS[i].button, FRAME_BUTTONS[i].x, 
												FRAME_BUTTONS[i].x+FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].y, 
												FRAME_BUTTONS[i].y+FRAME_BUTTONS[i].height);
	}
	setDefaultFocus(FRAME_BUTTONS[0].button);
	setEnabled(true);

}

LoadSaveFrame::~LoadSaveFrame()
{
	for (int i = 0; i < NUM_FRAME_BUTTONS; i++)
	{
		menu::Cursor::getInstance().removeComponent(this, FRAME_BUTTONS[i].button);
		delete FRAME_BUTTONS[i].button;
	}
	delete buttonFocusImage;
	delete buttonImage;

}

extern BOOL hasLoadedROM;

void Func_LoadSaveCardA()
{
	if(!hasLoadedROM) return; // "Please load a ROM first";
	// Adjust saveFile pointers
//	saveFile_dir = (item_num%2) ? &saveDir_CARD_SlotB : &saveDir_CARD_SlotA;
	saveFile_dir	   = &saveDir_CARD_SlotA;
	saveFile_readFile  = fileBrowser_CARD_readFile;
	saveFile_writeFile = fileBrowser_CARD_writeFile;
	saveFile_init      = fileBrowser_CARD_init;
	saveFile_deinit    = fileBrowser_CARD_deinit;
		
	// Try loading everything
	int result = 0;
	saveFile_init(saveFile_dir);
	result += loadEeprom(saveFile_dir);
	result += loadSram(saveFile_dir);
	result += loadMempak(saveFile_dir);
	result += loadFlashram(saveFile_dir);
	saveFile_deinit(saveFile_dir);
	
//	return result ? "Loaded save from memcard" : "No saves found on memcard";
}

void Func_LoadSaveCardB()
{
	if(!hasLoadedROM) return; // "Please load a ROM first";
	// Adjust saveFile pointers
//	saveFile_dir = (item_num%2) ? &saveDir_CARD_SlotB : &saveDir_CARD_SlotA;
	saveFile_dir	   = &saveDir_CARD_SlotB;
	saveFile_readFile  = fileBrowser_CARD_readFile;
	saveFile_writeFile = fileBrowser_CARD_writeFile;
	saveFile_init      = fileBrowser_CARD_init;
	saveFile_deinit    = fileBrowser_CARD_deinit;
		
	// Try loading everything
	int result = 0;
	saveFile_init(saveFile_dir);
	result += loadEeprom(saveFile_dir);
	result += loadSram(saveFile_dir);
	result += loadMempak(saveFile_dir);
	result += loadFlashram(saveFile_dir);
	saveFile_deinit(saveFile_dir);
	
//	return result ? "Loaded save from memcard" : "No saves found on memcard";
}

void Func_LoadSaveSD()
{
	if(!hasLoadedROM) return; // "Please load a ROM first";
	// Adjust saveFile pointers
	saveFile_dir = &saveDir_libfat_Default;
	saveFile_readFile  = fileBrowser_libfat_readFile;
	saveFile_writeFile = fileBrowser_libfat_writeFile;
	saveFile_init      = fileBrowser_libfat_init;
	saveFile_deinit    = fileBrowser_libfat_deinit;
		
	// Try loading everything
	int result = 0;
	result += loadEeprom(saveFile_dir);
	result += loadSram(saveFile_dir);
	result += loadMempak(saveFile_dir);
	result += loadFlashram(saveFile_dir);
		
//	return result ? "Loaded save from SD card" : "No saves found on SD card";
}

void Func_LoadSaveWiiFS()
{
#ifdef WII
	if(!hasLoadedROM) return; // "Please load a ROM first";
	// Adjust saveFile pointers
	saveFile_dir       = &saveDir_WiiFS;
	saveFile_readFile  = fileBrowser_WiiFS_readFile;
	saveFile_writeFile = fileBrowser_WiiFS_writeFile;
	saveFile_init      = fileBrowser_WiiFS_init;
	saveFile_deinit    = fileBrowser_WiiFS_deinit;
		
	// Try loading everything
	int result = 0;
	saveFile_init(saveFile_dir);
	result += loadEeprom(saveFile_dir);
	result += loadSram(saveFile_dir);
	result += loadMempak(saveFile_dir);
	result += loadFlashram(saveFile_dir);
	saveFile_deinit(saveFile_dir);
		
//	return result ? "Loaded save from filesystem" : "No saves found on filesystem";
#endif
}

extern MenuContext *pMenuContext;

void Func_ReturnFromLoadSaveFrame()
{
	pMenuContext->setActiveFrame(MenuContext::FRAME_MAIN);
}