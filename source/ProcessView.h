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

#ifndef PROCESS_VIEW_H
#define PROCESS_VIEW_H

// ====== Includes ======

#include "Tab.h"

// ====== Types ======

enum {
	COLUMN_NUM_ICON,
	COLUMN_NUM_NAME,
	COLUMN_NUM_TEAM_ID,
	COLUMN_NUM_THREAD_COUNT,
	COLUMN_NUM_AREA_COUNT,
	COLUMN_NUM_IMAGE_COUNT,
	COLUMN_NUM_CPU_USAGE,
	COLUMN_NUM_MEM_USAGE,
	COLUMN_NUM_MEM_USAGE_ABS,
	COLUMN_NUM_DIRECTORY,
};

struct column_info {
	const char	*key;				// Localization key of the title
	int32		 index;				// Index of column
	bool		 defaultVisible;	// Is this column visible on default?
	float		 defaultWidth;		// Default width of column, if not defined in settings
	float		 minWidth;			// Minimum width of column
};

// This array contains information of the string columns of the team view.
// The icon column is NOT included.
extern column_info team_view_colomn_info[];

// ====== Scripting Properties ======

// CProcessView
extern const char * const PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS;		// bool
extern const char * const PROCESS_VIEW_PROP_TEAM_MODEL;				// BMessenger

// ====== Message IDs ======

const int32 MSG_TEAM_ACTION    				= 'mTAC';
const int32 MSG_ASYNC_TEAM_ACTION_FAILED	= 'mTAF';
const int32 MSG_TEAM_SELECTED				= 'mTSE';
const int32 MSG_SELECT_AND_KILL_TEAM		= 'mSAK';
const int32 MSG_ALERT_CLOSED				= 'mALT';
const int32 MSG_SELECT_TEAM					= 'mSLT';

// ====== Message Fields ======

// MSG_ALERT_CLOSED and MSG_TEAM_ACTION
extern const char * const MESSAGE_DATA_ID_ACTION;					// int32
extern const char * const MESSAGE_DATA_ID_TEAM_ID;					// int32
extern const char * const MESSAGE_DATA_ID_PRIORITY;					// int32 (must be between 0 and 120)

// MSG_ASYNC_TEAM_ACTION_FAILED
extern const char * const MESSAGE_DATA_ID_ERROR_CODE;				// int32 (status_t)

// ====== Class Defs =======

class CProcessItem;
class CColumnListViewEx;
class CTeamModel;
class CTeamModelEntry;
class CTeamModelListener;

class CProcessView : public CTabNotifcationView
{
	public:
	CProcessView(BRect rect);

	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();
	virtual void MessageReceived(BMessage *message);
	virtual void Pulse();
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
				BMessage *specifier, int32 what, const char *property);
	virtual void Select(BView *owner);
	virtual void Deselect();

	protected:
	CProcessItem *FindItem(team_id teamId);

	void AddTeam(CTeamModelEntry *entry, bool sort);
	void RemoveTeam(CTeamModelEntry *entry);

	void KillTeamWithWarning(team_id id);
	void ActivateTeam(team_id id);
	void QuitTeam(team_id id);
	void KillTeam(team_id id);

	status_t SetHideSystemTeams(bool newValue);

	void TeamSelected(int32 selIndex);
	void UpdateListView();
	
	int32 ShowWarning(const char *text, const char *button1, 
				const char *prefName, bool asynchron=false, BMessage *msg=NULL);

	// If set to false the sorting is disabled until a MSG_MENU_ENDED 
	// message is received.
	bool				sortItems;		
	
	bool				hideSystemTeams;
	bigtime_t			lastUpdateTime;
	CColumnListViewEx  *listView;
	BButton			   *killButton, *selectTeamButton;
	CTeamModel		   *teamModel;
	CTeamModelListener *teamModelListener;
	
	friend CTeamModelListener;
};

#endif // PROCESS_VIEW_H