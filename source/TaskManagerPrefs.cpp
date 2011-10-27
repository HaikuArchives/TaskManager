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
#include "common.h"
#include "TaskManagerPrefs.h"

// ====== globals ======

const char * const PREF_MAIN_WINDOW_RECT					= "MainWindowRect";
const char * const PREF_PULSE_RATE							= "PulseRate";
const char * const PREF_SHOW_DESKBAR_REPLICANT				= "ShowDeskbarRep";
const char * const PREF_COLUMN_WIDTH_PREFIX					= "ColumnWidth_";
const char * const PREF_COLUMN_DISPLAY_ORDER				= "ColumnDisplayOrder";
const char * const PREF_COLUMN_VISIBLE_PREFIX				= "ColumnVisible_";
const char * const PREF_SORT_KEY_INFO						= "SortKeyInfo";
const char * const PREF_SHOW_KILL_WARNING					= "ShowKillWarning";
const char * const PREF_SHOW_SYSTEM_KILL_WARNING			= "ShowSystemKillWarning";
const char * const PREF_HIDE_DESKBAR_REPLICANT_ON_CLOSE		= "HideDeskbarRepOnClose";
const char * const PREF_HIDE_SYSTEM_TEAMS					= "HideSystemTeams";
const char * const PREF_SHOW_IN_ALL_WORKSPACES				= "ShowInAllWorkspaces";
const char * const PREF_ADD_PERFORMANCE_WINDOW_RECT			= "AddPreformanceWindowRect";
const char * const PREF_PERFORMANCE_LEGEND_BAR_WIDTH		= "PerfLegendBarWidth";
const char * const PREF_LANGUAGE							= "Language";
const char * const PREF_SELECTED_TAB						= "SelectedTab";

// ====== CTaskManagerPrefs ======

CTaskManagerPrefs::CTaskManagerPrefs() : 
	CFilePreferences("TaskMgr_settings")
{
}

BRect CTaskManagerPrefs::MainWindowRect()
{
	BRect rect;

	Read(PREF_MAIN_WINDOW_RECT, rect, BRect(100,100, 400, 400));
	
	return rect;
}

void CTaskManagerPrefs::SetMainWindowRect(BRect rect)
{
	Write(PREF_MAIN_WINDOW_RECT, rect);
}

BRect CTaskManagerPrefs::AddPreformanceWindowRect(BRect defaultRect)
{
	BRect rect;

	Read(PREF_ADD_PERFORMANCE_WINDOW_RECT, rect, defaultRect);
	
	return rect;
}

void CTaskManagerPrefs::SetAddPreformanceWindowRect(BRect rect)
{
	Write(PREF_ADD_PERFORMANCE_WINDOW_RECT, rect);
}

bigtime_t CTaskManagerPrefs::PulseRate()
{
	bigtime_t pulse;
	
	Read(PREF_PULSE_RATE, pulse, NORMAL_PULSE_RATE);
	
	return pulse;
}

void CTaskManagerPrefs::SetPulseRate(bigtime_t pulse)
{
	Write(PREF_PULSE_RATE, pulse);
}

bool CTaskManagerPrefs::ShowDeskbarReplicant()
{
	bool show;
	
	Read(PREF_SHOW_DESKBAR_REPLICANT, show);
	
	return show;
}

void CTaskManagerPrefs::SetShowDeskbarReplicant(bool show)
{
	Write(PREF_SHOW_DESKBAR_REPLICANT, show);
}

float CTaskManagerPrefs::PerformanceLegendBarWidth(float defaultValue)
{
	float value;

	Read(PREF_PERFORMANCE_LEGEND_BAR_WIDTH, value, defaultValue);
	
	return value;
}

void CTaskManagerPrefs::SetPerformanceLegendBarWidth(float width)
{
	Write(PREF_PERFORMANCE_LEGEND_BAR_WIDTH, width);
}

bool CTaskManagerPrefs::ShowInAllWorkspaces()
{
	bool show;
	
	Read(PREF_SHOW_IN_ALL_WORKSPACES, show, false);
	
	return show;
}

void CTaskManagerPrefs::SetShowInAllWorkspaces(bool show)
{
	Write(PREF_SHOW_IN_ALL_WORKSPACES, show);
}

bool CTaskManagerPrefs::HideSystemTeams()
{
	bool hideSystemTeams;
	
	Read(PREF_HIDE_SYSTEM_TEAMS, hideSystemTeams, false);
	
	return hideSystemTeams;
}

bool CTaskManagerPrefs::HideDeskbarReplicantOnClose()
{
	bool hideDeskbarRep;
	
	Read(PREF_HIDE_DESKBAR_REPLICANT_ON_CLOSE, hideDeskbarRep, true);
	
	return hideDeskbarRep;
}

float CTaskManagerPrefs::ColumnWidth(int32 columnNum, float defaultValue)
{
	char name[255];
	
	sprintf(name, "%s%ld", PREF_COLUMN_WIDTH_PREFIX, columnNum);
	
	float width;
	
	Read(name, width, defaultValue);
	
	return width;
}

void CTaskManagerPrefs::SetColumnWidth(int32 columnNum, float width)
{
	char name[255];
	
	sprintf(name, "%s%ld", PREF_COLUMN_WIDTH_PREFIX, columnNum);
	
	Write(name, width);
}

bool CTaskManagerPrefs::ColumnVisible(int32 columnNum, bool defaultValue)
{
	char name[255];
	
	sprintf(name, "%s%ld", PREF_COLUMN_VISIBLE_PREFIX, columnNum);
	
	bool visible;
	
	Read(name, visible, defaultValue);
	
	return visible;
}

void CTaskManagerPrefs::SetColumnVisible(int32 columnNum, bool visible)
{
	char name[255];
	
	sprintf(name, "%s%ld", PREF_COLUMN_VISIBLE_PREFIX, columnNum);
	
	Write(name, visible);
}

sort_key_info *CTaskManagerPrefs::SortKeyInfo()
{
	int32 num = SortKeyInfoCount();

	sort_key_info *sortInfo = new sort_key_info [num];
	
	Read(PREF_SORT_KEY_INFO, (void *)sortInfo, sizeof(sort_key_info), num);
	
	return sortInfo;
}

int32 CTaskManagerPrefs::SortKeyInfoCount()
{
	return (int32)(DataSize(PREF_SORT_KEY_INFO) / sizeof(sort_key_info));
}

void CTaskManagerPrefs::SetSortKeyInfo(sort_key_info *sortKeys, int32 num)
{
	Write(PREF_SORT_KEY_INFO, (void *)sortKeys, sizeof(sort_key_info), num);
}

int32 *CTaskManagerPrefs::ColumnDisplayOrder(int32 numColumns)
{
	int32 *dispOrder = new int32 [numColumns];
	
	// initalize array
	for(int i=0 ; i<numColumns ; i++)
		dispOrder[i] = i;
		
	Read(PREF_COLUMN_DISPLAY_ORDER, (void *)dispOrder, sizeof(int32), numColumns);

	return dispOrder;
}

void CTaskManagerPrefs::SetColumnDisplayOrder(int32 *displayOrder, int32 numColumns)
{
	Write(PREF_COLUMN_DISPLAY_ORDER, (void *)displayOrder, sizeof(int32), numColumns);
}

void CTaskManagerPrefs::SetLanguage(const char *language)
{
	Write(PREF_LANGUAGE, language);
}

BString CTaskManagerPrefs::Language()
{
	BString language;
	
	Read(PREF_LANGUAGE, language, "English");
	
	return language;
}
