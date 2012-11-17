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
#include "ArrowButton.h"
#include "FileGlyphMenuItem.h"
#include "Preferences.h"
#include "CommandLineParser.h"
#include "MRUSelectFileView.h"

#include "my_assert.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MRUSelectFileView"

// message data identifier
const char * const MESSAGE_DATA_ID_MRU_PATH	 = "MRU:Path";
const char * const MESSAGE_DATA_ID_MRU_INDEX = "MRU:Index";

const int32 MAX_MRU_LIST_ENTRIES = 7;

// distance of the controls from the border.
const float dist = 10.0;		

CMRUSelectFileView::CMRUSelectFileView(BRect frame,
	CPreferences *mru_prefs, const char *name, 
	uint32 resizingMode, uint32 flags) :
	CLocalizedDialogBase(frame, name, resizingMode, flags, NULL, NULL, true)
{
	prefs = mru_prefs;
	
	browsePanel = NULL;
	
	// create controls

	// Intermediate postion for the controls during creation.
	// They are moved to the correct postion afterwards.
	BRect dummyPos(0,0,50,5);

	CMRUList mruList;
	LoadMRUList(mruList);
	
	const char *initialText = "";
	
	if(mruList.CountItems() > 0) {
		// use first item in MRU list as initial text for text control
		initialText = mruList.ItemAt(0)->String();
	} else {
		// disable OK button
		EnableOkButton(false);
	}

	const char *pathControlTitle = B_TRANSLATE("Command:");

	pathControl = new BTextControl(dummyPos, "PathCtrl", pathControlTitle, initialText, 
										NULL, B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
											
	arrowButton = new CArrowButton(dummyPos, "ArrowButton", ARROW_RIGHT, 
											new BMessage(MSG_SHOW_MRU_LIST), 
											B_FOLLOW_TOP | B_FOLLOW_RIGHT);

	// calculate position
	
	const float minArrowHeight	= 20.0;
	float arrowHeight, arrowWidth, dummy;
	float windowW = Bounds().Width();
	
	// the arrow has the same height as the text control
	pathControl->GetPreferredSize(&dummy, &arrowHeight);	

	arrowWidth = arrowHeight = MAX(arrowHeight, minArrowHeight);

	BRect textRect(dist, dist, windowW - 2*dist - arrowWidth, dist + arrowHeight);
	BRect arrowButtonRect(windowW-dist-arrowWidth, dist, windowW-dist, dist+arrowHeight);

	// I can't use ResizeTo here, because of some stupid bug in the BeOS GUI.
	// The TextControl consists of 2 controls. If I resize the TextControl
	// the embedded EditControl isn't resized.
	delete pathControl;
	pathControl = new BTextControl(textRect, "PathCtrl", pathControlTitle, initialText, 
										NULL, B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
	pathControl->SetModificationMessage(new BMessage(MSG_PATH_CHANGED));

	// calculate the divider size from the current font.
	float dividerWidth = be_plain_font->StringWidth(pathControlTitle) + dist;
	pathControl->SetDivider(dividerWidth);

	arrowButton->MoveTo(arrowButtonRect.LeftTop());
	arrowButton->ResizeTo(arrowButtonRect.Width(), arrowButtonRect.Height());
	
	AddChild(pathControl);
	AddChild(arrowButton);
	
	SetHelpID("new_team.html");
}

CMRUSelectFileView::~CMRUSelectFileView()
{
	delete prefs;
	delete browsePanel;
}

void CMRUSelectFileView::AttachedToWindow()
{
	CLocalizedDialogBase::AttachedToWindow();
	
	arrowButton->SetTarget(this);
	pathControl->SetTarget(this);
}

bool CMRUSelectFileView::Ok()
{
	if(!CCommandLineParser::LaunchTeam(pathControl->Text()))
		return false;

	// Okay. The path seems to be valid. Add it to MRU list.
	
	CMRUList mruList;
	
	LoadMRUList(mruList);
	AddToMRUList(mruList, pathControl->Text());
	SaveMRUList(mruList);
	
	return true;
}

void CMRUSelectFileView::Cancel()
{
	// nothing to do....
}

void CMRUSelectFileView::LoadMRUList(CMRUList &list)
{
	char mruEntryName[B_ATTR_NAME_LENGTH];
	char mruEntry[4096];
	
	int i=1;
	
	while(true) {
		sprintf(mruEntryName, "%s%d", MRUPrefBaseName(), i);
	
		if(!prefs->Read(mruEntryName, mruEntry, sizeof(mruEntry))) 
			break;
			
		list.AddItem(new BString(mruEntry));
		
		i++;
	}
}

void CMRUSelectFileView::SaveMRUList(const CMRUList &list)
{
	char mruEntryName[B_ATTR_NAME_LENGTH];
	
	for(int i=0 ; i<list.CountItems() ; i++) {
		sprintf(mruEntryName, "%s%d", MRUPrefBaseName(), i+1);
	
		MY_ASSERT(prefs->Write(mruEntryName, *list.ItemAt(i)));
	}
}

void CMRUSelectFileView::DisplayMRUList()
{
	CMRUList mruList;
	
	LoadMRUList(mruList);

	BPopUpMenu *menu = new BPopUpMenu("Process", false);
	
	// add "Browse..." entry to menu
	BMenuItem *browseMenuItem = new BMenuItem(
		B_TRANSLATE("Browse..."), 
		new BMessage(MSG_BROWSE));
		
	browseMenuItem->SetTarget(this);
	
	menu->AddItem(browseMenuItem);

	if(mruList.CountItems() > 0) {
		menu->AddSeparatorItem();
		
		for(int i=0 ; i<mruList.CountItems() ; i++) {
			BString *mruEntry = mruList.ItemAt(i);
		
			CStringList parsedCmdLine;
		
			CCommandLineParser::SplitCommandLine(mruEntry->String(), parsedCmdLine);

			MY_ASSERT(parsedCmdLine.CountItems() >= 1);
		
			BString *appFile = parsedCmdLine.ItemAt(0);

			BMessage *msg = new BMessage(MSG_MRU_ENTRY_SEL);
			
			// the message contains the path of the MRU entry and it's
			// index in the list
			msg->AddInt32(MESSAGE_DATA_ID_MRU_INDEX, i);
			msg->AddString(MESSAGE_DATA_ID_MRU_PATH, mruList.ItemAt(i)->String());

			BString menuString;
			
			menuString.Append(BPath(appFile->String()).Leaf());
			
			for(int i=1 ; i<parsedCmdLine.CountItems() ; i++) {
				menuString.Append(" ");
				menuString.Append(parsedCmdLine.ItemAt(i)->String());
			}

			BEntry entry;
			BMenuItem *menuItem;

			if(CCommandLineParser::GetFullAppPath(appFile->String(), NULL, &entry)) {
				menuItem = new CFileGlyphMenuItem(menuString.String(), &entry, false, msg);
			} else {
				menuItem = new BMenuItem(menuString.String(), msg);
			}
	
			menu->AddItem(menuItem);
		}
	}

	// Add recently used applications to menu
	BMessage recentApps;
	entry_ref recentAppRef;

	int item=0;		// Number of recent apps added to the menu.
	
	be_roster->GetRecentApps(&recentApps, MAX_MRU_LIST_ENTRIES);
	
	for(int i=0 ; recentApps.FindRef("refs", i, &recentAppRef) == B_OK ; i++) {
		BEntry recentAppEntry(&recentAppRef);
		BPath recentAppPath(&recentAppEntry);

		bool addEntry = true;

		for(int j=0 ; j<mruList.CountItems() ; j++) {
			BString *mruEntry = mruList.ItemAt(j);
			
			if((*mruEntry) == recentAppPath.Path()) {
				// Entry alreay part of the list
				addEntry = false;
				break;
			}
		}

		if(addEntry) {	
			if(item++ == 0)
				menu->AddSeparatorItem();
	
			BMessage *msg = new BMessage(MSG_MRU_ENTRY_SEL);
	
			// -1 means: entry currently not part of the list.
			msg->AddInt32(MESSAGE_DATA_ID_MRU_INDEX, -1);
			msg->AddString(MESSAGE_DATA_ID_MRU_PATH, recentAppPath.Path());
			
			BBitmap *icon = new BBitmap(BRect(0,0,15,15), B_RGBA32);
				
			// load icon for file
			if(BNodeInfo::GetTrackerIcon(&recentAppRef, icon, B_MINI_ICON) != B_OK) {
				delete icon;
				icon = NULL;
			}
	
			const char *filename = recentAppPath.Leaf();
	
			CGlyphMenuItem *menuItem = new CGlyphMenuItem(filename, icon, false, msg);
				
			menu->AddItem(menuItem);
		}
	}

	menu->SetTargetForItems(this);

	BRect clickToOpen = arrowButton->Bounds();
	arrowButton->ConvertToScreen(&clickToOpen);
	
	BPoint openPoint = arrowButton->ConvertToScreen(arrowButton->Bounds().RightTop()); 

	// set a smaller font for the menu items
	menu->SetFont(be_plain_font);
	// open popup menu (synchonous)
	menu->Go(openPoint, true, true, clickToOpen, false);

	delete menu;
}

void CMRUSelectFileView::AddToMRUList(CMRUList &mruList, const char *path)
{
	bool alreadyInList=false;
	
	for(int i=0 ; i<mruList.CountItems() ; i++) {
		if(strcmp(mruList.ItemAt(i)->String(), path) == 0) {
			// entry is already in MRU list
			// copy to head of list.
			BString *obj = mruList.RemoveItem(i);
			
			mruList.AddItem(obj, 0);
			
			alreadyInList=true;
			
			break;
		}
	}
	
	if(!alreadyInList) {
		// add new path at head of list.
		mruList.AddItem(new BString(path), 0);
				
		if(mruList.CountItems() > MAX_MRU_LIST_ENTRIES) {
			// too many entries. Delete last entry.
			BString *obj = mruList.RemoveItem(mruList.CountItems()-1);
			delete obj;
		}
	}
}

void CMRUSelectFileView::MRUEntrySelected(const entry_ref &file)
{
	BEntry	fileEntry(&file);
	BPath	filePath;
	
	fileEntry.GetPath(&filePath);
	
	MY_ASSERT(filePath.InitCheck() == B_OK);
	
	BString escapedFilePath = CCommandLineParser::Escape(filePath.Path());
	
	MRUEntrySelected(-1, escapedFilePath.String());
}

void CMRUSelectFileView::MRUEntrySelected(int32 index, const char *path)
{
	pathControl->SetText(path);
	
	CMRUList mruList;
	
	LoadMRUList(mruList);

	if(index < 0) {
		AddToMRUList(mruList, path);
	} else {
		// the position in the list is known. Move entry to head of list.
		BString *obj = mruList.RemoveItem(index);
		mruList.AddItem(obj, 0);
	}
	
	SaveMRUList(mruList);
}

void CMRUSelectFileView::BrowseForFile()
{
	// Creates a new file panel, if it isn't already created.
	// Otherwise it simply shows stored panel.
	
	if(browsePanel == NULL) {
		// don't create this objects on heap. 
		// BFilePanel creates a copy.
		BMessenger msgnr(this);
		BMessage msg(MSG_BROWSE_FINISHED);
	
		entry_ref *p_start_dir=NULL, start_dir;
		
		// Use parent of selected MRU entry as start dir
		BPath startPath, ctrlPath;

		// only use the first component of the command line
		ctrlPath = CCommandLineParser::AppPathFromCmdLine(pathControl->Text());
		
		if(ctrlPath.InitCheck() == B_OK) {
			ctrlPath.GetParent(&startPath);
		
			if(get_ref_for_path(startPath.Path(), &start_dir) == B_OK)
				p_start_dir = &start_dir;
		}
		
		browsePanel = new BFilePanel(
							B_OPEN_PANEL, 
							&msgnr,
							p_start_dir,	// start directory
							B_FILE_NODE,
							false,			// no multiselect
							&msg,
							NULL,			// no ref filter
							true,			// modal
							true);			// hide when done
	}

	browsePanel->Show();
}

void CMRUSelectFileView::GetPreferredSize(float *width, float *height)
{
	CLocalizedDialogBase::GetPreferredSize(width, height);

	if(height) {
		*height += 2*dist + pathControl->Bounds().Height();
	}	
}

void CMRUSelectFileView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_SHOW_MRU_LIST:
			{
				// Arrow button was pressed.
				DisplayMRUList();
			}
			break;
		case MSG_PATH_CHANGED:
			{
				// User entered something in the "Path" control.
				// Update state of OK button.
			
				BTextControl *textControl;
				
				if(message->FindPointer("source", (void **)&textControl) == B_OK) {
					// if the text control is empty the button is disabled.
					EnableOkButton(textControl->Text()[0] != '\0');
				}
			}
			break;
		case MSG_MRU_ENTRY_SEL:
			{
				// User selected an entry in the MRU list.
			
				int32 index = message->FindInt32(MESSAGE_DATA_ID_MRU_INDEX);	
				const char *path = message->FindString(MESSAGE_DATA_ID_MRU_PATH);
			
				MRUEntrySelected(index, path);
			}
			break;
		case MSG_BROWSE:
			{
				// User selected the "Browse..." entry in the MRU list.
			
				BrowseForFile();
			}
			break;
		case MSG_BROWSE_FINISHED:			
			{
				// User selected a file in the BFilePanel.
			
				entry_ref fileRef;

				message->FindRef("refs", &fileRef);
			
				MRUEntrySelected(fileRef);
			}
			break;
		case B_SIMPLE_DATA:
			{
				// User dropped a file from the tracker into this view.
				
				entry_ref fileRef;
				
				if(message->FindRef("refs", &fileRef) == B_OK) {
					MRUEntrySelected(fileRef);
				}
			}
			break;
		default:
			CLocalizedDialogBase::MessageReceived(message);
	}
}
