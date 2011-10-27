#include "pch.h"
#include <device/SerialPort.h>
#include "Box.h"
#include "LCDDisplay.h"

// ====== main ======

int main(int argc, char **argv)
{
	#ifdef MEM_DEBUG
	SetNewLeakChecking(true);
	SetMallocLeakChecking(true);
	#endif // MEM_DEBUG

	new CLCDDisplayApp(); 

	be_app->Run();

	delete be_app;

	return 0;
}

// ==== CLCDDisplayApp ====

CLCDDisplayApp::CLCDDisplayApp() :
	BApplication("application/x-vnd.LCDDisplay")
{
}

CLCDDisplayApp::~CLCDDisplayApp()
{
}

void CLCDDisplayApp::ReadyToRun()
{
	CLCDMainWindow *mainWindow = new CLCDMainWindow();
	
	mainWindow->Show();
}

// ==== CLCDMainWindow ====

CLCDMainWindow::CLCDMainWindow() :
	BWindow(
		BRect(0, 0, 50, 50),
		"LCD Configuration",
		B_TITLED_WINDOW, 
		B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE,
		B_CURRENT_WORKSPACE)
{
	BView *view = new CLCDMainWindowView();

	float width, height;
	
	view->GetPreferredSize(&width, &height);
	view->ResizeTo(width, height);
	
	BScreen screen(this);
	
	BRect screenRect = screen.Frame();

	MoveTo((screenRect.Width()-width)/2, (screenRect.Height()-height)/2);
	ResizeTo(width, height);
	
	AddChild(view);
}

bool CLCDMainWindow::QuitRequested()
{
	// exit application
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	return true;
}

// ==== CLCDMainWindowView ====

const float CLCDMainWindowView::LIST_VIEW_WIDTH = 200;
const float CLCDMainWindowView::LIST_VIEW_HEIGHT = 300;
const float CLCDMainWindowView::DIST = 10;

CLCDMainWindowView::CLCDMainWindowView() :
	CLocalizedDialogBase(BRect(0,0,40,40), "MainWindowView", B_FOLLOW_ALL)
{
	float buttonWidth, buttonHeight;
	float prefButtonWidth, prefButtonHeight;

	BRect dummyRect(0, 0, 50, 50);

	addButton = new BButton(dummyRect, "AddButton", "Add...", 
		new BMessage(MSG_ADD_BUTTON_PRESSED), B_FOLLOW_TOP | B_FOLLOW_RIGHT);

	addButton->GetPreferredSize(&prefButtonWidth, &prefButtonHeight);
	
	buttonWidth = prefButtonWidth;
	buttonHeight = prefButtonHeight;
	
	removeButton = new BButton(dummyRect, "RemoveButton", "Remove", 
		new BMessage(MSG_REMOVE_BUTTON_PRESSED), B_FOLLOW_TOP | B_FOLLOW_RIGHT);

	removeButton->GetPreferredSize(&buttonWidth, &buttonHeight);
	
	buttonWidth = MAX(MAX(prefButtonWidth, buttonWidth), 120);
	
	float listViewWidth = LIST_VIEW_WIDTH + B_V_SCROLL_BAR_WIDTH;

	BRect perfSettingsRect(DIST, DIST, listViewWidth+buttonWidth+3*DIST, LIST_VIEW_HEIGHT+2*DIST);		

	CBox *perfSettingsBox = new CBox(perfSettingsRect, "PerfSettingsBox", B_FOLLOW_ALL);

	perfSettingsBox->SetLabel("Preformance Counter");
	
	BRect perfRect = perfSettingsBox->ClientRect();
	
	BRect listRect = perfRect;

	listRect.InsetBy(DIST, DIST);	
	listRect.right = listRect.left + LIST_VIEW_WIDTH - 2*DIST;

	listView = new BListView(listRect, "CounterList", B_SINGLE_SELECTION_LIST);

	float buttonLeft = perfRect.right - DIST - buttonWidth;

	addButton->MoveTo(buttonLeft, perfRect.top+DIST);
	addButton->ResizeTo(buttonWidth, buttonHeight);
	
	removeButton->MoveTo(buttonLeft, perfRect.top+2*DIST+buttonHeight);
	removeButton->ResizeTo(buttonWidth, buttonHeight);		

	BScrollView *listScrollView = 
		new BScrollView("CounterListScroller", listView, B_FOLLOW_ALL, 0, false, true);
		
	perfSettingsBox->AddChild(listScrollView);
	perfSettingsBox->AddChild(addButton);
	perfSettingsBox->AddChild(removeButton);
	
	AddChild(perfSettingsBox);
	
	BRect lcdSettingsRect(DIST, perfSettingsRect.bottom+DIST, perfSettingsRect.right, perfSettingsRect.bottom+DIST+300);		

	CBox *lcdSettingsBox = new CBox(lcdSettingsRect, "LCDSettings", B_FOLLOW_LEFT | B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	
	lcdSettingsBox->SetLabel("LCD Display");

	BMenu *serialPortMenu = new BMenu("<pick one>");

	BSerialPort serial;
	char devName[B_OS_NAME_LENGTH];

	for (int32 i=0 ; i<serial.CountDevices() ; i++) {
		serial.GetDeviceName(i, devName);
		
		serialPortMenu->AddItem(new BMenuItem(devName, NULL));
	}
	
	BMenu *sizeMenu = new BMenu("<pick one>");
	
	sizeMenu->AddItem(new BMenuItem("20x2", NULL));
	sizeMenu->AddItem(new BMenuItem("40x2", NULL));
	sizeMenu->AddItem(new BMenuItem("20x4", NULL));
	sizeMenu->AddItem(new BMenuItem("40x4", NULL));
	
	BMenu *updateSpeedMenu = new BMenu("<pick one>");
	
	updateSpeedMenu->AddItem(new BMenuItem("Fast", NULL));
	updateSpeedMenu->AddItem(new BMenuItem("Normal", NULL));
	updateSpeedMenu->AddItem(new BMenuItem("Slow", NULL));
	
	BRect menuFieldRect = lcdSettingsBox->ClientRect();
	
	font_height fh;
	
	be_plain_font->GetHeight(&fh);
	
	float fontHeight = fh.ascent + fh.descent;
	
	menuFieldRect.InsetBy(DIST, DIST);
	
	BRect serialPortMenuFieldRect = menuFieldRect;
	serialPortMenuFieldRect.bottom = serialPortMenuFieldRect.top + fontHeight;
	
	BMenuField *serialPortMenuField = new BMenuField(serialPortMenuFieldRect, "SerialPort", "Serial Port:", serialPortMenu);

	BRect sizeMenuFieldRect = menuFieldRect;
	sizeMenuFieldRect.top = serialPortMenuFieldRect.bottom + DIST;
	sizeMenuFieldRect.bottom = sizeMenuFieldRect.top + fontHeight;

	BMenuField *sizeMenuField = new BMenuField(sizeMenuFieldRect, "Size", "Size:", sizeMenu);
	
	BRect updateSpeedMenuFieldRect = menuFieldRect;
	updateSpeedMenuFieldRect.top = sizeMenuFieldRect.bottom + DIST;
	updateSpeedMenuFieldRect.bottom = updateSpeedMenuFieldRect.top + fontHeight;
	
	BMenuField *updateSpeedMenuField = new BMenuField(updateSpeedMenuFieldRect, "UpdateSpeed", "Update Speed:", updateSpeedMenu);
	
	lcdSettingsBox->AddChild(serialPortMenuField);
	lcdSettingsBox->AddChild(sizeMenuField);
	lcdSettingsBox->AddChild(updateSpeedMenuField);
	
	AddChild(lcdSettingsBox);
}

void CLCDMainWindowView::GetPreferredSize(float *width, float *height)
{
	float baseWidth, baseHeight;
	
	CLocalizedDialogBase::GetPreferredSize(&baseWidth, &baseHeight);

	float buttonWidth = 
		MAX(addButton->Bounds().Width(), removeButton->Bounds().Width());
	
	*width = MAX(3*DIST + LIST_VIEW_WIDTH + B_V_SCROLL_BAR_WIDTH + buttonWidth, baseWidth);

	*height = 2*DIST + LIST_VIEW_HEIGHT + baseHeight + 340;
}

void CLCDMainWindowView::AttachedToWindow()
{
	CLocalizedDialogBase::AttachedToWindow();
	
	addButton->SetTarget(this);
	removeButton->SetTarget(this);
}

void CLCDMainWindowView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case MSG_ADD_BUTTON_PRESSED:
			break;
			
		case MSG_REMOVE_BUTTON_PRESSED:
			break;
			
		default:
			CLocalizedDialogBase::MessageReceived(msg);
	}
}
