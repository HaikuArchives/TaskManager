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
#include "MRUSelectFileView.h"
#include "TaskManagerPrefs.h"
#include "CreateTeamWindow.h"

#include <Catalog.h>
#include <Locale.h>
#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "CreateTeamWindow"

// protected constructor
CCreateTeamWindow::CCreateTeamWindow() :
	CSingletonWindow(
		BRect(10,10,50,50), 
		B_TRANSLATE("New Team"), 
		B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_V_RESIZABLE |
		B_ASYNCHRONOUS_CONTROLS,
		B_CURRENT_WORKSPACE)	
{
	// create view on dummy postion
	BView *mruView = new CMRUSelectFileView(BRect(0,0,100,100), 
											new CTaskManagerPrefs());
		
	// calculate window size
	float width=320;
	float minWidth, height;
		
	mruView->GetPreferredSize(&minWidth, &height);

	width = MAX(width, minWidth);
		
	BWindow *mainWindow = be_app->WindowAt(0);
	BRect screenRect = BScreen(mainWindow).Frame();
	
	// This is a really stupid solution for a really stupid
	// problem:
	// I can't calculate the size without creating an object.
	// but I can't create an object without size. Therefore
	// I create first a dummy object and calculate the size,
	// delete it and create a new object with the correct size.
	delete mruView;
	mruView = new CMRUSelectFileView(BRect(0,0,width,height), 
										new CTaskManagerPrefs());
	
	// center window
	float wx = (screenRect.Width()-width)/2;
	float wy = (screenRect.Height()-height)/2;

	MoveTo(wx, wy);
	ResizeTo(width, height);
	
	AddChild(mruView);
	SetSizeLimits(minWidth, screenRect.Width(), height, height);
	Show();										 
}

CCreateTeamWindow::~CCreateTeamWindow()
{
	RemoveFromList(ClassName());
}

CCreateTeamWindow *CCreateTeamWindow::CreateInstance()
{
	// Initialize to quiet compiler.
	CCreateTeamWindow *window = NULL;

	return CreateSingleton(window, "CCreateTeamWindow");
}
