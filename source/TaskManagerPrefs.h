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

#ifndef TASKMANAGER_PREFS_H
#define TASKMANAGER_PREFS_H

extern const char * const PREF_MAIN_WINDOW_RECT;
extern const char * const PREF_PULSE_RATE;
extern const char * const PREF_SHOW_DESKBAR_REPLICANT;
extern const char * const PREF_COLUMN_WIDTH_PREFIX;
extern const char * const PREF_COLUMN_DISPLAY_ORDER;
extern const char * const PREF_COLUMN_VISIBLE_PREFIX;
extern const char * const PREF_SORT_KEY_INFO;
extern const char * const PREF_SHOW_KILL_WARNING;
extern const char * const PREF_SHOW_SYSTEM_KILL_WARNING;
extern const char * const PREF_HIDE_DESKBAR_REPLICANT_ON_CLOSE;
extern const char * const PREF_HIDE_SYSTEM_TEAMS;
extern const char * const PREF_SHOW_IN_ALL_WORKSPACES;
extern const char * const PREF_ADD_PERFORMANCE_WINDOW_RECT;
extern const char * const PREF_PERFORMANCE_LEGEND_BAR_WIDTH;
extern const char * const PREF_LANGUAGE;
extern const char * const PREF_SELECTED_TAB;

#include "Preferences.h"

class CPrefPersistent
{
	public:
	virtual void WriteToPrefs(CPreferences *prefs) = 0;
	virtual void ReadFromPrefs(CPreferences *prefs) = 0;
};

class sort_key_info
{
	public:
	sort_key_info() { columnIndex=0; sortMode=2; }

	int32 columnIndex;
	int32 sortMode;		//  (0=Ascending, 1=Desending, 2=NoSort)
};

class CTaskManagerPrefs : public CFilePreferences
{
	public:
	CTaskManagerPrefs();
	
	BRect MainWindowRect();
	void SetMainWindowRect(BRect rect);
	
	BRect AddPreformanceWindowRect(BRect defaultRect);
	void SetAddPreformanceWindowRect(BRect rect);
	
	bigtime_t PulseRate();
	void SetPulseRate(bigtime_t pulse);
	
	bool ShowDeskbarReplicant();
	void SetShowDeskbarReplicant(bool show);
	
	bool ShowInAllWorkspaces();
	void SetShowInAllWorkspaces(bool show);
	
	float ColumnWidth(int32 columnNum, float defaultValue);
	void SetColumnWidth(int32 columnNum, float width);

	bool ColumnVisible(int32 columnNum, bool defaultValue);
	void SetColumnVisible(int32 columnNum, bool visible);

	int32 ColumnSortMode(int32 columnNum, int32 defaultValue);
	void SetColumnSortMode(int32 columnNum, int32 sortMode);

	float PerformanceLegendBarWidth(float defaultValue);
	void SetPerformanceLegendBarWidth(float width);
	
	bool HideSystemTeams();
	bool HideDeskbarReplicantOnClose();
	
	// the array returned by this function belongs to the caller.
	// The caller must delete it.
	int32 *ColumnDisplayOrder(int32 numColumns);
	void SetColumnDisplayOrder(int32 *displayOrder, int32 numColumns);

	void SetLanguage(const char *language);
	BString Language();
	
	// Provides information about a sort keys in the process view.
	// The first key is the primary sort key, the second the secundary
	// sort key and so on. The array returned by this function has
	// 'SortKeyInfoCount()' entries. It belongs to the caller.
	sort_key_info *SortKeyInfo();
	// Returns the number of sort keys
	int32 SortKeyInfoCount();
	// Writes an array of sort key information struct into the prefs
	// file. The first entry in the array is the primary sort key,
	// the second the secundary sort key and so on.
	void SetSortKeyInfo(sort_key_info *sortKeys, int32 num);
};

#endif // TASKMANAGER_PREFS_H