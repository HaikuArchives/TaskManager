/*
 * Copyright 2000 by Thomas Krammer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "pch.h"
#include "alert.h"
#include "my_assert.h"
#include "common.h"
#include "SystemInfo.h"
#include "CounterNamespaceImpl.h"
#include "ProcessView.h"
#include "Color.h"
#include "AsynchronousPopupMenu.h"
#include "DataProvider.h"
#include "TaskManager.h"
#include "MainWindow.h"
#include "Detector.h"
#include "LedView.h"
#include "Tooltip.h"
#include "DeskbarLedView.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DeskbarLedView"

// ==== globals ====

BView *create_deskbar_replicant()
{
	return new CDeskbarLedView(DESKBAR_REPLICANT_ID);
}

#if B_BEOS_VERSION >= B_BEOS_VERSION_5

// This function is called by the Deskbar to create a deskbar
// item of a deskbar addon.
BView _EXPORT *instantiate_deskbar_item()
{
	return create_deskbar_replicant();
}

#endif // B_BEOS_VERSION_5

// ==== CDataProviderInfo ====

CDataProviderInfo::CDataProviderInfo(CPulseView *_view, 
	IDataProvider *_dataProvider, float _maxValue, rgb_color _color)
{
	view = _view;
	color = _color;
	maxValue = _maxValue;
	dataProvider = _dataProvider;
	value = 0.0;
}

CDataProviderInfo::~CDataProviderInfo()
{
	delete dataProvider;

	dataProvider = NULL;	
	view = NULL;
}

void CDataProviderInfo::UpdateValue()
{
	value = 0.0;

	if(dataProvider && dataProvider->GetNextValue(value)) {
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_RELATIVE)
			value /= view->ReplicantPulseRate();
			
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_PERCENT) {
			value *= 100.0;
			
			value = MAX(MIN(value, 100.0), 0.0);
		}
	}
}

// ==== CDeskbarLedView ====

CDeskbarLedView::CDeskbarLedView(const char *name) :
	CPulseView(BRect(0,0,12,15), name, B_FOLLOW_NONE, B_WILL_DRAW)
{
	Init();
	
	replicant = true;
	menuVisible = tooltipDisabled = false;
}

CDeskbarLedView::CDeskbarLedView(BMessage *archive) : 
	CPulseView(archive)
{
	Init();
	
	menuVisible = tooltipDisabled = false;
}

CDeskbarLedView::~CDeskbarLedView()
{
	if(tooltip) {
		tooltip->Lock();
		tooltip->Quit();
	 }
	 
	delete detector;
	delete tooltipDataProvider;
	
	detector = NULL;
	tooltipDataProvider = NULL;
	tooltip = NULL;
}

void CDeskbarLedView::Init()
{
	if(global_Namespace == NULL) {
		InitGlobalNamespace();
	}

	ledOnColor		= DEFAULT_LED_ON_COLOR;
	ledOffColor		= DEFAULT_LED_OFF_COLOR;

	detector		= NULL;
	tooltip			= NULL;

	pulseRate		= NORMAL_PULSE_RATE;
	
	char path[255];

	system_info sysInfo;
	
	get_cached_system_info(&sysInfo, NULL);

	IDataProvider *dataProvider;

	for(int32 cpuNum=0 ; cpuNum<sysInfo.cpu_count ; cpuNum++) {
		sprintf(path, "/Total/CPU Usage/CPU %ld", cpuNum+1);

		dataProvider = global_Namespace->DataProvider(path);
		
		if(dataProvider != NULL) {
			dataProviderList.AddItem(
				new CDataProviderInfo(this, dataProvider, 100.0, ledOnColor));
		}
	}

	dataProvider = global_Namespace->DataProvider("/Total/CPU Usage/Average");
	
	tooltipDataProvider = new CDataProviderInfo(this, dataProvider, 100.0);
	
	float width  = MAX(12, (5*dataProviderList.CountItems())+2);
	float height = 15;
	
	ResizeTo(width, height);
}

BArchivable *CDeskbarLedView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CDeskbarLedView"))
		return NULL;
		
	return new CDeskbarLedView(archive);
}

status_t CDeskbarLedView::Archive(BMessage *data, bool deep) const
{
	RETURN_IF_FAILED( CPulseView::Archive(data, deep) );

	data->AddString("add_on", APP_SIGNATURE);
	//data->AddString("class", "CDeskbarLedView");
	return B_OK;
}

void CDeskbarLedView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_SHOW_MAIN_WINDOW:
			{
				status_t create_stat;
	
				BMessenger messenger(APP_SIGNATURE, -1, &create_stat);
				
				if(create_stat == B_OK) {
					// Messenger was sucessfully created. Forward message to
					// application
					messenger.SendMessage(MSG_SHOW_MAIN_WINDOW);	
				} else {
					// TaskManager isn't running. Relaunch it.
					be_roster->Launch(APP_SIGNATURE);
				}
			}
			
			break;
		case MSG_SELECT_AND_KILL_TEAM:
			// Show X-Kill Window
			{
				status_t status, launchStatus;
			
				// Lauch the TaskManager application if it isn't already running.
				if((launchStatus = be_roster->Launch(APP_SIGNATURE)) != B_OK && 
				   launchStatus != B_ALREADY_RUNNING) {
					show_alert(strerror(launchStatus));
					return;
				}
				
				BMessenger messenger(APP_SIGNATURE, -1, &status);

				if(status != B_OK) {
					show_alert(strerror(status));
					return;
				}
				
				if(launchStatus != B_ALREADY_RUNNING) {
					// Display the main window if the 
					// application was started.
					messenger.SendMessage(MSG_SHOW_MAIN_WINDOW);
				}
				
				// Forward message to application
				messenger.SendMessage(MSG_SELECT_AND_KILL_TEAM);
			}
			
			break;
		case MSG_SHOW_REPLICANT:
			{
				bool show;
			
				if(message->FindBool(MESSAGE_DATA_ID_SHOW, &show) == B_OK) {
					if(show) {
						// The replicant is already shown. (How can a replicant
						// receive messages, if it doesn't exist??).
						// Ignore message.
					} else {
						// Hide deskbar replicant.
						BDeskbar deskbar;
						
						deskbar.RemoveItem(DESKBAR_REPLICANT_ID);
					}
				}
			}
			break;
		case MSG_CONTEXT_MENU:
			{
				BPoint point = message->FindPoint("where");
			
				if(tooltip) tooltip->HideTooltip();
		
				CAsynchronousPopUpMenu *contextMenu = ContextMenu();			
				
				float menuWidth, menuHeight;
				
				contextMenu->GetPreferredSize(&menuWidth, &menuHeight);
				
				ConvertToScreen(&point);

				// allow the user to move the mouse by 3 points in every direction
				// without closing the menu, when the mouse is released.
				BRect clickToOpen(point.x-3, point.y-3, point.x+3, point.y+3);
				
				BScreen screen(Window());
				BRect screenRect = screen.Frame();

				if(point.x + menuWidth > screenRect.Width()) {
					point.x -= menuWidth;
				}

				if(point.y + menuHeight > screenRect.Height()) {
					point.y -= menuHeight;
				}

				contextMenu->SetAsyncAutoDestruct(true);
		
				// open menu (asynchonous)
				contextMenu->Go(point, true, clickToOpen, true);
			}
			break;
		case MSG_MENU_BEGINNING:
			menuVisible = true;
			break;
		case MSG_MENU_ENDED:
			menuVisible = false;
			break;
		default:
			CPulseView::MessageReceived(message);
	}
}

void CDeskbarLedView::AttachedToWindow()
{
	CPulseView::AttachedToWindow();

	detector = new CContextMenuDetector(this);

	tooltip = new CTooltip(this);

	// Start BLooper. This unlocks the object.
	// The window is opened on outside the visible area of the screen.
	// Therefore you won't see a fickering object apearing.
	tooltip->Show();
	tooltip->Hide();
	
	SetViewColor(CColor::Transparent);
}

void CDeskbarLedView::UpdateTooltip(bool forceShow, BPoint point)
{
	// Only show the tooltip if:
	//  - The tooltip and its dataprovider are created
	//  - The context menu isn't shown
	//  - The user didn't perform any user interaction with this view
	if(tooltip && tooltipDataProvider && !tooltipDisabled && !menuVisible) {
		if(!tooltip->Visible() && !forceShow)
			return;
	
		if(tooltipDataProvider->MaxValue() != 0) {
			float usage = tooltipDataProvider->CurrentValue();

			char buffer[255];
		
			sprintf(buffer, B_TRANSLATE("CPU Usage: %.0f %%"), usage);

			BRect  screenRect  = ConvertToScreen(Bounds());
			BPoint screenPoint = ConvertToScreen(BPoint(Bounds().Width()/2, Bounds().Height()/2));

			enumToolTipCorner corner = TTC_NONE;

			BDeskbar deskbar;
			
			switch(deskbar.Location()) {
				case B_DESKBAR_TOP:
					corner = TTC_RIGHT_TOP;
					break;
				case B_DESKBAR_BOTTOM:
					corner = TTC_RIGHT_BOTTOM;
					break;
				case B_DESKBAR_LEFT_BOTTOM:
					corner = TTC_LEFT_BOTTOM;
					break;
				case B_DESKBAR_RIGHT_BOTTOM:
					corner = TTC_RIGHT_BOTTOM;
					break;
				case B_DESKBAR_LEFT_TOP:
					corner = TTC_LEFT_TOP;
					break;
				case B_DESKBAR_RIGHT_TOP:
					corner = TTC_RIGHT_TOP;
					break;
			}
	
			tooltip->ShowTooltip(buffer, screenRect, screenPoint, corner);
		}
	}
}

void CDeskbarLedView::Pulse()
{
	UpdateTooltip(false);

	for(int i=0 ; i<dataProviderList.CountItems() ; i++) {
		CDataProviderInfo *dataProviderInfo = dataProviderList.ItemAt(i);
		
		dataProviderInfo->UpdateValue();
	}
	
	tooltipDataProvider->UpdateValue();

	Invalidate();
}

void CDeskbarLedView::Draw(BRect updateRect)
{
	float windowWidth  = Bounds().Width();
	float windowHeight = Bounds().Height();

	float dist;

	float barWidth = (windowWidth-2) / dataProviderList.CountItems();

	for(int i=0 ; i<dataProviderList.CountItems() ; i++) {
		CDataProviderInfo *dataProviderInfo = dataProviderList.ItemAt(i);
		
		float value = dataProviderInfo->CurrentValue();
		float maxValue = dataProviderInfo->MaxValue();

		dist = windowHeight - 1 - ceil((value / maxValue) * (windowHeight-2));

		float x1 = i*barWidth+1;
		float x2 = (i+1)*barWidth+1;
	
		// --- Draw OFF LEDs
	
		SetHighColor(DEFAULT_LED_VIEW_BG);
		SetLowColor(ledOffColor);
	
		for(int y=1 ; y<=dist ; y+=2) {
			FillRect(BRect(x1, y, x2, y), B_MIXED_COLORS);
		}
	
		// --- Draw background
	
		SetHighColor(ledOffColor);
	
		for(int y=2 ; y<=dist ; y+=2) {
			FillRect(BRect(x1, y, x2, y));
		}
	
		// --- Draw ON LEDs
	
		SetHighColor(dataProviderInfo->Color());
		
		if(dataProviderList.CountItems() == 1) {
			// Only one bar.
			// Fill the whole available area.
			FillRect(BRect(x1, dist, x2, windowHeight-1));
		} else {
			FillRect(BRect(x1+1, dist, x2-1, windowHeight-1));
			
			SetHighColor(ledOffColor);
			FillRect(BRect(x1, dist, x1, windowHeight-1));
			FillRect(BRect(x2, dist, x2, windowHeight-1));
		}
		
		// --- Draw border
	
		SetHighColor(CColor::BeShadow);
		MovePenTo(BPoint(windowWidth, 0));
		StrokeLine(BPoint(0, 0));
		StrokeLine(BPoint(0, windowHeight));
		
		SetHighColor(CColor::BeHighlight);
		MovePenTo(BPoint(windowWidth, 1));
		StrokeLine(BPoint(windowWidth, windowHeight));
		StrokeLine(BPoint(1, windowHeight));
	}
}

CAsynchronousPopUpMenu *CDeskbarLedView::ContextMenu()
{
	CAsynchronousPopUpMenu *popup = new CAsynchronousPopUpMenu("ContextMenu", false);

	popup->AddItem(
		new BMenuItem(
			B_TRANSLATE("Show TaskManager"),
			new BMessage(MSG_SHOW_MAIN_WINDOW)));

	popup->AddSeparatorItem();

	popup->AddItem(
		new BMenuItem(
			B_TRANSLATE("X-Kill..."),
			new BMessage(MSG_SELECT_AND_KILL_TEAM)));
			
	popup->AddSeparatorItem();

	BMessage *hideRepMsg = new BMessage(MSG_SHOW_REPLICANT);
	hideRepMsg->AddBool(MESSAGE_DATA_ID_SHOW, false);
	
	popup->AddItem(
		new BMenuItem(
			B_TRANSLATE("Hide Replicant"),
			hideRepMsg));

	popup->SetFont(be_plain_font);
	popup->SetTargetForItems(this);

	popup->SetNotificationTarget(this, NULL);
	
	return popup;	
}

void CDeskbarLedView::MouseDown(BPoint point)
{
	if(Window() == NULL)
		return;

	// Hide and disable the tooltip.
	// Otherwise it might interfere with the user's
	// interaction with this view.
	tooltip->HideTooltip();
	tooltipDisabled = true;

	if(detector)
		detector->MouseDown(point);
		
	BMessage *currentMsg = Window()->CurrentMessage();
	
	uint32 buttons;
	uint32 clicks;
		
	currentMsg->FindInt32("buttons", (int32 *)&buttons);
	currentMsg->FindInt32("clicks", (int32 *)&clicks);
		
	if(buttons & B_PRIMARY_MOUSE_BUTTON) {
		if(clicks >= 2) {
			// double click with primary mouse button
			
			// Send message to myself. This unifies the handling
			// of this double click and the context menu entry
			// "Show TaskManager".
			Window()->PostMessage(MSG_SHOW_MAIN_WINDOW, this);
		}
	}
}

void CDeskbarLedView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	switch(transit) {
		case B_ENTERED_VIEW:
			UpdateTooltip(true, point);
			break;
		case B_EXITED_VIEW:
			// Enable tooltip again.
			tooltipDisabled = false;
			break;
	}
}

void CDeskbarLedView::MouseUp(BPoint point)
{
	if(detector)
		detector->MouseUp(point);
}
