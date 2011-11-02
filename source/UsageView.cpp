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
#include "signature.h"
#include "SystemInfo.h"
#include "BugfixedDragger.h"
#include "BorderView.h"
#include "GraphView.h"
#include "LedView.h"
#include "UsageView.h"
#include "BorderView.h"
#include "Box.h"
#include "CounterNamespaceImpl.h"

#include "my_assert.h"

#include <Catalog.h>
#include <Locale.h>
#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "UsageView"

// ====== globals ======

// message data fields for MSG_CPU_NUM_SEL
const char * const MESSAGE_DATA_ID_CPU_NUM		= "CPUGRAPHVIEW:CPUNum";

// colors
const rgb_color	DEFAULT_LINE_COLOR 			    = { 0, 255, 0, 255 };
const rgb_color	CPU_LINE_COLOR					= DEFAULT_LINE_COLOR;
const rgb_color	MEM_LINE_COLOR					= { 255, 255, 0, 255 };

// ====== CCPUGraphView ======

CCPUGraphView::CCPUGraphView(BRect rect, int32 cpuNum) :
	CGraphView(rect, "CPU Usage Graph", 100)
{
	char path[255];
	sprintf(path, "/Total/CPU Usage/CPU %ld", cpuNum+1);

	IDataProvider *dataProvider = global_Namespace->DataProvider(path);

	AddDataProvider(dataProvider, CPU_LINE_COLOR);
}

CCPUGraphView::CCPUGraphView(BMessage *archive) :
	CGraphView(archive)
{
}

BArchivable *CCPUGraphView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CCPUGraphView"))
		return NULL;
		
	return new CCPUGraphView(archive);
}

status_t CCPUGraphView::Archive(BMessage *data, bool deep) const
{
	CGraphView::Archive(data, deep);

	// data->AddString("class", "CCPUGraphView");
	data->AddString("add_on", APP_SIGNATURE);
	
	return B_OK;
}

BPopUpMenu *CCPUGraphView::ContextMenu()
{
	BPopUpMenu *contextMenu = CGraphView::ContextMenu();

	system_info sysInfo;
	get_cached_system_info(&sysInfo, NULL);

	if(sysInfo.cpu_count > 1) {
		// more than one CPU. Add CPU Selection to context menu.

		// I assume that there is only one DataInfo object
		// in the list, which contains a CCPUDataProvider.
		
		CDataInfo *dataInfo = dataInfoList.ItemAt(0);
		ICPUDataProvider *dataProvider = NULL;
		
		if(dataInfo)
			dataProvider = dynamic_cast<ICPUDataProvider *>(dataInfo->DataProvider());
		
		if(dataProvider) {
			int32 cpuNum = dataProvider->CPUNum();

			BMenu *cpuSubMenu = new BMenu(B_TRANSLATE("CPU"));
		
			for(int32 i=-1 ; i<sysInfo.cpu_count ; i++) {
				char buffer[255];
			
				if(i >= 0)
					sprintf(buffer, B_TRANSLATE("CPU %ld"), i+1);
				else
					strcpy(buffer, B_TRANSLATE("Average of all CPUs"));
			
				BMessage *selMessage = new BMessage(MSG_CPU_NUM_SEL);
				selMessage->AddInt32(MESSAGE_DATA_ID_CPU_NUM, i);
			
				BMenuItem *item = new BMenuItem(buffer, selMessage);
			
				if(i == cpuNum)
					item->SetMarked(true);
			
				cpuSubMenu->AddItem(item);
			}
		
			cpuSubMenu->SetRadioMode(true);
			cpuSubMenu->SetTargetForItems(this);
		
			contextMenu->AddSeparatorItem();
			contextMenu->AddItem(cpuSubMenu);
		}
	}
	
	return contextMenu;
}

void CCPUGraphView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_CPU_NUM_SEL:
			int32 newCpuNum;
			
			if(message->FindInt32(MESSAGE_DATA_ID_CPU_NUM, &newCpuNum) == B_OK) {
				// I assume that there is only one DataInfo object
				// in the list, which contains a CCPUDataProvider.
		
				CDataInfo *dataInfo = dataInfoList.ItemAt(0);
				ICPUDataProvider *dataProvider = NULL;
		
				if(dataInfo)
					dataProvider = dynamic_cast<ICPUDataProvider *>(dataInfo->DataProvider());

				if(dataInfo && dataProvider) {
					// empty history
					dataInfo->Clear();
					dataProvider->SetCPUNum(newCpuNum);
				
					Invalidate();
				}
			}
			break;
		default:
			CGraphView::MessageReceived(message);
	}
}

// ====== CMemGraphView ======

CMemGraphView::CMemGraphView(BRect rect) :
	CGraphView(rect, "Memory Usage Graph", 1)
{
	system_info sysInfo;
	
	get_cached_system_info(&sysInfo, NULL);

	const char *path = "/Total/Memory Usage";

	IDataProvider *dataProvider = global_Namespace->DataProvider(path);

	AddDataProvider(dataProvider, MEM_LINE_COLOR);
	
	SetMaxValue(sysInfo.max_pages);
}

CMemGraphView::CMemGraphView(BMessage *archive) :
	CGraphView(archive)
{
}

BArchivable *CMemGraphView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CMemGraphView"))
		return NULL;
		
	return new CMemGraphView(archive);
}

status_t CMemGraphView::Archive(BMessage *data, bool deep) const
{
	CGraphView::Archive(data, deep);

	// data->AddString("class", "CMemGraphView");
	data->AddString("add_on", APP_SIGNATURE);
	
	return B_OK;
}

// ====== CCPULedView ======

CCPULedView::CCPULedView(BRect rect) :
	CLedView(rect, "CPU Led View", 100, 10, 2, 32, 1)
{
	// maxValue is 100. Value is in PERCENT!

	const char *path = "/Total/CPU Usage/Average";
	IDataProvider *dataProvider = global_Namespace->DataProvider(path);

	SetDataProvider(dataProvider);
}

CCPULedView::CCPULedView(BMessage *archive) :
	CLedView(archive)
{
}

BArchivable *CCPULedView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CCPULedView"))
		return NULL;
		
	return new CCPULedView(archive);
}

status_t CCPULedView::Archive(BMessage *data, bool deep) const
{
	CLedView::Archive(data, deep);
	
	// data->AddString("class", "CCPULedView");
	data->AddString("add_on", APP_SIGNATURE);
	
	return B_OK;
}

// ====== CMemLedView ======

CMemLedView::CMemLedView(BRect rect) :
	CLedView(rect, "Mem Led View", 0, 10, 2, 32, 1)
{
	const char *path = "/Total/Memory Usage";
	IDataProvider *dataProvider = global_Namespace->DataProvider(path);

	SetDataProvider(dataProvider);

	Init();
}

CMemLedView::CMemLedView(BMessage *archive) :
	CLedView(archive)
{
	Init();
}

BArchivable *CMemLedView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CMemLedView"))
		return NULL;
		
	return new CMemLedView(archive);
}

status_t CMemLedView::Archive(BMessage *data, bool deep) const
{
	CLedView::Archive(data, deep);

	// data->AddString("class", "CMemLedView");
	data->AddString("add_on", APP_SIGNATURE);
	
	return B_OK;
}

void CMemLedView::Init()
{
	system_info sysInfo;
	
	get_cached_system_info(&sysInfo, NULL);
	
	maxValue = sysInfo.max_pages;
}

// ====== CUsageView ======

CUsageView::CUsageView(BRect rect) :
	CTabNotifcationView(rect, "UsageView", B_FOLLOW_ALL_SIDES,  B_FRAME_EVENTS),
	distTop(7), distBottom(10), distLeftRight(10), borderSize(1)
{
	// Dummy position for boxes. The correct position/size is set in FrameResized()
	BRect defaultPos(5, 5, 75, 75);

	cpuLedBox   = new CBox(defaultPos, "CPU Led Box");
	cpuLedBox->SetLabel(B_TRANSLATE("CPU Usage"));
	 
	cpuGraphBox = new CBox(defaultPos, "CPU Graph Box");
	cpuGraphBox->SetLabel(B_TRANSLATE("CPU Usage Graph"));
	
	memLedBox	= new CBox(defaultPos, "Mem Led Box");
	memLedBox->SetLabel(B_TRANSLATE("Memory Usage"));
	
	memGraphBox  = new CBox(defaultPos, "Mem Graph Box");
	memGraphBox->SetLabel(B_TRANSLATE("Memory Usage Graph"));

	AddChild(cpuLedBox);
	AddChild(cpuGraphBox);
	AddChild(memLedBox);
	AddChild(memGraphBox);

	// Position of views inside boxes.
	BRect viewRect(distLeftRight, cpuGraphBox->LabelHeight()+distTop, 
					defaultPos.Width()-distLeftRight, 
					defaultPos.Height()-distBottom);

	// Position of borders inside boxes.
	BRect borderRect = viewRect;
	borderRect.InsetBySelf(-borderSize, -borderSize);

	// Position for draggers
	BRect draggerRect(defaultPos.Width()-distLeftRight-2, defaultPos.Height()-distBottom-2, 
					defaultPos.Width()-distLeftRight+5, defaultPos.Height()-distBottom+5);
	
	system_info sysInfo;
	
	get_cached_system_info(&sysInfo, NULL);

	cpuCount = sysInfo.cpu_count;
	
	for(int i=0 ; i<cpuCount ; i++) {
		cpuGraphBorder[i]      = new CBorderView(borderRect, "CPU Graph Border", borderSize, B_FOLLOW_NONE);
		cpuGraphViews[i]       = new CCPUGraphView(viewRect, i);
		cpuGraphViewDragger[i] = new CBugfixedDragger(draggerRect, cpuGraphViews[i], B_FOLLOW_NONE);

		cpuGraphBox->AddChild(cpuGraphBorder[i]);
		cpuGraphBox->AddChild(cpuGraphViewDragger[i]);
		cpuGraphBox->AddChild(cpuGraphViews[i]);
	}

	CCPULedView *cpuLedView = new CCPULedView(viewRect);
	CMemLedView *memLedView = new CMemLedView(viewRect);
	CMemGraphView *memGraphView = new CMemGraphView(viewRect);

	cpuLedBox->AddChild(new CBorderView(borderRect, "CPU Led Border", borderSize, B_FOLLOW_ALL));
	memLedBox->AddChild(new CBorderView(borderRect, "Mem Led Border", borderSize, B_FOLLOW_ALL));
	memGraphBox->AddChild(new CBorderView(borderRect, "Mem Graph Border", borderSize, B_FOLLOW_ALL));

	cpuLedBox->AddChild(new CBugfixedDragger(draggerRect, cpuLedView, B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT));
	memLedBox->AddChild(new CBugfixedDragger(draggerRect, memLedView, B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT));
	memGraphBox->AddChild(new CBugfixedDragger(draggerRect, memGraphView, B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT));

	cpuLedBox->AddChild(cpuLedView);
	memLedBox->AddChild(memLedView);
	memGraphBox->AddChild(memGraphView);
}

void CUsageView::AttachedToWindow()
{
	CTabNotifcationView::AttachedToWindow();

	SetViewColor(CColor::BeBackgroundGray);

	FrameResized(Bounds().Width(), Bounds().Height());
}

void CUsageView::FrameResized(float width, float height)
{
	const int dist = 5;
	const int leftAreaWidth = 150;

	cpuLedBox->MoveTo(dist, dist);
	cpuLedBox->ResizeTo(leftAreaWidth - 2*dist, height/2-2*dist);

	memLedBox->MoveTo(dist, height/2 + dist);
	memLedBox->ResizeTo(leftAreaWidth - 2*dist, height/2-2*dist);
	
	cpuGraphBox->MoveTo(leftAreaWidth + dist, dist);
	cpuGraphBox->ResizeTo(width - leftAreaWidth - 2*dist, height/2-2*dist);

	memGraphBox->MoveTo(leftAreaWidth + dist, height/2 + dist);
	memGraphBox->ResizeTo(width - leftAreaWidth - 2*dist, height/2-2*dist);
	
	float cpuGraphWidth = cpuGraphBox->Bounds().Width() / cpuCount;
	
	for(int i=0 ; i<cpuCount ; i++) {
		float width  = cpuGraphWidth-2*distLeftRight;
		float height = cpuGraphBox->Bounds().Height() - cpuGraphBox->LabelHeight() - distTop - distBottom;

		// Without this +1 the views in the led box and graph box aren't 
		// on the same y position. I don't understand why. Bug??
		BRect viewRect(distLeftRight+cpuGraphWidth*i, distTop+cpuGraphBox->LabelHeight()+1, width, height);
	
		cpuGraphBorder[i]->MoveTo(viewRect.left-borderSize, viewRect.top-borderSize);
		cpuGraphBorder[i]->ResizeTo(width+2*borderSize, height+2*borderSize);

		cpuGraphViews[i]->MoveTo(viewRect.left, viewRect.top);	
		cpuGraphViews[i]->ResizeTo(width, height);

		cpuGraphViewDragger[i]->MoveTo(cpuGraphViews[i]->Frame().RightBottom().x-2,
									   cpuGraphViews[i]->Frame().RightBottom().y-2);
	}
}
