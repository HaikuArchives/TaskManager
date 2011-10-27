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
#include "LocalizationHelper.h"
#include "ProcessView.h"
#include "PrefCheckBox.h"
#include "SettingsView.h"
#include "MainWindow.h"
#include "ColumnListViewEx.h"
#include "SettingsWindow.h"

// protected contructor
CSettingsWindow::CSettingsWindow() :
	CSingletonWindow(
		BRect(0,0,50,50),
		CLocalizationHelper::GetDefaultInstance()->String("SettingsWindow.Title"),
		B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS,
		B_CURRENT_WORKSPACE)
{
	BRect dummy(0, 0, 50, 50);

	// --- Create views
	
	CSettingsView *settingsView = new CSettingsView(dummy, "Settings");

	CSettingsGroup *miscGroup = new CSettingsGroup(dummy, "MISC Settings");

	miscGroup->AddChild(new CPrefCheckBox(
								dummy, 
								PREF_HIDE_DESKBAR_REPLICANT_ON_CLOSE, 
								CLocalizationHelper::GetDefaultInstance()->String("SettingsWindow.DontHideCheckBox.Label"), 
								NULL,		// message
								true,		// default value, if pref setting is not present
								true		// invert value (also default value!)
								));

	BMessage *hideSystemTeamsProp = new BMessage;
	
	hideSystemTeamsProp->AddSpecifier(PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS);
	hideSystemTeamsProp->AddSpecifier("View",   (int32)TAB_ID_TEAMS);		// "Teams" tab
	hideSystemTeamsProp->AddSpecifier("View",   (int32)0);		// container for tabbed views
	hideSystemTeamsProp->AddSpecifier("View",   "MainViewTab");
	hideSystemTeamsProp->AddSpecifier("View",   "MainWindowView");
	hideSystemTeamsProp->AddSpecifier("Window", "TaskManager");

	miscGroup->AddChild(new CScriptingPrefCheckBox(
								dummy,
								hideSystemTeamsProp,
								PREF_HIDE_SYSTEM_TEAMS,
								CLocalizationHelper::GetDefaultInstance()->String("SettingsWindow.HideSystemTeamsCheckBox.Label"),
								NULL,		// message
								false,		// default value
								false		// invert value
								));

	settingsView->InternalView()->AddChild(miscGroup);
	
	CSettingsGroup *teamColumnGroup = new CSettingsGroup(dummy, "Team Column Settings", 2);
	teamColumnGroup->SetLabel(CLocalizationHelper::GetDefaultInstance()->String("SettingsWindow.ColumnsGroup.Label"));
	
	for(column_info *column=team_view_colomn_info ; column->key[0] != 0 ; column++) {
		char prefName[255];

		BMessage *scriptMessage = new BMessage;
		
		// Add specifiers for the property "Visible" of the Column(index) in
		// the column list view.
		scriptMessage->AddSpecifier(COLUMN_LIST_VIEW_PROP_COLUMN_VISIBLE);
		scriptMessage->AddSpecifier("Column", column->index);
		scriptMessage->AddSpecifier("View",	  "ProcessColumnListView");
		scriptMessage->AddSpecifier("View",	  (int32)0);		// column list view container
		scriptMessage->AddSpecifier("View",   (int32)1);		// "Teams" tab (don't use name, because it depends on the selected language).
		scriptMessage->AddSpecifier("View",   (int32)0);		// container for tabbed views
		scriptMessage->AddSpecifier("View",   "MainViewTab");
		scriptMessage->AddSpecifier("View",   "MainWindowView");
		scriptMessage->AddSpecifier("Window", "TaskManager");
		
		sprintf(prefName, "%s%ld", PREF_COLUMN_VISIBLE_PREFIX, column->index);
	
		const char *title = CLocalizationHelper::GetDefaultInstance()->String(column->key);
	
		teamColumnGroup->AddChild(new CScriptingPrefCheckBox(
										dummy,						// frame
										scriptMessage,				// script message
										prefName,					// pref setting
										title,						// name and title
										NULL,						// message
										column->defaultVisible,		// default value
										false						// invert value
										));
	}

	settingsView->InternalView()->AddChild(teamColumnGroup);
	
	settingsView->ResizeToPreferred();

	BRect screenRect = BScreen(be_app->WindowAt(0)).Frame();

	// --- Center window
	
	float width  = settingsView->Bounds().Width();
	float height = settingsView->Bounds().Height();
	
	float wx = (screenRect.Width()-width)/2;
	float wy = (screenRect.Height()-height)/2;

	MoveTo(wx, wy);
	ResizeTo(width, height);
	AddChild(settingsView);
	
	// --- Show window
	
	Show();
}

CSettingsWindow::~CSettingsWindow()
{
	RemoveFromList(ClassName());
}

CSettingsWindow *CSettingsWindow::CreateInstance()
{
	// Initialize to quiet compiler.
	CSettingsWindow *window = NULL;

	return CreateSingleton(window, "CSettingsWindow");
}