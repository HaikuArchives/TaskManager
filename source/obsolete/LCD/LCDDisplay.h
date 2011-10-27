#ifndef TM_LCD_DISPLAY_H
#define TM_LCD_DISPLAY_H

#include "DialogBaseEx.h"

class CLCDDisplayApp : public BApplication
{
	public:
	CLCDDisplayApp();
	virtual ~CLCDDisplayApp();

	virtual void ReadyToRun();
	
	protected:
};

class CLCDMainWindow : public BWindow
{
	public:
	CLCDMainWindow();
	virtual ~CLCDMainWindow() {}
	
	virtual bool QuitRequested();
};

class CLCDMainWindowView : 
	public CLocalizedDialogBase
{
	public:
	CLCDMainWindowView();
	virtual ~CLCDMainWindowView() {}
	
	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	
	virtual void MessageReceived(BMessage *msg);
	
	protected:
	BButton *addButton, *removeButton;
	BListView *listView;
	
	static const float LIST_VIEW_WIDTH;
	static const float LIST_VIEW_HEIGHT;
	static const float DIST;
};

const int32 MSG_ADD_BUTTON_PRESSED		= 'mADD';
const int32 MSG_REMOVE_BUTTON_PRESSED	= 'mREM';

#endif // TM_LCD_DISPLAY_H