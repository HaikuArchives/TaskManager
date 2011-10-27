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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// ====== Message IDs ======

const int32 MSG_SHOW_SETTINGS_WINDOW		= 'mSEW';
const int32 MSG_SHOW_IN_ALL_WORKSPACES		= 'mSAW';
const int32 MSG_CREATE_TEAM					= 'mCRT';

// ====== Deskbar Replicant ID ======

extern const char * const DESKBAR_REPLICANT_ID;

// ====== Tab ID Enumeration ======

enum enumTabID {
	TAB_ID_USAGE = 0,
	TAB_ID_TEAMS,
	TAB_ID_PERFORMANCE,
};

// ====== Class Defs ======

class CMainWindowView : public BView
{
	public:
	CMainWindowView(BRect rect);
	
	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();
	
	protected:
	BTabView *tabView;
};

class CMainWindow : public BWindow
{
	public:
	CMainWindow(BRect rect, bigtime_t updateSpeed, 
		bool showReplicant, bool showOnAllWorkspaces);
		
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage *message);
	virtual void MenusBeginning();
	
	static bool ReplicantVisible();
	static status_t ShowDeskbarReplicant(bool showAlert=true);
	static status_t HideDeskbarReplicant(bool showAlert=true);
	
	void CreateTeam();
	void ShowSettingsWindow();
	
	protected:
	void MarkSource(BMessage *message, bool mark);	
};

#endif // MAIN_WINDOW_H