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
#include "common.h"

#include "Tab.h"
#include "FlickerFreeTabView.h"
#include "GraphView.h"
#include "LedView.h"
#include "UsageView.h"
#include "PerformanceView.h"
#include "IconStringItem.h"
#include "ProcessView.h"
#include "SettingsWindow.h"
#include "TaskManager.h"
#include "CreateTeamWindow.h"
#include "TaskManagerPrefs.h"
#include "DeskbarLedView.h"

#include "MainWindow.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"

// ====== globals ======

const char * const DESKBAR_REPLICANT_ID		= APP_SIGNATURE ":LedView";

// ====== CMainWindowView ======

CMainWindowView::CMainWindowView(BRect rect) :
	BView(rect, "MainWindowView", B_FOLLOW_ALL_SIDES,  B_FRAME_EVENTS)		
{
	tabView = new CFlickerFreeTabView(Bounds(), "MainViewTab", B_WIDTH_FROM_WIDEST);
	
	BRect tabRect = tabView->Bounds();
	tabRect.bottom -= tabView->TabHeight();

	BTab *usageTab = new CBugFixedTab();
	
	tabView->AddTab(new CUsageView(tabRect), usageTab);
	usageTab->SetLabel(B_TRANSLATE("Usage"));

	BTab *teamTab = new CBugFixedTab();
	
	tabView->AddTab(new CProcessView(tabRect), teamTab);
	teamTab->SetLabel(B_TRANSLATE("Teams"));

	BTab *perfTab = new CBugFixedTab();
	
	tabView->AddTab(new CPerformanceView(tabRect), perfTab);
	perfTab->SetLabel(B_TRANSLATE("Performance"));

	// This code ensures that the tabbed views are added to the view hierachry
	// before there're first selected.
	for(int i=0 ; i<tabView->CountTabs() ; i++) {
		BView *tabbedView = tabView->TabAt(i)->View();
	
		tabView->ContainerView()->AddChild(tabbedView);
		
		tabbedView->Hide();
	}

	AddChild(tabView);  
}

void CMainWindowView::AttachedToWindow()
{
	CTaskManagerPrefs prefs;

	int32 selTab;
	
	prefs.Read(PREF_SELECTED_TAB, selTab, 0);

	tabView->Select(selTab);
	
	BView::AttachedToWindow();
}

void CMainWindowView::DetachedFromWindow()
{
	CTaskManagerPrefs prefs;
	
	prefs.Write(PREF_SELECTED_TAB, tabView->Selection());
	
	BView::DetachedFromWindow();
}

// ====== CMainWindow ======

CMainWindow::CMainWindow(BRect rect, bigtime_t updateSpeed, bool showReplicant, bool showOnAllWorkspaces) : 
	BWindow(rect, APP_NAME, B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 
		B_ASYNCHRONOUS_CONTROLS, showOnAllWorkspaces ? B_ALL_WORKSPACES : B_CURRENT_WORKSPACE)
{
	BMenuBar *menuBar = new BMenuBar(BRect(0,0,0,0), NULL);
		
	BMenu *fileSubMenu        = new BMenu(B_TRANSLATE("File"));
	BMenu *viewSubMenu        = new BMenu(B_TRANSLATE("View"));
	BMenu *updateSpeedSubMenu = new BMenu(B_TRANSLATE("Update Speed"));
	
	fileSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("NewTeam..."), new BMessage(MSG_CREATE_TEAM), 'N'));
	fileSubMenu->AddSeparatorItem();
	fileSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED)));
	fileSubMenu->ItemAt(2)->SetTarget(be_app_messenger);
	fileSubMenu->AddSeparatorItem();
	fileSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));
	
	updateSpeedSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Slow"), new BMessage(MSG_SLOW_UPDATE)));
	updateSpeedSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Normal"), new BMessage(MSG_NORMAL_UPDATE)));
	updateSpeedSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Fast"), new BMessage(MSG_FAST_UPDATE)));

	if(PulseRate() == SLOW_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(0)->SetMarked(true);
	} else if(PulseRate() == NORMAL_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(1)->SetMarked(true);
	} else if(PulseRate() == FAST_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(2)->SetMarked(true);
	} else {
		// User defined update speed.
	}

	SetPulseRate(updateSpeed);

	updateSpeedSubMenu->SetRadioMode(true);
	
	viewSubMenu->AddItem(updateSpeedSubMenu);
	viewSubMenu->AddSeparatorItem();
	viewSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Show in Deskbar"), new BMessage(MSG_SHOW_REPLICANT)));
	viewSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Show in all Workspaces"), new BMessage(MSG_SHOW_IN_ALL_WORKSPACES)));
	viewSubMenu->AddSeparatorItem();
	viewSubMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Settings..."), new BMessage(MSG_SHOW_SETTINGS_WINDOW)));
	
	if(showReplicant)
		ShowDeskbarReplicant();
	
	menuBar->AddItem(fileSubMenu);
	menuBar->AddItem(viewSubMenu);
	
	AddChild(menuBar);
	SetKeyMenuBar(menuBar);
	
	BRect viewRect = Bounds();
	
	viewRect.top += menuBar->Bounds().Height() + 1;

	AddChild(new CMainWindowView(viewRect));	
}

// MarkSource
// marks the source menu item of a command message
void CMainWindow::MarkSource(BMessage *message, bool mark)
{
	BMenuItem *item;
	
	if(message->FindPointer("source", (void **)&item) == B_OK) {
		item->SetMarked(mark);
	}
}

void CMainWindow::MenusBeginning()
{
	// "View/Show in Deskbar"
	BMenuItem *showInDeskbarItem	= KeyMenuBar()->ItemAt(1)->Submenu()->ItemAt(2);

	// "View/Show in all Workspaces"
	BMenuItem *showInAllWorkspaces	= KeyMenuBar()->ItemAt(1)->Submenu()->ItemAt(3);
	
	showInDeskbarItem->SetMarked(ReplicantVisible());
	showInAllWorkspaces->SetMarked(Workspaces() == B_ALL_WORKSPACES);
}

void CMainWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_SLOW_UPDATE:
			SetPulseRate(SLOW_PULSE_RATE);
			break;
		case MSG_NORMAL_UPDATE:
			SetPulseRate(NORMAL_PULSE_RATE);
			break;
		case MSG_FAST_UPDATE:
			SetPulseRate(FAST_PULSE_RATE);
			break;
		case MSG_SHOW_REPLICANT:
			if(ReplicantVisible()) {
				HideDeskbarReplicant();
			} else {
				ShowDeskbarReplicant();
			}
			
			MarkSource(message, ReplicantVisible());
			
			break;
		case MSG_CREATE_TEAM:
			CreateTeam();
			break;
		case MSG_SHOW_SETTINGS_WINDOW:	
			ShowSettingsWindow();
			break;
		case MSG_SHOW_IN_ALL_WORKSPACES:
			{
				uint32 workspaces = (Workspaces() == B_ALL_WORKSPACES) ? B_CURRENT_WORKSPACE : B_ALL_WORKSPACES;
		
				SetWorkspaces(workspaces);
			}
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
} 

void CMainWindow::CreateTeam()
{
	CCreateTeamWindow::CreateInstance();
}

void CMainWindow::ShowSettingsWindow()
{
	CSettingsWindow::CreateInstance();
}

bool CMainWindow::ReplicantVisible()
{
	return BDeskbar().HasItem(DESKBAR_REPLICANT_ID);
}

status_t CMainWindow::ShowDeskbarReplicant(bool showAlert)
{
	if(ReplicantVisible())
		return B_OK;
	
	BDeskbar deskBar;
	
	status_t err;
	
#if B_BEOS_VERSION >= B_BEOS_VERSION_5
	// Get app_info about myself.
	app_info appInfo;
	be_app->GetAppInfo(&appInfo);
	
	// Add this application as deskbar addon.
	err = deskBar.AddItem(&appInfo.ref);
#else
	BView *deskBarLedView = create_deskbar_replicant();

	// Add view as deskbar replicant.
	err = deskBar.AddItem(deskBarLedView);

	delete deskBarLedView;
#endif
	
	if(err != B_OK && showAlert) {
		BString message (B_TRANSLATE("Can't add deskbar replicant.\nReason: \0"));
		
		message << strerror(err);
	
		show_alert(message);
	}
	
	return err;
}

status_t CMainWindow::HideDeskbarReplicant(bool showAlert)
{
	if(!ReplicantVisible())
		return B_OK;
	
	BDeskbar deskBar;
		
	status_t err;
		
	if((err = deskBar.RemoveItem(DESKBAR_REPLICANT_ID)) != B_OK && showAlert) {
		BString message (B_TRANSLATE("Can't remove deskbar replicant.\nReason: \0"));
		
		message << strerror(err);
	
		show_alert(message);
	}
	
	return err;
}

bool CMainWindow::QuitRequested()
{
	// save settings
	CTaskManagerPrefs prefs;
	
	prefs.SetMainWindowRect(Frame());
	prefs.SetPulseRate(PulseRate());
	prefs.SetShowDeskbarReplicant(ReplicantVisible());
	prefs.SetShowInAllWorkspaces(Workspaces() == B_ALL_WORKSPACES);

	// remove deskbar replicant
	if(prefs.HideDeskbarReplicantOnClose())
		HideDeskbarReplicant();
	
	// exit application
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	return true;
}
