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
#include "version.h"
#include "my_assert.h"
#include "CounterNamespaceImpl.h"
#include "InstallationDialog.h"
#include "AboutWindow.h"
#include "MainWindow.h"
#include "ProcessView.h"
#include "TaskManagerPrefs.h"
#include "TaskManager.h"

#include "TeamModel.h"

#include <Catalog.h>
#include <Locale.h>
#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "TaskManager"

// ====== globals ======

// Properties
const char * const TASKMANAGER_PROPERTY_TEAMLIST	= "Teamlist";

// Message field for MSG_SHOW_REPLICANT
const char * const MESSAGE_DATA_ID_SHOW				= "SHOWREP:Show";

// ====== CTaskManagerApplication ======

CTaskManagerApplication::CTaskManagerApplication() : 
	BApplication(APP_SIGNATURE)
{
	mainWindow = NULL;
	
	// Init my only global object
	InitGlobalNamespace();
	
	showMainWindow = true;
}

CTaskManagerApplication::~CTaskManagerApplication()
{
}

void CTaskManagerApplication::AboutRequested()
{
	CAboutWindow::CreateInstance();	
}

void CTaskManagerApplication::ArgvReceived(int32 argc, char **argv)
{
	BApplication::ArgvReceived(argc, argv);

	if(IncludesOption(argc, argv, "help")) {
		printf(FULL_APP_NAME " V" TASKMANAGER_VERSION_STRING);
		printf(" (c) 1999-2002 by Thomas Krammer\n\n");
		printf("  --help                  Display this help text\n");
		printf("  --install               Show installation dialog\n");
		printf("  --show_deskbar_rep      Add replicant to deskbar and then quit\n");
		printf("  --hide_deskbar_rep      Remove replicant from deskbar and then quit\n");
		printf("  --tweak_deskbar         Make the deskbar topmost and then quit\n");
		
		if(IsLaunching()) {
			// Don't close application, if I receive this message when the
			// application is already running. This can happen, if "single launch"
			// is set and the application is restarted from the shell.
			PostMessage(B_QUIT_REQUESTED);
		}
	} else {
		// '--help' overwrites all other options.

		bool quit = false;
	
		if(IncludesOption(argc, argv, "show_deskbar_rep")) {
			// I can't call CMainWindow::ShowDeskbarReplicant() directly, because
			// that function needs a fully construted and running BApplication
	
			BMessage *showRep = new BMessage(MSG_SHOW_REPLICANT);
			
			showRep->AddBool(MESSAGE_DATA_ID_SHOW, true);
		
			PostMessage(showRep);

			quit = true;		
		}
		
		if(IncludesOption(argc, argv, "hide_deskbar_rep")) {
			// I can't call CMainWindow::HideDeskbarReplicant() directly, because
			// that function needs a fully construted and running BApplication
		
			BMessage *hideRep = new BMessage(MSG_SHOW_REPLICANT);
			
			hideRep->AddBool(MESSAGE_DATA_ID_SHOW, false);
		
			PostMessage(hideRep);
		
			quit = true;		
		}

		if(IncludesOption(argc, argv, "tweak_deskbar")) {
			PostMessage(new BMessage(MSG_TWEAK_DESKBAR));
		
			quit = true;		
		}

		if(IncludesOption(argc, argv, "install")) {
			CInstallationDialog::CreateInstance();
			
			showMainWindow = false;
		}

		
		// Post B_QUIT_REQUESTED after last message, if 'quit' is set.
		if(quit) {
			// Only tell application to quit, if B_ARGS_RECEIVED is received
			// during startup. 
		
			if(IsLaunching())
				PostMessage(B_QUIT_REQUESTED);
		}
	}
}

void CTaskManagerApplication::ReadyToRun()
{
	BApplication::ReadyToRun();
	
	// --- Update MIME Database
	
	// Update MIME database to represent the current application
	// image path.

	app_info appInfo;
	
	GetAppInfo(&appInfo);
	
	BMimeType appSignature(APP_SIGNATURE);
	
	appSignature.SetAppHint(&appInfo.ref);

	// --- Create and display main window
	
	if(showMainWindow) {
		CTaskManagerPrefs prefs;
	
		// If the screen size changed the window maybe invisible.
		// Move the window to visible area.
		BRect mainWindowRect = prefs.MainWindowRect();
		BRect screenRect = BScreen().Frame();
	
		if(mainWindowRect.left < 0)
			mainWindowRect.OffsetTo(0, mainWindowRect.top);
	
		if(mainWindowRect.top < 0)
			mainWindowRect.OffsetTo(mainWindowRect.left, 0);
		
		if(mainWindowRect.right > screenRect.right) {
			float width = mainWindowRect.Width();
			mainWindowRect.OffsetTo(screenRect.right - width, mainWindowRect.top);
		}
	
		if(mainWindowRect.bottom >= screenRect.bottom) {
			float height = mainWindowRect.Height();
			mainWindowRect.OffsetTo(mainWindowRect.left, screenRect.bottom - height);
		}
		
		mainWindow = new CMainWindow(mainWindowRect, 
									 prefs.PulseRate(), 
									 prefs.ShowDeskbarReplicant(),
									 prefs.ShowInAllWorkspaces());
	
		// I don't call mainWindow->ShowWindow() here, because if any option in "ArgvReceived"
		// causes the application to quit, the window would still be shown for a second.
		// Because this message is added AFTER the B_QUIT_REQUESTED message into the 
		// message queue, the window doesn't show up in that case.
		PostMessage(MSG_SHOW_MAIN_WINDOW);
	}
}

BHandler *CTaskManagerApplication::ResolveSpecifier(BMessage *message, int32 index, BMessage *specifier, int32 what, const char *property)
{
	if(strcmp(property, TASKMANAGER_PROPERTY_TEAMLIST) == 0) {
		if(mainWindow != NULL) {
			// Forward message to TeamModel of the ProcessView (main view of the
			// teams tab).
		
			BMessage teamListSpec(B_GET_PROPERTY);

			// Get a messenger targeting the TeamModel.
			teamListSpec.AddSpecifier(PROCESS_VIEW_PROP_TEAM_MODEL);
			teamListSpec.AddSpecifier("View",   (int32)1);		// "Teams" tab
			teamListSpec.AddSpecifier("View",   (int32)0);		// container for tabbed views
			teamListSpec.AddSpecifier("View",   "MainViewTab");
			teamListSpec.AddSpecifier("View",   "MainWindowView");
			
			BMessenger windowMessenger(mainWindow);
			BMessage reply;
			
			if(windowMessenger.SendMessage(&teamListSpec, &reply) != B_OK) {
				return NULL;
			}
				
			BMessenger teamModelMessenger;
				
			if(reply.FindMessenger("result", &teamModelMessenger) != B_OK) {
				return NULL;
			}
			
			if(!teamModelMessenger.IsValid()) {
				return NULL;
			}
	
			// Forward message		
			message->PopSpecifier();
			
			teamModelMessenger.SendMessage(message, message->ReturnAddress());
			
			return NULL;
		}
	}
	
	return BApplication::ResolveSpecifier(message, index, specifier, what, property);
}

void CTaskManagerApplication::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_SHOW_MAIN_WINDOW:
			if(CountWindows() > 0) {
				BAutolock windowLock(mainWindow);
			
				if(mainWindow->IsHidden()) {
					mainWindow->Show();
				} else {
					mainWindow->Activate();
				}
			}

			break;
		case MSG_SHOW_REPLICANT:
			{
				bool show;
			
				if(message->FindBool(MESSAGE_DATA_ID_SHOW, &show) == B_OK) {
					if(show)
						CMainWindow::ShowDeskbarReplicant(false);
					else
						CMainWindow::HideDeskbarReplicant(false);
				}
			}
			break;
		case MSG_TWEAK_DESKBAR:
			{
				BString errorString;
				status_t result;
			
				BMessenger deskbarMessenger("application/x-vnd.Be-TSKB");
				
				if(deskbarMessenger.IsValid()) {
					// Deskbar exists.
					
					BMessage msg, reply;

					// Set the property Window["Deskbar"].Feel to B_FLOATING_ALL_WINDOW_FEEL
					// This makes the Deskbar topmost!
					msg.what = B_SET_PROPERTY;
					msg.AddInt32("data", B_FLOATING_ALL_WINDOW_FEEL);					
					msg.AddSpecifier("Feel");				// B_DIRECT_SPECIFIER
					msg.AddSpecifier("Window", "Deskbar");	// B_NAME_SPECIFIER
					
					if( (result = deskbarMessenger.SendMessage(&msg, &reply)) == B_OK) {
						if(reply.FindInt32("error", &result) == B_OK && result != B_OK) {
							const char *errorMessage=NULL;
						
							reply.FindString("message", &errorMessage);
						
							errorString << "SetProperty failed ("
										<< (errorMessage ? errorMessage : strerror(result))
										<< ")";
							
							show_alert(errorString);
						} else {
							// Deskbar should now be TOPMOST!
						}
					} else {
						errorString << "SendMessage failed (" << strerror(result) << ")";

						show_alert(errorString);
					}
				} else {
					show_alert("No team with MIME type \'application/x-vnd.Be-TSKB\' found.\n"
							   "Deskbar not running??");
				}
			}
		case MSG_SELECT_AND_KILL_TEAM:
			// Forward message to Teams view (CProcessView).
			// This view responds to this message by opening the "Select Team" window
			// (CSelectTeamWindow).
			{
				BMessage teamListSpec(B_GET_PROPERTY);
	
				// Get a messenger targeting the Teams view (CProcessView).
				teamListSpec.AddSpecifier("Messenger");
				teamListSpec.AddSpecifier("View",   (int32)TAB_ID_TEAMS);// "Teams" tab
				teamListSpec.AddSpecifier("View",   (int32)0);		// container for tabbed views
				teamListSpec.AddSpecifier("View",   "MainViewTab");
				teamListSpec.AddSpecifier("View",   "MainWindowView");
				
				BMessenger windowMessenger(mainWindow);
				BMessage reply;
				status_t status;

				MY_ASSERT(mainWindow != NULL);
				
				if((status = windowMessenger.SendMessage(&teamListSpec, &reply)) != B_OK) {
					show_alert(strerror(status));
					return;
				}
					
				BMessenger teamViewMessenger;
					
				if((status = reply.FindMessenger("result", &teamViewMessenger)) != B_OK) {
					show_alert(strerror(status));
					return;
				}
				
				if(!teamViewMessenger.IsValid()) {
					show_alert("Messenger invalid");
					return;
				}
				
				// Forward the message.
				if((status = teamViewMessenger.SendMessage(message)) != B_OK) {
					show_alert(strerror(status));
				}
			}
		default:
			BApplication::MessageReceived(message);
	}
}

bool CTaskManagerApplication::IncludesOption(int32 argc, char **argv, const char *option)
{
	for(int i=1 ; i<argc ; i++) {
		if(strncmp(argv[i], "--", 2) == 0) {
			// is option
			
			if(strcmp(&argv[i][2], option) == 0) {
				return true;
			}
		}
	}
	
	return false;
}

// ====== main ======

int main(int argc, char **argv)
{
	#ifdef MEM_DEBUG
	SetNewLeakChecking(true);
	SetMallocLeakChecking(true);
	#endif // MEM_DEBUG

	new CTaskManagerApplication(); 

	be_app->Run();

	delete be_app;

	return 0;
}