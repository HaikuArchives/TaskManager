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
#include "msg_helper.h"
#include "common.h"
#include "my_assert.h"
#include "AsynchronousPopupMenu.h"
#include "FlickerFreeButton.h"
#include "SelectTeamWindow.h"
#include "MainWindow.h"
#include "TaskManagerPrefs.h"
#include "GlyphMenuItem.h"
#include "TaskManager.h"
#include "TaskManagerPrefs.h"
#include "PerformanceView.h"
#include "PointerList.h"
#include "AlertEx.h"
#include "TeamModel.h"
#include "Process.h"
#include "ColumnListViewEx.h"
#include "ProcessView.h"

#include <Catalog.h>
#include <Locale.h>
#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "ProcessView"

// ====== internal classes =======

class CTeamModelListener : public ITeamModelListener
{
	public:
	CTeamModelListener(CProcessView *_teamView)
	{
		teamView = _teamView;
	}
	
	void ItemAdded(CTeamModelEntry *entry, int32 index)
	{
		teamView->AddTeam(entry, index);
	}
	
	void ItemRemoved(CTeamModelEntry *entry, int32 index)
	{
		teamView->RemoveTeam(entry);
	}
	
	protected:
	CProcessView *teamView;
};

class CProcessItem : public CLVEasyItemEx
{
	public:
	CProcessItem(CTeamModelEntry *_teamModelEntry); 
		
	team_id TeamId() const 			{ return teamModelEntry->TeamId(); }
	bool	IsSystemTeam() const 	{ return teamModelEntry->IsSystemTeam(); }
	bool	IsIdleTeam() const 		{ return teamModelEntry->IsIdleTeam(); }
	
	CTeamModelEntry *Entry() const  { return teamModelEntry; }

	static int CompareItems(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn);
	
	int Compare(const CProcessItem &other, int32 key) const;
	void Update(const system_info *sysInfo, bigtime_t cpuActiveTime);
	
	virtual void DisplayContextMenu(BView *owner, BPoint point);
	
	protected:
	CTeamModelEntry    *teamModelEntry;
	bigtime_t			lastUserTime;		// total user time before last update
	bigtime_t			lastKernelTime;		// total kernel time before last update
};

class CUsagePainter : public CLVTextPainter
{
	public:
	CUsagePainter();

	virtual void DrawItemColumn(BView *owner, BRect item_column_rect, bool complete);

	protected:
	virtual void DrawUsageBar(BView *owner, BRect item_column_rect, bool complete) = 0;

	float maxTextWidth, textHeight;
};

class CMemUsagePainter : public CUsagePainter
{
	public:
	CMemUsagePainter();
	
	void Update(float memUsage);
	float Usage() { return usage; }
	
	protected:
	virtual void DrawUsageBar(BView *owner, BRect item_column_rect, bool complete);

	float usage;
};

class CCPUUsagePainter : public CUsagePainter
{
	public:
	CCPUUsagePainter();
	
	void Update(float totalUsage, float kernelUsage);
	float Usage() { return total; }
	float KernelUsage() { return kernel; }
	
	protected:
	virtual void DrawUsageBar(BView *owner, BRect item_column_rect, bool complete);

	float kernel, total;
};

// ====== globals ======

// Message fields for MSG_ALERT_CLOSED and MSG_TEAM_ACTION
const char * const MESSAGE_DATA_ID_ACTION					= "TEAM:Action";
const char * const MESSAGE_DATA_ID_TEAM_ID					= "TEAM:TeamId";	// only for ALERT_CLOSED
const char * const MESSAGE_DATA_ID_PRIORITY					= "TEAM:Prio";		// only if ACTION == SET_PRIORITY

// Properties of CProcessView
const char * const PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS		= "HideSystemTeams";
const char * const PROCESS_VIEW_PROP_TEAM_MODEL				= "TeamModel";

column_info team_view_colomn_info[] = {
	{ B_TRANSLATE("Name"),				COLUMN_NUM_NAME,			true,	108.0,	50.0	},
	{ B_TRANSLATE("Team-ID"),			COLUMN_NUM_TEAM_ID,			true,	 40.0,	20.0	},
	{ B_TRANSLATE("Thread Count"),		COLUMN_NUM_THREAD_COUNT,	true,	 40.0,	20.0	},
	{ B_TRANSLATE("Area Count"),			COLUMN_NUM_AREA_COUNT,		false,	 40.0,	20.0	},
	{ B_TRANSLATE("Image Count"),		COLUMN_NUM_IMAGE_COUNT,		false,	 40.0,	20.0	},
	{ B_TRANSLATE("CPU Usage"),			COLUMN_NUM_CPU_USAGE,		true,	 80.0,	20.0	},
	{ B_TRANSLATE("Memory Usage (%)"),		COLUMN_NUM_MEM_USAGE,		false,	 80.0,	20.0	},
	{ B_TRANSLATE("Memory Usage (absolute)"),	COLUMN_NUM_MEM_USAGE_ABS,	false,	 40.0,	20.0	},
	{ B_TRANSLATE("Directory"),			COLUMN_NUM_DIRECTORY,		true,	200.0,	50.0	},
	// terminate list
	{ "", 									-1, 						false,	  0.0,	 0.0 	},			
};

// ====== CUsagePainter ======

CUsagePainter::CUsagePainter() :
	CLVTextPainter("0.0 %", true, CColor::Black)
{
	textHeight = maxTextWidth = 0.0;
}

void CUsagePainter::DrawItemColumn(BView *owner, BRect item_column_rect, bool complete)
{
	float width = item_column_rect.Width();

	if(maxTextWidth == 0.0 || textHeight == 0.0) {
		BFont owner_font;
		owner->GetFont(&owner_font);
		
		font_height fh;
		
		owner_font.GetHeight(&fh);
		
		textHeight = fh.ascent;
		
		// The 2.0 is added because the CLVTextPainter displays a 2 pixel
		// distance on both sides of the string. The other 2 pixels are used
		// as distance to the usage bar.
		maxTextWidth = owner_font.StringWidth("100.0 %") + 2.0;
	}

	BRect textRect = item_column_rect;
	
	if(width >= maxTextWidth + 4) {
		BRect borderRect;
	
		float width = item_column_rect.Width() - maxTextWidth - 4;
	
		borderRect = item_column_rect;

		borderRect.top     += (item_column_rect.Height()-textHeight) / 2.0;
		borderRect.bottom	= borderRect.top + textHeight;
		borderRect.left    += maxTextWidth + 2;
		borderRect.right    = borderRect.left + width;
	
		DrawUsageBar(owner, borderRect, complete);
	
		textRect.right = textRect.left + maxTextWidth + 2.0;
	}
	
	CLVTextPainter::DrawItemColumn(owner, textRect, complete);
}

// ====== CMemUsagePainter ======

CMemUsagePainter::CMemUsagePainter()
{
	usage = 0.0;
}

void CMemUsagePainter::Update(float memUsage)
{
	char memUsageString[50];

	sprintf(memUsageString, "%.1f %%", MIN(MAX(memUsage, 0.0), 100.0));
	
	SetText(memUsageString);
	
	usage = memUsage;
}

void CMemUsagePainter::DrawUsageBar(BView *owner, BRect borderRect, bool complete)
{
	BRect barRect, whiteRect;

	float width = borderRect.Width();
	
	barRect = borderRect;
	barRect.top	 	   += 1.0;
	barRect.left  	   += 1.0;
	barRect.bottom 	   -= 1.0;
	barRect.right  		= MIN(barRect.left + (width-2) * usage / 100, borderRect.right-1.0);
	
	whiteRect = borderRect;
	
	whiteRect.top	   += 1.0;
	whiteRect.bottom   -= 1.0;
	whiteRect.left		= barRect.right + 1.0;
	whiteRect.right    -= 1.0;
	
	if(barRect.Width() >= 1.0) {
		owner->SetHighColor(0, 0, 200);
		owner->FillRect(barRect);
	}

	if(whiteRect.Width() >= 1.0) {
		owner->SetHighColor(CColor::White);
		owner->FillRect(whiteRect);
	}
	
	owner->SetHighColor(CColor::Black);
	owner->StrokeRect(borderRect);
}

// ====== CCPUUsagePainter ======

CCPUUsagePainter::CCPUUsagePainter()
{
	kernel = total = 0.0;
}
	
void CCPUUsagePainter::Update(float totalUsage, float kernelUsage)
{
	char cpuUsageString[50];

	sprintf(cpuUsageString, "%.1f %%", MIN(MAX(totalUsage, 0.0), 100.0));
	
	SetText(cpuUsageString);
	
	total = totalUsage;
	kernel = kernelUsage;
}

void CCPUUsagePainter::DrawUsageBar(BView *owner, BRect borderRect, bool complete)
{
	BRect totalRect, kernelRect, whiteRect;
	
	float width = borderRect.Width();
	
	totalRect =  borderRect;

	totalRect.top	   += 1.0;
	totalRect.bottom   -= 1.0;
	
	kernelRect = totalRect;
	
	kernelRect.left     = totalRect.left = borderRect.left + 1;
	kernelRect.right    = MIN(kernelRect.left + (width-2) * kernel / 100, borderRect.right-1.0);
	totalRect.right     = MIN(totalRect.left + (width-2) * total / 100, borderRect.right-1.0);
	
	whiteRect = borderRect;
	
	whiteRect.top	   += 1.0;
	whiteRect.bottom   -= 1.0;
	whiteRect.left		= totalRect.right + 1.0;
	whiteRect.right    -= 1.0;
	
	if(totalRect.Width() >= 1.0) {
		owner->SetHighColor(0, 150, 0);
		owner->FillRect(totalRect);
	}

	if(kernelRect.Width() >= 1.0) {
		owner->SetHighColor(0, 200, 0);
		owner->FillRect(kernelRect);
	}

	if(whiteRect.Width() >= 1.0) {
		owner->SetHighColor(CColor::White);
		owner->FillRect(whiteRect);
	}
	
	owner->SetHighColor(CColor::Black);
	owner->StrokeRect(borderRect);
}

// ====== CProcessItem ======

CProcessItem::CProcessItem(CTeamModelEntry *_teamModelEntry) :
	CLVEasyItemEx(0, false, false, 20.0)
{
	teamModelEntry = _teamModelEntry;

	MY_ASSERT(teamModelEntry->TeamId() != -1);

	char idString[77], threadCountString[77];
	char areaCountString[77], imageCountString[77];

	sprintf(idString, "%ld", teamModelEntry->TeamId());
	sprintf(threadCountString, "%ld", teamModelEntry->ThreadCount()); 
	sprintf(areaCountString, "%ld", teamModelEntry->AreaCount()); 
	sprintf(imageCountString, "%ld", teamModelEntry->ImageCount()); 

	BPath fileName = teamModelEntry->FileName();
	
	const char *name = teamModelEntry->Name();
	
	bool systemTeam = teamModelEntry->IsSystemTeam();
	
	if(fileName.Path() != NULL) {
		BEntry 		appEntry(fileName.Path());
		entry_ref	appEntryRef;

		appEntry.GetRef(&appEntryRef);

		BBitmap *icon = new BBitmap(BRect(0,0,15,15), B_RGBA32);
		
		// This code also returns an icon for applications not on BeFS devices
		// and/or without an icon in their resources.
		if(BNodeInfo::GetTrackerIcon(&appEntryRef, icon, B_MINI_ICON) != B_OK) {
			delete icon;
			icon = NULL;
		}
		
		BPath dir;
		fileName.GetParent(&dir);

		SetColumnContent(COLUMN_NUM_ICON, icon, 2.0, false);	// (don't copy the bitmap!)
		SetColumnContent(COLUMN_NUM_DIRECTORY, dir.Path());
	}

	SetColumnContent(COLUMN_NUM_NAME, 
		new CLVTextPainter(name, false, systemTeam ? CColor::Blue : CColor::Black));	
	
	SetColumnContent(COLUMN_NUM_TEAM_ID, new CLVTextPainter(idString, true));
	SetColumnContent(COLUMN_NUM_THREAD_COUNT, new CLVTextPainter(threadCountString, true));
	SetColumnContent(COLUMN_NUM_AREA_COUNT, new CLVTextPainter(areaCountString, true));
	SetColumnContent(COLUMN_NUM_IMAGE_COUNT, new CLVTextPainter(imageCountString, true));
	SetColumnContent(COLUMN_NUM_CPU_USAGE, new CCPUUsagePainter());
	SetColumnContent(COLUMN_NUM_MEM_USAGE, new CMemUsagePainter());
	SetColumnContent(COLUMN_NUM_MEM_USAGE_ABS, new CLVTextPainter("0 K", true));
	
	lastUserTime   = teamModelEntry->UserTime();
	lastKernelTime = teamModelEntry->KernelTime();
}

void CProcessItem::Update(const system_info *sysInfo, bigtime_t cpuUsage)
{
	bigtime_t userTime   = teamModelEntry->UserTime();
	bigtime_t kernelTime = teamModelEntry->KernelTime();

	char threadCountString[77], areaCountString[77], imageCountString[77];

	sprintf(threadCountString, "%ld", teamModelEntry->ThreadCount()); 
	sprintf(areaCountString, "%ld", teamModelEntry->AreaCount()); 
	sprintf(imageCountString, "%ld", teamModelEntry->ImageCount()); 

	SetColumnContent(COLUMN_NUM_THREAD_COUNT, threadCountString);
	SetColumnContent(COLUMN_NUM_AREA_COUNT, areaCountString);
	SetColumnContent(COLUMN_NUM_IMAGE_COUNT, imageCountString);
		
	if(cpuUsage > 0 && (lastUserTime > 0 || lastKernelTime > 0)) {
		bigtime_t kernelUsageTime = (kernelTime - lastKernelTime);
		bigtime_t totalUsageTime  = (userTime - lastUserTime) + kernelUsageTime;
		
		float percentKernelUsage, percentCpuUsage;
		
		percentCpuUsage    = (totalUsageTime / (float)cpuUsage) * 100.0;
		percentKernelUsage = (kernelUsageTime / (float)cpuUsage) * 100.0;
	
		CCPUUsagePainter *cpuUsagePainter = dynamic_cast<CCPUUsagePainter *>
										(GetColumnContentPainter(COLUMN_NUM_CPU_USAGE));

		cpuUsagePainter->Update(percentCpuUsage, percentKernelUsage);		
	}

	size_t totalAreaSize = teamModelEntry->AreaSize();
		
	float percentMemUsage;
		
	percentMemUsage = (totalAreaSize / (float)(sysInfo->used_pages * B_PAGE_SIZE)) * 100.0;
		
	CMemUsagePainter *memUsagePainter = dynamic_cast<CMemUsagePainter *>
											(GetColumnContentPainter(COLUMN_NUM_MEM_USAGE));

	memUsagePainter->Update(percentMemUsage);							
	
	char absMemUsage[77];
		
	sprintf(absMemUsage, "%ld K", totalAreaSize / 1024);
		
	SetColumnContent(COLUMN_NUM_MEM_USAGE_ABS, absMemUsage);
		
	lastUserTime   = userTime;
	lastKernelTime = kernelTime;
}

void CProcessItem::DisplayContextMenu(BView *owner, BPoint point)
{
	CAsynchronousPopUpMenu *contextMenu = 
		new CAsynchronousPopUpMenu("Process", false);

	BMessage *killMessage = new BMessage(MSG_TEAM_ACTION);
	killMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_KILL);
	
	BMessage *activateMessage = new BMessage(MSG_TEAM_ACTION);
	activateMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_ACTIVATE);

	BMessage *quitMessage = new BMessage(MSG_TEAM_ACTION);
	quitMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_QUIT);

	// "Activate" is the default item of this menu. (Displayed using a bold font).
	contextMenu->AddItem(
		new CGlyphMenuItem(
			B_TRANSLATE("Activate"),
			NULL,
			true,
			activateMessage));
	contextMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Quit"), quitMessage));
	contextMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Kill"), killMessage));

	contextMenu->AddSeparatorItem();
	contextMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Add to Performance Tab..."), 
			new BMessage(MSG_VIEW_PERFORMANCE_OBJECTS)));
	
	contextMenu->SetTargetForItems(owner);
	
	app_info appInfo;

	be_roster->GetRunningAppInfo(TeamId(), &appInfo);
	
	status_t create_stat;

	BMessenger appMessenger(NULL, TeamId(), &create_stat);
	
	if(create_stat != B_OK || appInfo.flags & (B_BACKGROUND_APP | B_ARGV_ONLY)) {
		// disable the possibility to activate if the application
		// is a background application (without a window).
		contextMenu->ItemAt(0)->SetEnabled(false);
	}

	if(create_stat != B_OK) {
		// I can't send a B_QUIT_REQUESTED message to an application without
		// a BApplication (looper).
		contextMenu->ItemAt(1)->SetEnabled(false);
	}
	
	// Add prio submenu
	BMenu *prioSubMenu = new BMenu(B_TRANSLATE("Priority"));

	const int32 maxEntries=5;
	struct {
		const char *key;
		int32 prio;
	} menuInfo[maxEntries] = {
		{ B_TRANSLATE_MARK("Low Priority"),			 5, },
		{ B_TRANSLATE_MARK("Normal Priority"),			10, },
		{ B_TRANSLATE_MARK("Display Priority"),		15, },
		{ B_TRANSLATE_MARK("Urgent Display Priority"),	20, },
		{ B_TRANSLATE_MARK("Realtime Priority"),	   100, },
	};
	
	for(int32 i=0 ; i<maxEntries ; i++) {
		BMessage *msg = new BMessage(MSG_TEAM_ACTION);
		
		msg->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_SET_PRIORITY);
		msg->AddInt32(MESSAGE_DATA_ID_PRIORITY, menuInfo[i].prio);
	
		prioSubMenu->AddItem(new BMenuItem(menuInfo[i].key, msg));
	}
	
	prioSubMenu->SetTargetForItems(owner);
	prioSubMenu->SetRadioMode(true);

	// Don't allow user to set team to realtime priority. This
	// is a simple safety feature.
	prioSubMenu->ItemAt(4)->SetEnabled(false);

	int32 basePrio;
	bool disablePrioMenu=false;
	
	if(CTeam(TeamId()).GetBasePriority(basePrio) == B_OK) {
		if(0 <= basePrio && basePrio <= 5) {
			prioSubMenu->ItemAt(0)->SetMarked(true);
		} else if(6 <= basePrio && basePrio <= 10) {
			prioSubMenu->ItemAt(1)->SetMarked(true);
		} else if(11 <= basePrio && basePrio <= 15) {
			prioSubMenu->ItemAt(2)->SetMarked(true);
		} else if(16 <= basePrio && basePrio <= 99) {
			prioSubMenu->ItemAt(3)->SetMarked(true);
		} else if(100 <= basePrio && basePrio <= 120) {
			// Disable all items. Don't let user change
			// prio of realtime teams.
			disablePrioMenu = true;
	
			prioSubMenu->ItemAt(4)->SetMarked(true);
		}
	} else {
		// The team_id seems to be invalid. Disable all priority settings.
		disablePrioMenu = true;
	}

	if(disablePrioMenu) {
		for(int i=0 ; i<prioSubMenu->CountItems() ; i++)
			prioSubMenu->SetEnabled(false);
	}
	
	contextMenu->AddSeparatorItem();
	contextMenu->AddItem(prioSubMenu);

	contextMenu->SetNotificationTarget(owner);

	// allow the user to move the mouse by 3 points in every direction
	// without closing the menu, when the mouse is released.
	BRect clickToOpen(point.x-3, point.y-3, point.x+3, point.y+3);
	
	// open menu (asynchonous)
	contextMenu->Go(point, true, clickToOpen, true);
}

int CProcessItem::CompareItems(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn)
{
	CProcessItem *pi1 = (CProcessItem *)a_Item1;
	CProcessItem *pi2 = (CProcessItem *)a_Item2;
	
	if(pi1 == NULL || pi2 == NULL)
		return 0;
		
	return pi1->Compare(*pi2, KeyColumn);
}	
	
int CProcessItem::Compare(const CProcessItem &other, int32 key) const
{
	switch(key) {
		case COLUMN_NUM_DIRECTORY:
			// FALL THROUGH
		case COLUMN_NUM_NAME:
		{
			const char *myText    = GetColumnContentText(key); 
			const char *otherText = other.GetColumnContentText(key);
			
			if(myText == NULL)
				return 1;
				
			if(otherText == NULL)
				return -1;
		
			return strcasecmp(myText, otherText);
		}
		case COLUMN_NUM_TEAM_ID:
			return TeamId() - other.TeamId();
		case COLUMN_NUM_THREAD_COUNT:
			// FALL THROUGH
		case COLUMN_NUM_AREA_COUNT:
			// FALL THROUGH
		case COLUMN_NUM_IMAGE_COUNT:
			return atoi(GetColumnContentText(key)) - atoi(other.GetColumnContentText(key));
		case COLUMN_NUM_CPU_USAGE:
		{
			CCPUUsagePainter *painter1 = dynamic_cast<CCPUUsagePainter *>
											(GetColumnContentPainter(COLUMN_NUM_CPU_USAGE));
			CCPUUsagePainter *painter2 = dynamic_cast<CCPUUsagePainter *>
											(other.GetColumnContentPainter(COLUMN_NUM_CPU_USAGE));
			
		
			return (int)((painter1->Usage() - painter2->Usage()) * 100);
		}
		case COLUMN_NUM_MEM_USAGE:
			// FALL THROUGH
		case COLUMN_NUM_MEM_USAGE_ABS:
		{
			CMemUsagePainter *painter1 = dynamic_cast<CMemUsagePainter *>
											(GetColumnContentPainter(COLUMN_NUM_MEM_USAGE));
			CMemUsagePainter *painter2 = dynamic_cast<CMemUsagePainter *>
											(other.GetColumnContentPainter(COLUMN_NUM_MEM_USAGE));
		
			return (int)((painter1->Usage() - painter2->Usage()) * 100);
		}
		default:
			return 0;
	}
}

// ====== CProcessView ======

CProcessView::CProcessView(BRect rect) :
	CTabNotifcationView(rect, "ProcessView", B_FOLLOW_ALL_SIDES,  B_FRAME_EVENTS | B_PULSE_NEEDED)		
{
	lastUpdateTime = 0;
	sortItems = true;

	teamModel = NULL;

	// --- Create buttons

	BRect dummyRect(0,0,5,5);

	const float minButtonWidth=90, minButtonHeight=22;
	float bw, bh;
	float buttonWidth, buttonHeight;

	BMessage *killMessage = new BMessage(MSG_TEAM_ACTION);
	killMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_KILL);

	killButton = new CFlickerFreeButton(dummyRect, "KillButton", 
						B_TRANSLATE("Kill"), 
						killMessage, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	
	selectTeamButton = new CFlickerFreeButton(dummyRect, "SelectTeamButton", 
						B_TRANSLATE("X-Kill..."),
						new BMessage(MSG_SELECT_AND_KILL_TEAM), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	
	killButton->GetPreferredSize(&bw, &bh);
	
	buttonWidth  = MAX(minButtonWidth, bw);
	buttonHeight = MAX(minButtonHeight, bh);

	selectTeamButton->GetPreferredSize(&bw, &bh);

	buttonWidth  = MAX(buttonWidth, bw);
	buttonHeight = MAX(buttonHeight, bh);

	killButton->MoveTo(Bounds().right-buttonWidth-7, Bounds().bottom-buttonHeight-7);
	killButton->ResizeTo(buttonWidth, buttonHeight);	

	selectTeamButton->MoveTo(Bounds().left+7, Bounds().bottom-buttonHeight-7);
	selectTeamButton->ResizeTo(buttonWidth, buttonHeight);	

	// Initally the button is disabled. If a entry in the listview is selected 
	// it's enabled.
	killButton->SetEnabled(false);

	// --- Create listview

	BRect listViewRect(7,7,
				Bounds().right-B_V_SCROLL_BAR_WIDTH-10,
				Bounds().bottom-B_H_SCROLL_BAR_HEIGHT-buttonHeight-17);
	
	CLVContainerView* containerView;
	
	listView = new CColumnListViewEx(listViewRect,				// frame 
									&containerView,				// container [out]
									"ProcessColumnListView",	// name
									B_FOLLOW_ALL_SIDES,
									B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE,
									B_SINGLE_SELECTION_LIST,
									false,						// hierarchical
									true,						// horizontal scroll bar
									true,						// vertical scroll bar
									true,						// scroll view corner
									B_FANCY_BORDER);
	
	// Column for icons
	listView->AddColumn(new CLVColumn(NULL, 20.0,
		CLV_LOCK_AT_BEGINNING|CLV_NOT_MOVABLE|
		CLV_NOT_RESIZABLE|CLV_PUSH_PASS|CLV_MERGE_WITH_RIGHT));

	CTaskManagerPrefs prefs;
		
	//  Add string columns
	for(column_info *column=team_view_colomn_info ; column->key[0] != 0 ; column++) {
		uint32 stdFlags = CLV_SORT_KEYABLE|CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH;
	
		listView->AddColumn(
			new CLVColumn(
				BString(column->key), 
				prefs.ColumnWidth(column->index, column->defaultWidth),
				prefs.ColumnVisible(column->index, column->defaultVisible) ? stdFlags : stdFlags|CLV_HIDDEN,
			 	column->minWidth
			 )
		); 
	}	
	
	// set sort function
	listView->SetSortFunction(CProcessItem::CompareItems);

	BMessage *activateMessage = new BMessage(MSG_TEAM_ACTION);
	activateMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_ACTIVATE);

	listView->SetInvocationMessage(activateMessage);
	listView->SetSelectionMessage(new BMessage(MSG_TEAM_SELECTED));

	// restore sort keys
	CAPointer<sort_key_info> sortKeyInfo = prefs.SortKeyInfo();

	int32 numSortKeys = prefs.SortKeyInfoCount();

	if(numSortKeys == 0) {	
		// default is sort by name
		listView->SetSortKey(1);
	} else {
		for(int32 i=0 ; i<numSortKeys ; i++) {
			if(sortKeyInfo[i].sortMode != NoSort) {
				listView->AddSortKey(sortKeyInfo[i].columnIndex);
				listView->SetSortMode(sortKeyInfo[i].columnIndex, 
									  (CLVSortMode)sortKeyInfo[i].sortMode);
			}
		}
	}

	// restore display order
	CAPointer<int32> displayOrder = prefs.ColumnDisplayOrder(listView->CountColumns());
	listView->SetDisplayOrder(displayOrder);

	AddChild(containerView);
	AddChild(selectTeamButton);
	AddChild(killButton);
}

void CProcessView::Pulse()
{
	if(!IsHidden()) UpdateListView();
}

void CProcessView::AttachedToWindow()
{
	CTabNotifcationView::AttachedToWindow();
	
	SetViewColor(CColor::BeBackgroundGray);

	// I can only create BMessenger objects after the
	// view (handler) was added to a BLooper.
	killButton->SetTarget(this);

	selectTeamButton->SetTarget(this);

	listView->SetTarget(this);

	hideSystemTeams = CTaskManagerPrefs().HideSystemTeams();
	
	teamModel = new CTeamModel(Window());

	teamModelListener = new CTeamModelListener(this);
	
	teamModel->AddTeamModelListener(teamModelListener);
	
	UpdateListView();
}

void CProcessView::DetachedFromWindow()
{
	if(teamModel != NULL) {
		teamModel->RemoveTeamModelListener(teamModelListener);

		delete teamModelListener;
		delete teamModel;
		
		teamModel = NULL;
	}

	// Store settings
	CTaskManagerPrefs prefs;

	// Width of each column
	for(int column=0 ; column<listView->CountColumns() ; column++)
		prefs.SetColumnWidth(column, listView->ColumnAt(column)->Width());

	// Sortkey information
	int32 *      sortKeys  = new int32 [listView->CountColumns()];
	CLVSortMode *sortModes = new CLVSortMode [listView->CountColumns()];

	int32 numKeys = listView->Sorting(sortKeys, sortModes);
	
	sort_key_info *sortKeyInfo = new sort_key_info[numKeys];
	
	for(int32 i=0 ; i<numKeys ; i++) {
		sortKeyInfo[i].columnIndex	= sortKeys[i];
		sortKeyInfo[i].sortMode		= sortModes[i];
	}
	
	prefs.SetSortKeyInfo(sortKeyInfo, numKeys);
	
	delete [] sortKeys;
	delete [] sortModes;
	delete [] sortKeyInfo;

	// Display order of columns
	int32 *displayOrder = listView->DisplayOrder();

	prefs.SetColumnDisplayOrder(displayOrder, listView->CountColumns());
	
	delete [] displayOrder;

	CTabNotifcationView::DetachedFromWindow();
}

CProcessItem *CProcessView::FindItem(team_id teamId)
{
	for(int i=0 ; i<listView->CountItems() ; i++) {
		CProcessItem *item = dynamic_cast<CProcessItem *>(listView->ItemAt(i));
		
		if(item && item->TeamId() == teamId) {
			return item;
		}
	}
	
	return NULL;
}

// ShowWarning
// Displays an extended alert with a "Show this dialog again" checkbox.
// The alert is only displayed, if the preferences setting with prefsName
// has the value 'true' (as B_TYPE_BOOL). If 'asynchron' is true, the function
// returns immediatly and a message with the button id in the 'which' field
// is sent to this view. The other fields are copied from 'msg'. The alert
// owns the message! Don't delete it yourself.
// Return:
//  0   if the user selected button0
//  1   if the user selected cancel or pressed ESC.
// -1   if the alert wasn't displayed.
// -2	the alert was displayed and asynchron is true.
int32 CProcessView::ShowWarning(const char *text, const char *button0, 
	const char *prefName, bool asynchron, BMessage *msg)
{
	CTaskManagerPrefs *prefs = new CTaskManagerPrefs();

	bool showWarning;

	prefs->Read(prefName, showWarning, true);

	if(showWarning) {
		BAlert *alert = new CAlertEx(
								"Warning",
								text,
								button0,
							    cancel_button_label(),
								NULL,
								B_WIDTH_AS_USUAL,
								B_EVEN_SPACING,
								B_WARNING_ALERT,
								B_TRANSLATE("Show this dialog again"),
								prefs,
								prefName,
								showWarning);

		alert->SetShortcut(1, B_ESCAPE);
	
		if(asynchron) {
			BInvoker *invoker = new BInvoker(msg, this);
			
			alert->Go(invoker);
			
			return -2;
		}
	
		return alert->Go();
	}
	
	// normally the message and prefs are deleted
	// by the alert. Now I must do this by hand...
	delete msg;
	delete prefs;
	
	return -1;
}

status_t CProcessView::SetHideSystemTeams(bool newValue)
{
	if(hideSystemTeams == newValue)
		return B_OK;
	
	hideSystemTeams = newValue;
	
	int32 itemCount = listView->CountItems();
	
	listView->RemoveItems(0, itemCount);
	
	BAutolock teamModelLock(teamModel->Looper());
	
	for(int32 i=0 ; i<teamModel->CountEntries() ; i++) {
		CTeamModelEntry *entry = teamModel->EntryAt(i);
	
		if(!(hideSystemTeams && entry->IsSystemTeam()))
			listView->AddItem(new CProcessItem(entry));
	}
	
	listView->SortItems();
	
	return B_OK;
}

BHandler *CProcessView::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_SET_PROPERTY:
		case B_GET_PROPERTY:
			if( (strcmp(property, PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS) == 0 || 
				 strcmp(property, PROCESS_VIEW_PROP_TEAM_MODEL) == 0) &&
				what == B_DIRECT_SPECIFIER) {
				
				return this;
			}
			break;
	}
	
	return CTabNotifcationView::ResolveSpecifier(message, index, specifier, what, property);
}

void CProcessView::ActivateTeam(team_id id)
{
	if(CTeam(id).Activate() != B_OK) {
		beep();
	}
}

// called by message handler of MSG_TEAM_ACTION
void CProcessView::KillTeamWithWarning(team_id id)
{
	CTeam team(id);	

	if(team.IsIdleTeam()) {
		show_alert_async(B_TRANSLATE("You can't kill the kernel team!"),
					NULL, "Warning", B_STOP_ALERT);
		
		return;
	}

	BString warning;
	BString name;
	
	if(team.GetName(&name) != B_OK)
		name << "#" << id;

	const char *prefName = PREF_SHOW_KILL_WARNING;

	if(team.IsSystemTeam()) {
		warning << B_TRANSLATE("The team '\0' is a component of the BeOS.\n\nKill the team anyway??") << name;
		
		warning << name;
				  
		prefName = PREF_SHOW_SYSTEM_KILL_WARNING;
	} else {
		warning << B_TRANSLATE("Are you sure that you want to kill the team '\0'??") << name;
	
		warning << name;
	}
	
	BMessage *msg = new BMessage(MSG_ALERT_CLOSED);
	
	msg->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_KILL);
	msg->AddInt32(MESSAGE_DATA_ID_TEAM_ID, id);

	int32 result;

	const char *killButton = B_TRANSLATE("Kill");

	// Shows a warning, if the user didn't disable it in
	// the preferences.
	result = ShowWarning(
		warning.String(),
		killButton,
		prefName,
		true,
		msg);

	if(result != -1) {
		// The dialog was displayed (not disabled!). 
		// The alert passes its result as message.
		return;
	}

	KillTeam(id);
}

// called by KillTeamWithWarning(team_id) and handler of MSG_ALERT_CLOSED
void CProcessView::KillTeam(team_id id)
{
	status_t result;

	CTeam team(id);

	if((result = team.Kill()) != B_OK) {
		BString teamName;
		BString message;
		CProcessItem *item = FindItem(id);
	
		if(item) {
			teamName << "'" << item->GetColumnContentText(COLUMN_NUM_NAME) << "'";
		} else {
			teamName << "#" << (int32)id;
		}
	
		if(result == E_CONTROLLED_BY_DEBUGGER) {
			// Team is controlled by debugger. Ask user if he/she wants to
			// kill the debugger.

			team_id debuggerTeam;

			if(team.GetDebuggerTeam(debuggerTeam) == B_OK) {
				message << B_TRANSLATE("The team '\0' is controlled by a debugger.\nDo you want to kill the debugger??");

				message << teamName;
				
				BMessage *msg = new BMessage(MSG_ALERT_CLOSED);
		
				msg->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_KILL);
				msg->AddInt32(MESSAGE_DATA_ID_TEAM_ID, debuggerTeam);

				BAlert *alert = new BAlert(
										"Question",
										message.String(),
										"Kill Debugger",
									    cancel_button_label(),
										NULL,
										B_WIDTH_FROM_WIDEST,
										B_EVEN_SPACING,
										B_WARNING_ALERT);
	
				alert->SetShortcut(1, B_ESCAPE);
	
				// when the alert is close I receive a MSG_ALERT_CLOSED message, which
				// tells me to kill the debugger (or not, if the user selected cancel).
				alert->Go(new BInvoker(msg, this));
			}
		} else {
			message << B_TRANSLATE("Can't kill team '\0'.\nReason: \1");
			message << teamName;
			message << strerror(result);
	
			show_alert(message, "Error");
		}
	}
}

void CProcessView::TeamSelected(int32 selIndex)
{
	// enable or disable kill button
	if(selIndex < 0)
		killButton->SetEnabled(false);
	else
		killButton->SetEnabled(true);
}

void CProcessView::QuitTeam(team_id id)
{
	// Execute this operation in this own thread.
	// If any error occours a MSG_ASYNC_TEAM_ACTION_FAILED message 
	// is sent to this window, containing the error in its
	// MESSAGE_DATA_ID_ERROR_CODE data field.
	
	CTeam *team = new CTeam(id);
		
	BInvoker *invoker = new BInvoker(new BMessage(MSG_ASYNC_TEAM_ACTION_FAILED), this);
		
	CAsyncMessageTeamAction *teamAction = new CAsyncMessageTeamAction(team, TEAM_ACTION_QUIT, invoker);
	
	teamAction->Run();
}

void CProcessView::MessageReceived(BMessage *message)
{
	switch(message->what) {
	/*
	case B_OBSERVER_NOTICE_CHANGE:
		// The model changed
		
		int32 which;
		
		message->FindInt32("be:observe_change_what", &which);
		
		switch(which) {
			case MSG_NOTIFY_ITEM_ADDED:
				{
					CTeamModelEntry *entry;
					
					if(message->FindPointer(MESSAGE_DATA_ID_ITEM, (void **)&entry) == B_OK) {
						AddTeam(entry, true);
					}
				}
				break;
			case MSG_NOTIFY_ITEM_REMOVED:
				{
					CTeamModelEntry *entry;
				
					if(message->FindPointer(MESSAGE_DATA_ID_ITEM, (void **)&entry) == B_OK) {
						RemoveTeam(entry);
					}
				}
				break;
		}
		*/
	case B_SET_PROPERTY:
		{
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
				if(strcmp(property, PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS) == 0 && 
					what == B_DIRECT_SPECIFIER) {
					// SET_PROPERTY for DisplaySystemTeams
					
					status_t result;
					bool newHideSystemTeams;
					
					if((result = message->FindBool("data", &newHideSystemTeams)) == B_OK) {
						result = B_OK;
						
						message->PopSpecifier();
					}
					
					result = SetHideSystemTeams(newHideSystemTeams);
					
					BMessage reply(B_REPLY);
					
					send_script_reply(reply, result, message);
					
					return;
				}
			}
			
			CTabNotifcationView::MessageReceived(message);
		}
		break;
	case B_GET_PROPERTY:
		{
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
				if(strcmp(property, PROCESS_VIEW_PROP_HIDE_SYSTEM_TEAMS) == 0 && 
					what == B_DIRECT_SPECIFIER) {
					// GET_PROPERTY for DisplaySystemTeams
					
					BMessage reply(B_REPLY);

					reply.AddBool("result", hideSystemTeams);

					send_script_reply(reply, B_OK, message);
					message->PopSpecifier();
					
					return;
				}
				
				if(strcmp(property, PROCESS_VIEW_PROP_TEAM_MODEL) == 0 && 
					what == B_DIRECT_SPECIFIER) {
					// GET_PROPERTY for TeamModel
					
					BMessage reply(B_REPLY);

					BMessenger teamModelMessenger(teamModel);

					reply.AddMessenger("result", teamModelMessenger);

					send_script_reply(reply, B_OK, message);
					message->PopSpecifier();
					
					return;
				}
			}
			
			CTabNotifcationView::MessageReceived(message);
		}
		break;
	case MSG_ALERT_CLOSED:
		// This message is received, if an asyncrounous alert is closed.
		// The action depends on the button, which was pressed and which
		// operation opened the alert.

		{
			int32 buttonId;
			enumTeamAction action;
			team_id id;
			
			if(message->FindInt32("which", &buttonId) == B_OK && 
			   message->FindInt32(MESSAGE_DATA_ID_ACTION, (int32 *)&action) == B_OK &&
			   message->FindInt32(MESSAGE_DATA_ID_TEAM_ID, (int32 *)&id) == B_OK) {
			   
				if(action == TEAM_ACTION_KILL) {
					if(buttonId == 0) {
						// The user selected "Kill"
						KillTeam(id);		
					} else if(buttonId == 1) {
						// The user selected "Cancel"
					} else {
						MY_ASSERT(!"Invalid button id!!");
					}
				} else if(action == TEAM_ACTION_QUIT) {
					if(buttonId == 0) {
						// The user selected "Retry"
						QuitTeam(id);
					} else if(buttonId == 1) {
						// The user selected "Kill"
						KillTeam(id);
					} else if(buttonId == 2) {
						// The user selected "Cancel"
					}
				}
			}
		}
		break;
	case MSG_ASYNC_TEAM_ACTION_FAILED:
		// An asynchronous team action failed.
	
		{
			status_t errorCode;
			enumTeamAction action;
			team_id teamId;
			
			if(message->FindInt32(MESSAGE_DATA_ID_ERROR_CODE, (int32 *)&errorCode) == B_OK &&
				message->FindInt32(MESSAGE_DATA_ID_TEAM_ID, (int32 *)&teamId) == B_OK &&
				message->FindInt32(MESSAGE_DATA_ID_ACTION, (int32 *)&action) == B_OK) {

				CProcessItem *item = FindItem(teamId);
						
				if(item) {
					switch(action) {
						case TEAM_ACTION_QUIT:
							if(errorCode == B_TIMED_OUT) {
								// SendMessage(B_QUIT_REQUESTED) timed out.
								// Application hangs!
							
								BString text(B_TRANSLATE("The application '\0' isn't responding."));
						
								text << item->GetColumnContentText(COLUMN_NUM_NAME);
						
								const char *killButton  = B_TRANSLATE("Kill");
								const char *retryButton = B_TRANSLATE("Retry");
						
								BAlert *alert = new BAlert("App not responding",			// Title
														text.String(),						// Text
														retryButton,						// Button 0
														killButton,							// Button 1
														cancel_button_label(),				// Button 2
														B_WIDTH_AS_USUAL,					// widthStyle
														B_WARNING_ALERT);					// type
							
								alert->SetShortcut(2, B_ESCAPE);
						
								BMessage *closeMessage = new BMessage(MSG_ALERT_CLOSED);
						
								closeMessage->AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_QUIT);
								closeMessage->AddInt32(MESSAGE_DATA_ID_TEAM_ID, teamId);
						
								alert->Go(new BInvoker(closeMessage, this));
							}
							break;
						case TEAM_ACTION_KILL:
							{
								BString message;
			
								message << "Can't kill '"
										<< item->GetColumnContentText(COLUMN_NUM_NAME)
										<< "'.\nReason: " << strerror(errorCode);
		
								show_alert_async(message, this, "Error");
							}
							break;
						default:
							beep();
					}
				} else {
					// this team is no longer part of my list.
					// Perhaps it was already killed...
				}
			}
		}
		break;
	case MSG_TEAM_ACTION:
		// The user did one of the following things:
		// * Select a context menu entry.
		// * Click the kill button.
	
		{
			enumTeamAction action;
			team_id teamId=-1;
				
			if(message->FindInt32(MESSAGE_DATA_ID_ACTION, (int32 *)&action) == B_OK) {
				if(message->FindInt32(MESSAGE_DATA_ID_TEAM_ID, (int32 *)&teamId) != B_OK) {
					// The message doesn't contain a team_id. Use the team_id of
					// the team selected in the listview.
					int32 selIndex = listView->CurrentSelection(0);
			
					if(selIndex >= 0) {
						CProcessItem *processItem = dynamic_cast<CProcessItem *>(listView->ItemAt(selIndex));
						
						MY_ASSERT(processItem);
				
						teamId = processItem->TeamId();
					} else {
						teamId = -1;
					}
				}
					
				if(teamId != -1) {	
					switch(action) {
						case TEAM_ACTION_ACTIVATE:
							ActivateTeam(teamId);
							break;
						case TEAM_ACTION_KILL:
							KillTeamWithWarning(teamId);
							break;
						case TEAM_ACTION_QUIT:
							QuitTeam(teamId);
							break;
						case TEAM_ACTION_SET_PRIORITY:
							{
								int32 newBasePrio;
								
								if(message->FindInt32(MESSAGE_DATA_ID_PRIORITY, &newBasePrio) == B_OK) {
									CTeam(teamId).SetBasePriority(newBasePrio);
								}
							}
							break;
						default:
							MY_ASSERT(!"Invalid enumTeamAction value");
					}
				} else {
					// no team selected
					beep();
				}
			 }
		}	
		break;
	case MSG_SELECT_AND_KILL_TEAM:
		{
			// Switch to teams tab
			// This is done because this view isn't neccessary selected when
			// this message is sent by the deskbar replicant.
			dynamic_cast<BTabView *>(Parent()->Parent())->Select(TAB_ID_TEAMS);
		
			CSelectTeamWindow *selectTeamWindow = CSelectTeamWindow::CreateInstance();
			
			selectTeamWindow->SetTarget(this);
		}
		break;
	case MSG_TEAM_SELECTED:
		{
			TeamSelected(listView->CurrentSelection(0));
		}
		break;
	case MSG_SELECT_TEAM:
		{
			// Select the specified team in the team list.
		
			team_id teamId;
		
			if(message->FindInt32(MESSAGE_DATA_ID_TEAM_ID, (int32 *)&teamId) == B_OK) {
				int currentSel;
			
				if((currentSel = listView->CurrentSelection(0)) != -1) {
					CProcessItem *processItem = dynamic_cast<CProcessItem *>(listView->ItemAt(currentSel));
					
					if(processItem->TeamId() == teamId) {
						// Already the correct team selected
						return;
					}
				}
				
				for(int i=0 ; i<listView->CountItems() ; i++) {
					CProcessItem *processItem = dynamic_cast<CProcessItem *>(listView->ItemAt(i));
					
					if(processItem->TeamId() == teamId) {
						// Found the correct entry. Select it.
						listView->Select(i);
						listView->ScrollToSelection();
						break;
					}
				}
			}
		}
		break;
	case MSG_VIEW_PERFORMANCE_OBJECTS:
		{
			// User selected "Add to Performance Tab" entry in context menu.
		
			int32 currentSel = listView->CurrentSelection(0);
			
			if(currentSel >= 0) {
				CProcessItem *processItem = dynamic_cast<CProcessItem *>(listView->ItemAt(currentSel));
					
				MY_ASSERT(processItem);
				
				team_id teamId = processItem->TeamId();

				BString counterPath;
			
				// Path of performance counter
				counterPath << "/Teams/" << teamId;
			
				// Display dialog
				CPerformanceView::DisplayAddDialog(counterPath.String());
				
				// Switch to performance tab
				dynamic_cast<BTabView *>(Parent()->Parent())->Select(TAB_ID_PERFORMANCE);
			}
		}
		break;
	case MSG_MENU_BEGINNING:
		{
			// Disable sorting, while the menu is displayed. Otherwise the
			// items move under the menu, which is very confusing.
			sortItems = false;
		}
		break;
	case MSG_MENU_ENDED:
		{
			// Context menu was closed. Enable sorting
			sortItems = true;
			listView->SortItems();
		}
		break;
	default:
		CTabNotifcationView::MessageReceived(message);
	}
}

void CProcessView::AddTeam(CTeamModelEntry *entry, bool sort)
{
	bool hideSystemTeams = CTaskManagerPrefs().HideSystemTeams();

	if(hideSystemTeams && entry->IsSystemTeam()) {
		// Don't add to list.
	} else {
		CProcessItem *item = new CProcessItem(entry);
		
		listView->AddItem(item);

		if(sort && sortItems) {
			listView->SortItems();
		}
	}
}

void CProcessView::RemoveTeam(CTeamModelEntry *entry)
{
	for(int i=0 ; i<listView->CountItems() ; i++) {
		CProcessItem *processItem = dynamic_cast<CProcessItem *>(listView->ItemAt(i));
		
		MY_ASSERT(processItem != NULL);
		
		if(processItem->Entry() == entry) {
			listView->RemoveItem(i);
			delete processItem;
			break;
		}
	}
}

void CProcessView::Select(BView *owner)
{
	// this method gets called, if the tab which contains this
	// view is selected. 'owner' is a pointer to the BTabView.
	
	UpdateListView();
}

void CProcessView::Deselect()
{
	// this method gets called, if the tab which contains this
	// view is deselected.
}

void CProcessView::UpdateListView()
{
	// This function is nessecary, because I only get notifyed by
	// the roster, if a desktop application is started. I DONT
	// get notified, if a shell application is launched. So
	// (from time to time) I have to refresh the team list.

	BAutolock windowAutoLocker(Window());

	if(!teamModel)
		return;

	BAutolock teamModelAutoLocker(teamModel->Looper());
	
	system_info sysInfo;

	// maximum active time (commulated of all CPUs).
	bigtime_t cpuActiveTime=0;
	
	bigtime_t timeSinceLastUpdate = (lastUpdateTime != 0) ? (system_time()-lastUpdateTime) : Window()->PulseRate();
		
	if(get_system_info(&sysInfo) == B_OK) {
		cpuActiveTime = sysInfo.cpu_count * timeSinceLastUpdate;
	}	

	// store time to calc time between updates (for next call)
	lastUpdateTime = system_time();

	// remeber old selection
	CProcessItem *selItem = (CProcessItem *)listView->ItemAt(listView->CurrentSelection(0));

	// Update model
	teamModel->Update();

	// Update entries	
	for(int i=0 ; i<listView->CountItems() ; i++) {
		CProcessItem *listViewItem = (CProcessItem *)listView->ItemAt(i);

		listViewItem->Update(&sysInfo, cpuActiveTime);
	}
	
	if(sortItems) {
		listView->SortItems();
	} else {
		// Without the sorting the listview isn't updated.
		listView->Invalidate();
	}

	// restore selection
	int32 newSelection = listView->IndexOf(selItem);
	listView->Select(newSelection);
}
