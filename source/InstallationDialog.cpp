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
#include "help.h"
#include "version.h"
#include "MenuField.h"
#include "FileGlyphMenuItem.h"
#include "MainWindow.h"
#include "TaskManager.h"
#include "TaskManagerPrefs.h"
#include "InstallationDialog.h"

// ==== CInstallationDialog ====

// protected constructor
CInstallationDialog::CInstallationDialog() :
	CSingletonWindow(
		BRect(0,0,50,50), 
		"Install " APP_NAME " " TASKMANAGER_VERSION_STRING, 
		B_TITLED_WINDOW, 
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, 
		B_CURRENT_WORKSPACE)
{
	BView *view = new CInstallationDialogView(Bounds());
	
	float width, height;
	
	view->GetPreferredSize(&width, &height);
	view->ResizeToPreferred();
	
	BRect screenRect = BScreen(this).Frame();
	
	// center window
	float wx = (screenRect.Width()-width)/2;
	float wy = (screenRect.Height()-height)/2;

	MoveTo(wx, wy);
	ResizeTo(width, height);

	AddChild(view);
	Show();
}

CInstallationDialog::~CInstallationDialog()
{
	RemoveFromList(ClassName());
}

CInstallationDialog *CInstallationDialog::CreateInstance()
{
	// Initialize to quiet compiler.
	CInstallationDialog *window = NULL;

	return CreateSingleton(window, "CInstallationDialog");
}

bool CInstallationDialog::QuitRequested()
{
	if(be_app->CountWindows() <= 1) {
		// exit application, if this is the last window
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
	
	return true;
}

// ==== CInstallationDialogView ====

const float CInstallationDialogView::dist = 10.0;

struct check_box_info
{
	const char * const label;
	bool checked;
};

const check_box_info checkBoxInfo[] = 
{
	{ "Add Link to Be Menu", true	},
	{ "Add Link to Desktop", false	},
	{ "View Deskbar Replicant on Startup", true	},	
};

CInstallationDialogView::CInstallationDialogView(BRect frame) :
	CDialogBase(frame, "InstallationDialogView")
{
	BRect dummyFrame(0, 0, 200, 5);
	
	generalOptionsBox  = new BBox(dummyFrame, "GeneralOptionsBox");
	languageOptionsBox = new BBox(dummyFrame, "LanguageOptionsBox");
	
	AddChild(generalOptionsBox);
	AddChild(languageOptionsBox);
	
	// Initialize check boxes
	
	for(int i=0 ; i<NUM_CHECK_BOXES ; i++) {
		BString checkBoxName = "CheckBox";
		checkBoxName << i;
		
		checkBoxes[i] = new BCheckBox(dummyFrame, checkBoxName.String(), checkBoxInfo[i].label, NULL);

		checkBoxes[i]->SetValue(checkBoxInfo[i].checked ? B_CONTROL_ON : B_CONTROL_OFF);
		checkBoxes[i]->ResizeToPreferred();
					
		generalOptionsBox->AddChild(checkBoxes[i]);
	}

	// Initialize group menu field

	BMenu *groupsMenu = new BPopUpMenu("<Select>");
	
	BPath beMenuPath;
		
	if(find_directory(B_USER_DESKBAR_DIRECTORY, &beMenuPath) == B_OK) {
		BDirectory beMenuDir(beMenuPath.Path());

		BEntry entry;

		while(beMenuDir.GetNextEntry(&entry, false) == B_OK) {
			if(entry.IsDirectory()) {		
				BPath entryPath;
			
				entry.GetPath(&entryPath);
				
				CFileGlyphMenuItem *item = 
					new CFileGlyphMenuItem(NULL, &entryPath, false);
				
				if(strcmp(entryPath.Leaf(), "Applications") == 0) {
					item->SetMarked(true);
				}
				
				groupsMenu->AddItem(item);
			}
		}
	}
	
	groupMenuField = create_menu_field(B_ORIGIN, "GroupsMenuField", "Group:", groupsMenu);

	generalOptionsBox->AddChild(groupMenuField);
	
	// Initialize language selection box

	BPath languageDirPath = get_app_dir();
	languageDirPath.Append("language");

	BDirectory languageDir(languageDirPath.Path());
	
	BMenu *languagesMenu = new BPopUpMenu("<Select>");
	BEntry entry;
	
	BString selectedLanguage = CTaskManagerPrefs().Language();
	
	while(languageDir.GetNextEntry(&entry, false) == B_OK) {
		if(entry.IsFile()) {
			BPath entryPath;
			
			entry.GetPath(&entryPath);
			
			const char *language = entryPath.Leaf();
		
			BMenuItem *item = new BMenuItem(language, NULL);
			
			if(strcmp(language, selectedLanguage.String()) == 0) {
				item->SetMarked(true);
			}

			languagesMenu->AddItem(item);
		}
	}
	
	languageMenuField = create_menu_field(B_ORIGIN, "LanguageMenuField", "Language:", languagesMenu);
	
	languageOptionsBox->AddChild(languageMenuField);
}

void CInstallationDialogView::AttachedToWindow()
{
	CDialogBase::AttachedToWindow();
	
	for(int i=0 ; i<NUM_CHECK_BOXES ; i++) {
		checkBoxes[i]->StartWatching(this, B_CONTROL_INVOKED);
	}
}

void CInstallationDialogView::FrameResized(float width, float height)
{
	CDialogBase::FrameResized(width, height);

	float checkBoxHeight = checkBoxes[0]->Bounds().Height();
	
	float pos_y = dist;
	
	for(int i=0 ; i<NUM_CHECK_BOXES ; i++) {
		checkBoxes[i]->MoveTo(dist, pos_y);
		checkBoxes[i]->ResizeTo(width-4*dist, checkBoxHeight);

		pos_y += dist+checkBoxHeight;
		
		if(i == 0) {
			// additional space for the "Groups" menu field.
			groupMenuField->MoveTo(3*dist, pos_y-dist/2);
			
			pos_y += dist+checkBoxHeight;
		}
	}
	
	generalOptionsBox->MoveTo(dist, dist);
	generalOptionsBox->ResizeTo(width-2*dist, pos_y);
	
	languageMenuField->MoveTo(dist, dist);
	
	languageOptionsBox->MoveTo(dist, pos_y+2*dist);
	languageOptionsBox->ResizeTo(width-2*dist, 2*dist+checkBoxHeight);
}

void CInstallationDialogView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case B_OBSERVER_NOTICE_CHANGE:
			{
				int32 notifyWhat = msg->FindInt32("be:observe_change_what");
				
				if(notifyWhat == B_CONTROL_INVOKED) {
					// Some contol in the dialog was invoked.
					BControl *control;
					
					if(msg->FindPointer("source", (void **)&control) == B_OK) {
						if(control == checkBoxes[0]) {
							// checkbox 0 ("Add Link to Be Menu")
							groupMenuField->SetEnabled(
								checkBoxes[0]->Value() == B_CONTROL_ON);
						}
					}
				}
			}
			break;
		default:
			CDialogBase::MessageReceived(msg);
	}
}

void CInstallationDialogView::GetPreferredSize(float *width, float *height)
{
	float checkBoxHeight = checkBoxes[0]->Bounds().Height();
	float menuFieldHeight = checkBoxHeight;
	float elementWidth=0.0;

	for(int i=0 ; i<NUM_CHECK_BOXES ; i++) {
		elementWidth = MAX(elementWidth, checkBoxes[i]->Bounds().Width());
	}
	
	elementWidth = MAX(elementWidth, groupMenuField->Bounds().Width()+1*dist); 
	elementWidth = MAX(elementWidth, languageMenuField->Bounds().Width());
	
	float baseWidth, baseHeight;
	
	CDialogBase::GetPreferredSize(&baseWidth, &baseHeight);
	
	if(width) {
		*width = MAX(baseWidth, elementWidth+4*dist);
	}
	
	if(height) {
		*height = NUM_CHECK_BOXES * (checkBoxHeight+dist) + 7*dist + baseHeight + 2*menuFieldHeight;
	}
}

bool CInstallationDialogView::Ok()
{
	// Get path of this application.
	app_info appInfo;
	
	be_app->GetAppInfo(&appInfo);
	
	BEntry entry(&appInfo.ref);
	BPath exePath;
	
	entry.GetPath(&exePath);
	
	status_t status;

	if(checkBoxes[0]->Value() == B_CONTROL_ON) {
		// "Add Link to Be Menu" selected
		BPath beMenuPath, applicationsPath;
		
		if((status = find_directory(B_USER_DESKBAR_DIRECTORY, &beMenuPath)) == B_OK) {
			// get selected group
			BMenuItem *selItem = groupMenuField->Menu()->FindMarked();
			const char *group;
			
			if(selItem == NULL) {
				// default group is 'Applications'
				group = "Applications";
			} else {
				group = selItem->Label();
			}
		
			applicationsPath = beMenuPath;
			applicationsPath.Append(group);
		
			BEntry applicationsEntry(applicationsPath.Path());

			BDirectory dir;
			
			if(!applicationsEntry.Exists()) {
				// Generate "Applications" sub-folder in Be Menu.
				BDirectory beMenuDir(beMenuPath.Path());
				beMenuDir.CreateDirectory(applicationsPath.Path(), &dir);
			} else {
				// Use existing directory.
				dir.SetTo(applicationsPath.Path());
			}
		
			BEntry symLinkEntry(&dir, APP_NAME, false);
			BSymLink symLink;
			
			if(symLinkEntry.Exists()) {
				// The SymLink already exists.
				// Delete it and then re-create it with the new position.
				symLinkEntry.Remove();
			}
			
			if((status = dir.CreateSymLink(APP_NAME, exePath.Path(), &symLink)) != B_OK) {
				BString message;
				
				message << "Can't create link in '"
						<< applicationsPath.Path()
						<< "'\n Reason: "
						<< strerror(status);
				
				show_alert(message);
			}
		} else {
			BString message;
			
			message << "Can't locate Be Menu directory\n Reason: "
					<< strerror(status);
			
			show_alert(message);
		}
	}

	if(checkBoxes[1]->Value() == B_CONTROL_ON) {
		// "Add Link to Desktop" selected
		BPath desktopPath;
		
		if((status = find_directory(B_DESKTOP_DIRECTORY, &desktopPath)) == B_OK) {
			BDirectory desktopDir(desktopPath.Path());

			BEntry symLinkEntry(&desktopDir, APP_NAME, false);
			BSymLink symLink;
			
			if(symLinkEntry.Exists()) {
				// The SymLink already exists.
				// Delete it and then re-create it with the new position.
				symLinkEntry.Remove();
			}
			
			if((status = desktopDir.CreateSymLink(APP_NAME, exePath.Path(), &symLink)) != B_OK) {
				BString message;
				
				message << "Can't create link in '"
						<< desktopPath.Path()
						<< "'\n Reason: "
						<< strerror(status);
				
				show_alert(message);
			}
		} else {
			BString message;
			
			message << "Can't locate Desktop directory\n Reason: "
					<< strerror(status);
			
			show_alert(message);
		}
	}

	if(checkBoxes[2]->Value() == B_CONTROL_ON) {
		// "Show Deskbar Replicant on Startup" selected
		CTaskManagerPrefs().Write(PREF_HIDE_DESKBAR_REPLICANT_ON_CLOSE, false);
		CMainWindow::ShowDeskbarReplicant();
	}

	BMenuItem *languageSelItem = languageMenuField->Menu()->FindMarked();
	
	if(languageSelItem != NULL) {
		CTaskManagerPrefs().SetLanguage(languageSelItem->Label());
	}
	
	return true;
}
