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
#include "common.h"
#include "signature.h"
#include "my_assert.h"
#include "ColorSelectMenuItem.h"
#include "GraphView.h"
#include "Detector.h"
#include "DataProvider.h"

#include "msg_helper.h"

#include <Catalog.h>
#include <Locale.h>
#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "GraphView"

// ====== globals ======

// archive fields of CDataInfo
const char * const DATA_INFO_ARCHIVE_VALUE_COUNT			= "DATAINFO:ValueCount";
const char * const DATA_INFO_ARCHIVE_LINE_COLOR				= "DATAINFO:LineColor";
const char * const DATA_INFO_ARCHIVE_SCALE					= "DATAINFO:Scale";
const char * const DATA_INFO_ARCHIVE_DATA_PROVIDER			= "DATAINFO:DataProvider";

// archive fields of CGraphView
const char * const GRAPH_VIEW_ARCHIVE_VALUE_COUNT			= "GRAPHVIEW:ValueCount";
const char * const GRAPH_VIEW_ARCHIVE_DISTANCE				= "GRAPHVIEW:Distance";
const char * const GRAPH_VIEW_ARCHIVE_MAX_VALUE				= "GRAPHVIEW:MaxValue";
const char * const GRAPH_VIEW_ARCHIVE_GRID_COLOR			= "GRAPHVIEW:GridColor";
const char * const GRAPH_VIEW_ARCHIVE_GRID_SPACE			= "GRAPHVIEW:GridSpace";
const char * const GRAPH_VIEW_ARCHVIE_DATA_INFO_LIST		= "GRAPHVIEW:DataInfoList";
const char * const GRAPH_VIEW_ARCHIVE_AUTO_SCALE			= "GRAPHVIEW:AutoScale";

// archive fields of COverlayGraphView
const char * const OVERLAY_VIEW_ARCHIVE_OVERLAY_INDEX		= "OVERLAY:OverlayIndex";
const char * const OVERLAY_VIEW_ARCHIVE_BUFFERED_DRAWING	= "OVERLAY:BufferedDrawing";

// scripting properties of CGraphView
const char * const GRAPH_VIEW_PROP_DATA_INFO				= "DataInfo";
const char * const GRAPH_VIEW_PROP_AUTO_SCALE				= "AutoScale";
const char * const GRAPH_VIEW_PROP_POINT_DISTANCE			= "PointDistance";
const char * const GRAPH_VIEW_PROP_GRID_SPACE				= "GridSpace";
const char * const GRAPH_VIEW_PROP_GRID_COLOR				= "GridColor";
const char * const GRAPH_VIEW_PROP_MAX_VALUE				= "MaxValue";

// scripting properties of CDataInfo
const char * const DATA_INFO_PROP_COLOR						= "Color";
const char * const DATA_INFO_PROP_SCALE						= "Scale";
const char * const DATA_INFO_PROP_MAX						= "Max";
const char * const DATA_INFO_PROP_AVG						= "Avg";
const char * const DATA_INFO_PROP_CURRENT					= "Current";
const char * const DATA_INFO_PROP_DATA_PROVIDER				= "DataProvider";

// scripting properties of COverlayGraphView
const char * const OVERLAY_VIEW_PROP_OVERLAY_INDEX			= "OverlayIndex";
const char * const OVERLAY_VIEW_PROP_TEXT_COLOR				= "TextColor";
const char * const OVERLAY_VIEW_PROP_TEXT_BG_COLOR			= "TextBackgroundColor";
const char * const OVERLAY_VIEW_PROP_BG_COLOR				= "BackgroundColor";

// message fields of MSG_ADD_DATA_PROVIDER
const char * const MESSAGE_DATA_ID_DATA_PROVIDER			= "ADDDP:DataProvider";

// message field for various DATA_PROVIDER related messages
const char * const MESSAGE_DATA_ID_SCALE	                = "COMMON:Scale";

// colors
const rgb_color DEFAULT_GRAPH_VIEW_BG						= { 0, 0, 0, 255 };
const rgb_color	DEFAULT_GRID_COLOR			                = { 0, 128, 0, 255 };


// ====== CGraphViewUI =====

//: Constructor
//!param: _graphView - The graph view displayed by this UI delegate.
CGraphViewUI::CGraphViewUI(CGraphView *_graphView)
{
	graphView = _graphView;
}

void CGraphViewUI::Draw(BView *view, const BRect &updateRect)
{
	BRect clientRect = view->Bounds();

	int32 valueCount = graphView->ValueCount();
	int32 distance   = graphView->PointDistance();
	int32 maxValue	 = graphView->MaxValue();
	int32 gridSpace	 = graphView->GridSpace();
	int32 gridOffset = graphView->GridOffset();
	
	rgb_color gridColor = graphView->GridColor();

	// startpoint
	int start = (int)MIN(valueCount-1, MAX(0, floor((clientRect.right - updateRect.right) / (float)distance)-1));

	// endpoint (index in array)
	int end   = (int)MIN(valueCount-2, MAX(0, ceil((clientRect.right - updateRect.left) / (float)distance)+2));

	// scale
	float scale = clientRect.Height() / maxValue;

	// Even if updateRect.Width() is zero it includes one pixel!
	// I need to draw at least one line.
	for(int i=start ; i<=end ; i++) {
		float x1 = clientRect.right - distance*i;
		float x2 = clientRect.right - distance*(i+1);

		MY_ASSERT(i >= 0 && i < valueCount);
		MY_ASSERT(i+1 >= 0 && i+1 < valueCount);

		// draw vertical grid lines
		if(((i-gridOffset)%gridSpace) == 0) {
			view->SetHighColor(gridColor);
			view->StrokeLine(BPoint(x2,updateRect.top), BPoint(x2, updateRect.bottom));
		}
		
		for(int k=0 ; k<graphView->CountDataProvider() ; k++) {
			const CDataInfo *dataInfo = graphView->DataProviderAt(k);
		
			float y1 = clientRect.bottom - dataInfo->Value(i) * scale;
			float y2 = clientRect.bottom - dataInfo->Value(i+1) * scale;
			
			// connect two values
			view->SetHighColor(dataInfo->Color());
			view->StrokeLine(BPoint(x1,y1), BPoint(x2,y2));
		}
	}
}

// ====== COverlayGraphViewUI =====

//: Constructor
//!param: graphView - The graph view displayed by this UI delegate.
COverlayGraphViewUI::COverlayGraphViewUI(COverlayGraphView *graphView) :
	CGraphViewUI(graphView)
{
	maxStringWidth = 0.0;
}
	
void COverlayGraphViewUI::Draw(BView *view, const BRect &updateRect)
{
	CGraphViewUI::Draw(view, updateRect);

	COverlayGraphView *olGraphView = dynamic_cast<COverlayGraphView *>(graphView);

	int32 overlayIndex = olGraphView->OverlayIndex();
	
	rgb_color textBackgroundColor = olGraphView->OverlayBackgroundColor();
	rgb_color textColor = olGraphView->OverlayTextColor();

	const CDataInfo *dataInfo = graphView->DataProviderAt(overlayIndex);

	if(dataInfo && dataInfo->DataProvider()) {
		const char *unitString="";
		float mul=1.0;

		switch(dataInfo->DataProvider()->Unit()) {
			case IDataProvider::DP_UNIT_BYTE:
				unitString = B_TRANSLATE("byte");
				break;
			case IDataProvider::DP_UNIT_KILOBYTE:
				unitString = B_TRANSLATE("kB");
				break;
			case IDataProvider::DP_UNIT_MEGABYTE:
				unitString = B_TRANSLATE("MB");
				break;
			case IDataProvider::DP_UNIT_PAGES:
				unitString = B_TRANSLATE("kB");
				mul = 1024/(float)B_PAGE_SIZE;
				break;
			case IDataProvider::DP_UNIT_RPM:
				unitString = B_TRANSLATE("RPM");
				break;
			case IDataProvider::DP_UNIT_DEGREES:
				unitString = B_TRANSLATE("°");
				break;
			case IDataProvider::DP_UNIT_VOLT:
				unitString = B_TRANSLATE("V");
				break;
			case IDataProvider::DP_UNIT_WATT:
				unitString = B_TRANSLATE("W");
				break;
			case IDataProvider::DP_UNIT_AMPERE:
				unitString = B_TRANSLATE("A");
				break;
			case IDataProvider::DP_UNIT_NONE:
				if(dataInfo->DataProvider()->Flags() & IDataProvider::DP_TYPE_PERCENT) {
					unitString = "%";
				} else {
					unitString = "";
				}
				break;				
			default:
				unitString = "";
				break;
		}

		char curString[255], maxString[255], avgString[255];

		sprintf(curString, "%s %.2f %s", B_TRANSLATE("cur:"), dataInfo->Cur()*mul, unitString);
		sprintf(maxString, "%s %.2f %s", B_TRANSLATE("avg:"), dataInfo->Avg()*mul, unitString);
		sprintf(avgString, "%s %.2f %s", B_TRANSLATE("max:"), dataInfo->Max()*mul, unitString);

		maxStringWidth = MAX(maxStringWidth, view->StringWidth(curString));
		maxStringWidth = MAX(maxStringWidth, view->StringWidth(maxString));
		maxStringWidth = MAX(maxStringWidth, view->StringWidth(avgString));

		view->SetDrawingMode(B_OP_ALPHA);
		view->SetHighColor(textBackgroundColor);
	
		font_height fh;
		view->GetFontHeight(&fh);

		float height = fh.ascent + fh.descent + 4.0;
		float width  = (maxStringWidth+7)*3+fh.ascent;
	
		view->FillRect(BRect(0, 0, width, height));
		view->FillArc(BRect(width-height, -height, width+height, height), 270, 90);

		view->SetDrawingMode(B_OP_COPY);	

		view->SetHighColor(textColor);
		view->SetLowColor(textBackgroundColor);
		
		view->DrawString(curString, BPoint(fh.ascent + 7.0, fh.ascent+2.0));
		view->DrawString(maxString, BPoint(fh.ascent + 7.0 + maxStringWidth, fh.ascent+2.0));
		view->DrawString(avgString, BPoint(fh.ascent + (7.0 + maxStringWidth)*2, fh.ascent+2.0));
		
		BPoint colorBoxTopLeft = BPoint(3.0, 3.0);
		BRect colorBox(colorBoxTopLeft, colorBoxTopLeft+BPoint(fh.ascent-1, fh.ascent-1));
		
		view->SetHighColor(graphView->DataProviderAt(overlayIndex)->Color());
		view->FillRect(colorBox);
	}
}

// ====== CBufferedUI ======

//: Constructor
//!param: view - The view to display.
//!param: ui - The real UI delegate doing the drawing.
CBufferedUI::CBufferedUI(BView *view, IUI *_ui)
{
	ui = _ui;

	BRect bounds = view->Bounds();

	buffer = new BBitmap(BRect(0, 0, bounds.Width(), bounds.Height()), 
					BScreen(view->Window()).ColorSpace(), true);
}

//: Destructor
CBufferedUI::~CBufferedUI()
{
	delete buffer;
}

//: Draw the view.
// First draws into a BBitmap and then draw the result into the specified view.
void CBufferedUI::Draw(BView *view, const BRect &updateRect)
{
	// erase background
	rgb_color bgColor;
	
	COverlayGraphView * olGraphView = dynamic_cast<COverlayGraphView*>(view);
	
	if(olGraphView != NULL) {
		bgColor = olGraphView->BackgroundColor();
	} else {
		bgColor = view->ViewColor();
	}
	
	if(buffer->Lock()) {
		if(buffer->CountChildren() < 1) {
			buffer->AddChild(new BView(buffer->Bounds(), "UIView", B_FOLLOW_ALL, B_WILL_DRAW));
		}
	
		MY_ASSERT(buffer->CountChildren() == 1);
		
		BView *offscreenView = buffer->ChildAt(0);
		
		MY_ASSERT(offscreenView);

		BRect bounds = view->Bounds();
		BPoint topLeft = bounds.LeftTop();
		
		offscreenView->ResizeTo(bounds.Width(), bounds.Height());
		offscreenView->SetHighColor(bgColor);
		offscreenView->FillRect(updateRect);
		
		ui->Draw(offscreenView, buffer->Bounds());
		offscreenView->Sync();
		
		BRect drawRect = updateRect;
		
		drawRect.OffsetBy(B_ORIGIN-topLeft);
		
		// display resulting bitmap.
		view->DrawBitmapAsync(buffer, drawRect, updateRect);
		
		buffer->Unlock();
	}
}

void CBufferedUI::FrameResized(BView *view, float width, float height)
{
	if(width <= 0 || height <= 0) {
		// If I create a bitmap with negative width and height the AppServer
		// crashes.
		return;
	}

	if(buffer) {
		float dx = buffer->Bounds().Width() - width;
		float dy = buffer->Bounds().Height() - height;

		if(dx > 5 || dy > 5) {
			delete buffer;
			buffer = new BBitmap(BRect(0, 0, width, height), BScreen(view->Window()).ColorSpace(), true);
		} else if(dx < 0 || dy < 0) {
			delete buffer;
			buffer = new BBitmap(BRect(0, 0, width+5, height+5), BScreen(view->Window()).ColorSpace(), true);
		}
	}
}

// ====== CGraphView ======

//: Constructor
// Creates a new CGraphView.
//!param:	frame		- The frame rectangle
//!param:	title		- The view's title
//!param:	_maxValue	- Maximum value for the samples displayed in the view
//!param:	_distance	- Distance between the sample points in pixels
//!param:	_valueCount - Number of stored samples
//!param	resizeMode	- The resize mode.
//!param	flags		- BView flags. The following flags are automatically added when they aren't set:
//							<CODE>B_WILL_DRAW</CODE>, <CODE>B_PULSE_NEEDED</CODE> and <CODE>B_FULL_UPDATE_ON_RESIZE</CODE>.
CGraphView::CGraphView(BRect frame, const char *title, int32 _maxValue, int32 _distance, int32 _valueCount, int32 resizeMode, int32 flags) : 
	CPulseView(frame, title, B_FOLLOW_ALL_SIDES,  
		flags | B_WILL_DRAW | B_PULSE_NEEDED | B_FULL_UPDATE_ON_RESIZE)
{
	valueCount	= _valueCount;	// number of rembered points
	distance	= _distance;	// distance between points (in pixels)
	maxValue	= _maxValue;	// max value

	// color for grid lines
	gridColor = DEFAULT_GRID_COLOR;

	// distance between two grid lines. (Not in pixels. Multiple of
	// 'distance').
	gridSpace   = 10;

	autoScale = false;

	Init();
}

//: Creates a CGraphView from a archived instance
//!param	archive		- BMessage containing the archived view
CGraphView::CGraphView(BMessage *archive) :
	CPulseView(archive)
{
	valueCount	= archive->FindInt32(GRAPH_VIEW_ARCHIVE_VALUE_COUNT);
	distance	= archive->FindInt32(GRAPH_VIEW_ARCHIVE_DISTANCE);
	maxValue	= archive->FindInt32(GRAPH_VIEW_ARCHIVE_MAX_VALUE);
	gridColor	= FindColor(archive, GRAPH_VIEW_ARCHIVE_GRID_COLOR);

	int32 i=0;
	BMessage dataInfoArchive;

	while(archive->FindMessage(GRAPH_VIEW_ARCHVIE_DATA_INFO_LIST, i++, &dataInfoArchive) == B_OK) {
		CDataInfo *dataInfo = dynamic_cast<CDataInfo *>(instantiate_object(&dataInfoArchive));
		
		if(dataInfo) {
			dataInfo->SetView(this);
			dataInfoList.AddItem(dataInfo);
		}
	}

	if(archive->FindBool(GRAPH_VIEW_ARCHIVE_AUTO_SCALE, &autoScale) != B_OK)
		autoScale = false;

	if(archive->FindInt32(GRAPH_VIEW_ARCHIVE_GRID_SPACE, &gridSpace) != B_OK)
		gridSpace = 10;

	Init();
}

//: Destructor
CGraphView::~CGraphView()
{
	delete notifyMessenger;
	delete notifyMessage;
	delete detector;
	delete ui;
}

//: Instantates an archived CGraphView.
BArchivable *CGraphView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CGraphView"))
		return NULL;
		
	return new CGraphView(archive);
}

//: Archives a CGraphView.
// Archives the CGraphView by recording all its data in a BMessage archive. 
// If the deep flag is true, this function also archives all CDataInfo and sub-views.
// If the flag is false, only the CGraphView is archived.
status_t CGraphView::Archive(BMessage *data, bool deep) const
{
	RETURN_IF_FAILED( CPulseView::Archive(data, deep) );

	data->AddString("add_on", APP_SIGNATURE);

	data->AddInt32(GRAPH_VIEW_ARCHIVE_VALUE_COUNT, valueCount);
	data->AddInt32(GRAPH_VIEW_ARCHIVE_DISTANCE, distance);
	data->AddInt32(GRAPH_VIEW_ARCHIVE_MAX_VALUE, maxValue);
	data->AddInt32(GRAPH_VIEW_ARCHIVE_GRID_SPACE, gridSpace);
	data->AddBool(GRAPH_VIEW_ARCHIVE_AUTO_SCALE, autoScale);
		
	AddColor(data, GRAPH_VIEW_ARCHIVE_GRID_COLOR, gridColor);
		
	if(deep) {
		for(int i=0 ; i<dataInfoList.CountItems() ; i++) {
			BMessage dataInfoArchive;
		
			if( dataInfoList.ItemAt(i)->Archive(&dataInfoArchive, deep) == B_OK ) {
				data->AddMessage(GRAPH_VIEW_ARCHVIE_DATA_INFO_LIST, &dataInfoArchive);
			}
		}
	}
	
	return B_OK;
}

//: Adds a data provider to the view.
// If the passed data provider is already part of the list of displayed data providers it isn't added
// again. Only the 'scale' factor and the 'color' are changed and 'updated' is set to true.
// Returns at which index the data provider was added to the list.
//!param:	provider	- The data provider to add.
//!param:	color		- The color used to display the data provider
//!param:	scale		- The scale factor applied to the data provider's samples before they are displayed.
//!param:	updated		- Set to true if the passed data provider was already displayed by this view
int32 CGraphView::AddDataProvider(IDataProvider *provider, rgb_color color, float scale, bool *updated)
{
	if(updated)
		*updated = false;

	if(provider == NULL)
		return -1;

	// check for duplicates
	for(int i=0 ; i<dataInfoList.CountItems() ; i++) {
		if(dataInfoList.ItemAt(i)->DataProvider() &&
		   dataInfoList.ItemAt(i)->DataProvider()->Equal(provider)) {
			// Free provider. If the provider is added
			// that is done in the destructor of CDataInfo.
			delete provider;
			
			// change color and scale
			dataInfoList.ItemAt(i)->SetColor(color);
			dataInfoList.ItemAt(i)->SetScale(scale);

			if(updated)
				*updated = true;
			
			return i;
		}
	}

	CDataInfo *dataInfo = new CDataInfo(this, valueCount, provider, color, scale);

	dataInfoList.AddItem(dataInfo);

	if(Window()) {
		// View already attached to window. Add dataInfo as handler.
		Window()->AddHandler(dataInfo);
	}
	
	return dataInfoList.CountItems()-1;
}

//: Get the data provider at the passed index.
//!param:	index	- The index
// Returns the data provider at the specified index or NULL if the index is invalid.
const CDataInfo *CGraphView::DataProviderAt(int32 index) const
{
	return dataInfoList.ItemAt(index);
}

//: Get the number of registered data providers.
int32 CGraphView::CountDataProvider() const
{
	return dataInfoList.CountItems();
}

//: Initializes the member variables.
void CGraphView::Init()
{
	gridOffset  = 0;
	
	notifyMessenger = NULL;
	notifyMessage	= NULL;
	
	ui = NULL;
	
	// This object will send a MSG_CONTEXT_MENU message
	// to this view when it detects that the user wants
	// to open the context menu.
	detector = new CContextMenuDetector(this);
}

//: BeOS hook function
void CGraphView::AttachedToWindow()
{
	CPulseView::AttachedToWindow();
	SetViewColor(DEFAULT_GRAPH_VIEW_BG);
	
	// Add all CDataInfo objects to BLooper.
	for(int i=0 ; i<dataInfoList.CountItems() ; i++) {
		Window()->AddHandler(dataInfoList.ItemAt(i));
	}
	
	ui = CreateUI();
}

//: BeOS hook function
// Forwards the mouse message to the attached CContextMenuDetector.
void CGraphView::MouseDown(BPoint point)
{
	detector->MouseDown(point);
}

//: BeOS hook function
// Forwards the mouse message to the attached CContextMenuDetector.
void CGraphView::MouseUp(BPoint point)
{
	detector->MouseUp(point);
}

//: Sets a notification message and handler.
// A notification message is sent to the handler, when:
// <UL>
// <LI>A data info object changed.
// <LI>A data provider was added.
// <LI>A data provider was removed.
// </UL>
// !param:	handler - Handler receiving the notification.
// !param:  message - Template message.
void CGraphView::SetNotification(BHandler *handler, BMessage *message)
{
	delete notifyMessenger;
	delete notifyMessage;
	
	notifyMessenger = new BMessenger(handler);
	notifyMessage	= message;
}

//: Creates the context menu for this view.
// Called when a MSG_CONTEXT_MENU message is received.
// Returns the context menu.
BPopUpMenu *CGraphView::ContextMenu()
{
	BPopUpMenu *contextMenu = new BPopUpMenu("Context Menu", false);

	const char *updateSpeedMenuLabel   = B_TRANSLATE("Update Speed");
	const char *updateSpeedSlowLabel   = B_TRANSLATE("Slow");
	const char *updateSpeedNormalLabel = B_TRANSLATE("Normal");
	const char *updateSpeedFastLabel   = B_TRANSLATE("Fast");

	BMenu *updateSpeedSubMenu = new BMenu(updateSpeedMenuLabel);
	
	updateSpeedSubMenu->AddItem(new BMenuItem(updateSpeedSlowLabel, new BMessage(MSG_SLOW_UPDATE)));
	updateSpeedSubMenu->AddItem(new BMenuItem(updateSpeedNormalLabel, new BMessage(MSG_NORMAL_UPDATE)));
	updateSpeedSubMenu->AddItem(new BMenuItem(updateSpeedFastLabel, new BMessage(MSG_FAST_UPDATE)));

	updateSpeedSubMenu->SetRadioMode(true);
	updateSpeedSubMenu->SetTargetForItems(this);

	if(ReplicantPulseRate() == SLOW_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(0)->SetMarked(true);
	} else if(ReplicantPulseRate() == NORMAL_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(1)->SetMarked(true);
	} else if(ReplicantPulseRate() == FAST_PULSE_RATE) {
		updateSpeedSubMenu->ItemAt(2)->SetMarked(true);
	} else {
		// User defined update speed.
	}
	
	contextMenu->AddItem(updateSpeedSubMenu);
	
	return contextMenu;
}

//: BeOS hook function
// Gets a new sample from all data providers and invalidates the view.
void CGraphView::Pulse()
{
	float currentMaxValue = 0.0;

	for(int32 i=0 ; i<dataInfoList.CountItems() ; i++) {
		CDataInfo *dataInfo = dataInfoList.ItemAt(i);
	
		bool update = dataInfo->Update();
	
		if(update)
			currentMaxValue = MAX(dataInfo->Max()*dataInfo->Scale(), currentMaxValue);
	}

	if(autoScale && currentMaxValue > maxValue) {
		while(currentMaxValue > maxValue)
			maxValue *= 2;
			
		Invalidate();
	} else  if(autoScale && currentMaxValue != 0.0 && currentMaxValue*10 < maxValue) {
		while(currentMaxValue*10 < maxValue && maxValue >= 1)
			maxValue /= 2;

		if(maxValue <= 0) {
			maxValue = 1;
		}
		
		Invalidate();
	} else {
		if (!IsHidden()) {
			CopyContent();
		}
	}
		
	gridOffset++;

	if(gridOffset >= gridSpace) {
		gridOffset = 0;
	}		
}

//: UI delegate factory method.
IUI *CGraphView::CreateUI()
{
	return new CGraphViewUI(this);
}

//: Copies the content of the view using CopyBits.
// Copies the view's context by 'distance' to the left and invalidates the revealed area.
void CGraphView::CopyContent()
{
	BRect source, dest, other;
			
	source = dest = other = Bounds();
				
	source.left += distance;
	dest.right  -= distance;
	other.left  = other.right - distance;

	CopyBits(source, dest);
			
	Invalidate(other);
}

//: BeOS hook function.
void CGraphView::Draw(BRect updateRect)
{
	if(Bounds().Width() <= 0 || Bounds().Height() <= 0)
		return;

	BAutolock autoLocker(Window());

	if(ui)
		ui->Draw(this, updateRect);
} 

//: Sends a MSG_NOTIFY_DATA_INFO_CHANGED notification to the registered notification handler.
void CGraphView::SendNotify_DataInfoChanged(int32 dataInfoIndex)
{
	if(notifyMessenger) {
		BMessage notify;
							
		if(notifyMessage) {
			// If the user supplyed a message template --> copy it.
			notify = *notifyMessage;
		}
						
		CDataInfo *dataInfo = dataInfoList.ItemAt(dataInfoIndex);

		notify.what = MSG_NOTIFY_DATA_INFO_CHANGED;

		notify.AddInt32(MESSAGE_DATA_ID_INDEX, dataInfoIndex);
		notify.AddFloat(MESSAGE_DATA_ID_SCALE, dataInfo->Scale());
		AddColor(&notify, MESSAGE_DATA_ID_COLOR, dataInfo->Color());

		notifyMessenger->SendMessage(&notify);
	}
}

//: Sends a MSG_NOTIFY_DATA_PROVIDER_ADDED notification to the registered notification handler.
void CGraphView::SendNotify_DataProviderAdded(int32 dataInfoIndex)
{
	if(notifyMessenger) {
		BMessage notify;
							
		if(notifyMessage) {
			// If the user supplyed a message template --> copy it.
			notify = *notifyMessage;
		}

		CDataInfo *dataInfo = dataInfoList.ItemAt(dataInfoIndex);
						
		notify.what = MSG_NOTIFY_DATA_PROVIDER_ADDED;

		notify.AddInt32(MESSAGE_DATA_ID_INDEX, dataInfoIndex);
		notify.AddFloat(MESSAGE_DATA_ID_SCALE, dataInfo->Scale());
		notify.AddPointer(MESSAGE_DATA_ID_DATA_PROVIDER, dataInfo->DataProvider());
		AddColor(&notify, MESSAGE_DATA_ID_COLOR, dataInfo->Color());

		notifyMessenger->SendMessage(&notify);
	}
}

//: Sends a MSG_NOTIFY_DATA_INFO_DELETED notification to the registered notification handler.
void CGraphView::SendNotify_DataInfoDeleted(int32 dataInfoIndex)
{
	if(notifyMessenger) {
		BMessage notify;
							
		if(notifyMessage) {
			// If the user supplyed a message template --> copy it.
			notify = *notifyMessage;
		}

		notify.what = MSG_NOTIFY_DATA_INFO_DELETED;

		notify.AddInt32(MESSAGE_DATA_ID_INDEX, dataInfoIndex);

		notifyMessenger->SendMessage(&notify);
	}
}

//! Determines the next index of a DataProvider object from a script message.
// Determines the index of a data provider from a script message and stores it in
// 'dataInfoIndex'. If the script message specifies multiple objects you can iterate
// over them by calling this method multiple times until it returns false. The
// state is stored in the passed 'cookie'. To start the iteration set 'cookie' to 0.
// Returns false it the end of the list is reached.
//!param:	specifier		- The script specifier
//!param:	what			- The specifier's 'what' field
//!param:	dataInfoIndex	- On return the data info index
//!param:	cookie			- A cookie allowing to iterate over multiple indices.
//!param:	result			- On return the result of the operation
bool CGraphView::GetNextDataInfoIndex(BMessage *specifier, int32 what, 
	int32 /*[out]*/ &dataInfoIndex, int32 /*[in, out]*/ &cookie, status_t /*[out]*/ &result)
{
	result=B_ERROR;

	switch(what)
	{
		case B_INDEX_SPECIFIER:
			result = specifier->FindInt32("index", &dataInfoIndex);

			if(cookie++ == 0)
				return true;
				
			break;
		case B_REVERSE_INDEX_SPECIFIER:
			result = specifier->FindInt32("index", &dataInfoIndex);

			dataInfoIndex = dataInfoList.CountItems()-dataInfoIndex-1;

			if(cookie++ == 0)
				return true;

			break;
		case B_RANGE_SPECIFIER:
			if((result = specifier->FindInt32("index", &dataInfoIndex)) == B_OK) {
			
				int32 range;

				if((result = specifier->FindInt32("range", &range)) == B_OK) {
					if(cookie < range) {
						dataInfoIndex += cookie;
					
						cookie++;
					
						return true;
					}
				}
			}
			break;
		case B_REVERSE_RANGE_SPECIFIER:
			if((result = specifier->FindInt32("index", &dataInfoIndex)) == B_OK) {
			
				dataInfoIndex = dataInfoList.CountItems()-dataInfoIndex-1;
			
				int32 range;
			
				if((result = specifier->FindInt32("range", &range)) == B_OK) {
					if(cookie < range) {
						dataInfoIndex -= cookie;
					
						cookie++;
					
						return true;
					}
				}
			}
			break;
		default:
			result = B_BAD_VALUE;
	}

	return false;
}

//! BeOS hook function
status_t CGraphView::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)GRAPH_VIEW_PROP_AUTO_SCALE,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)GRAPH_VIEW_PROP_POINT_DISTANCE,	// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 3rd property
			(char *)GRAPH_VIEW_PROP_GRID_SPACE,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 4th property
			(char *)GRAPH_VIEW_PROP_GRID_COLOR,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 5th property
			(char *)GRAPH_VIEW_PROP_MAX_VALUE,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 6th property
			(char *)GRAPH_VIEW_PROP_DATA_INFO,		// name
			{ B_COUNT_PROPERTIES, 0 },				// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 7th property
			(char *)GRAPH_VIEW_PROP_DATA_INFO,		// name
			{ B_DELETE_PROPERTY, 0 },				// commands
			{ 										// specifiers
				B_INDEX_SPECIFIER, 
				B_REVERSE_INDEX_SPECIFIER, 
				0 
			},				
			"",										// usage
			0										// extra_data
		},
		{ 										// 8th property
			(char *)GRAPH_VIEW_PROP_DATA_INFO,		// name
			{ 0 },									// commands
			{ 										// specifiers
				B_INDEX_SPECIFIER, 
				B_REVERSE_INDEX_SPECIFIER,
				B_RANGE_SPECIFIER,
				B_REVERSE_RANGE_SPECIFIER,
				0 
			},				
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-graph-view");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return CPulseView::GetSupportedSuites(message);
}

//! BeOS hook function.
BHandler *CGraphView::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_COUNT_PROPERTIES:
			if(what == B_DIRECT_SPECIFIER && strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0) {
				return this;
			}
			break;
		case B_DELETE_PROPERTY:
			if(what == B_INDEX_SPECIFIER || what == B_REVERSE_INDEX_SPECIFIER) {
				if( strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0 ) {
					return this;
				}
			}
			break;
		case B_SET_PROPERTY:
			// FALL THROUGH
		case B_GET_PROPERTY:
			if( strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0 ) {
				switch(what) {
					case B_INDEX_SPECIFIER:
						// FALL THROUGH
					case B_REVERSE_INDEX_SPECIFIER:
						{
							int32 dataInfoIndex;
							int32 cookie = 0;
							status_t result;
							CDataInfo *dataInfo;
						
							// This is a index specifier. GetNextDataInfoIndex only returns one
							// index.
							GetNextDataInfoIndex(specifier, what, dataInfoIndex, cookie, result);
								
							if(result == B_OK) {
								dataInfo = dataInfoList.ItemAt(dataInfoIndex);
								
								if(dataInfo) {
									message->PopSpecifier();
									return dataInfo;
								}

								result = B_BAD_INDEX;
							}
							
							// send error message
							BMessage reply(B_REPLY);
							send_script_reply(reply, result, message);

							// don't handle this message.
							return NULL;
						}
						break;
					case B_RANGE_SPECIFIER:
						// FALL THROUGH
					case B_REVERSE_RANGE_SPECIFIER:
						// If the user requests a range of properties,
						// the CDataInfo class can't handle the request directly.
						// This object has to iterate over all objects in the
						// range and return the data in one message.
						return this;
				}
			}
			
			if(what == B_DIRECT_SPECIFIER) {
				if( strcmp(property, GRAPH_VIEW_PROP_AUTO_SCALE) == 0 ||
					strcmp(property, GRAPH_VIEW_PROP_POINT_DISTANCE) == 0 ||
					strcmp(property, GRAPH_VIEW_PROP_GRID_SPACE) == 0 ||
					strcmp(property, GRAPH_VIEW_PROP_GRID_COLOR) == 0 ||
					strcmp(property, GRAPH_VIEW_PROP_MAX_VALUE) == 0 ) {
					return this;
				}
			}

			break;
	}

	return CPulseView::ResolveSpecifier(message, index, specifier, what, property);
}

//! BeOS hook function.
void CGraphView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case B_ABOUT_REQUESTED:
			{
				show_alert_async( "TaskManager Graph View (Replicant)\n\n"
								  B_UTF8_COPYRIGHT" 1999-2002 by Thomas Krammer",
								  this, "About GraphView" );
			}
			break;
		case MSG_ADD_DATA_PROVIDER:
			{
				IDataProvider *dataProvider;
			
				if(msg->FindPointer(MESSAGE_DATA_ID_DATA_PROVIDER, (void **)&dataProvider) == B_OK) {
					rgb_color color;
					float scale;
				
					if(FindColor(msg, MESSAGE_DATA_ID_COLOR, color) == B_OK && 
					   msg->FindFloat(MESSAGE_DATA_ID_SCALE, &scale) == B_OK) {
						// The passed data provider is only valid as long as the source
						// exists. Create a clone of the object.
						dataProvider = dataProvider->Clone();
						
						bool updated=false;
						
						int32 index = AddDataProvider(dataProvider, color, scale, &updated);

						if(updated) {
							SendNotify_DataInfoChanged(index);
						} else {
							SendNotify_DataProviderAdded(index);
						}						
					}
					
					Invalidate();
				}
			}
			break;
		case B_COUNT_PROPERTIES:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0) {
						// COUNT_PROPERTIES for 'DataInfo' property.
						
						msg->PopSpecifier();

						BMessage reply(B_REPLY);
						reply.AddInt32("result", dataInfoList.CountItems());

						send_script_reply(reply, B_OK, msg);
					} else {
						CPulseView::MessageReceived(msg);
					}
				}
			}
			break;
		case B_GET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					BMessage reply(B_REPLY);
					status_t result;
				
					if(strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0) {
						// GET_PROPERTY for 'DataInfo' property.

						msg->PopSpecifier();
						
						int32 dataInfoIndex;
						int32 cookie = 0;

						while(GetNextDataInfoIndex(&specifier, what, dataInfoIndex, cookie, result)) {
							CDataInfo *dataInfo = dataInfoList.ItemAt(dataInfoIndex);
							
							if(dataInfo == NULL) {
								result = B_BAD_INDEX;
								break;
							} else {
								int32 index;
								BMessage specifier;
								int32 what;
								const char *property;
							
								if((result = msg->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
									result = dataInfo->GetProperty(property, what, reply);

									if(result == E_NOT_HANDLED) result = B_BAD_VALUE;
								}
							}
							
							if(result != B_OK) {
								// don't process other items if error occured.
								break;
							}
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_AUTO_SCALE) == 0) {
						// GET_PROPERTY for 'AutoScale' property.
						result = reply.AddBool("result", autoScale);
					} else if(strcmp(property, GRAPH_VIEW_PROP_POINT_DISTANCE) == 0) {
						// GET_PROPERTY for 'PointDistance' property.
						result = reply.AddInt32("result", distance);
					} else if(strcmp(property, GRAPH_VIEW_PROP_GRID_SPACE) == 0) {
						// GET_PROPERTY for 'GridSpace' property.
						result = reply.AddInt32("result", gridSpace);
					} else if(strcmp(property, GRAPH_VIEW_PROP_GRID_COLOR) == 0) {
						// GET_PROPERTY for 'GridColor' property.
						result = AddColor(&reply, "result", gridColor);
					} else if(strcmp(property, GRAPH_VIEW_PROP_MAX_VALUE) == 0) {
						// GET_PROPERTY for 'MaxValue' property.
						result = reply.AddFloat("result", maxValue);
					} else {
						CPulseView::MessageReceived(msg);
						return;
					}
					
					msg->PopSpecifier();										
						
					send_script_reply(reply, result, msg);
				}
			}
			break;
		case B_SET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					BMessage reply(B_REPLY);
					status_t result;
				
					if(strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0) {
						// SET_PROPERTY for 'DataInfo' property.

						msg->PopSpecifier();
						
						int32 dataInfoIndex;
						int32 cookie = 0;
						
						while(GetNextDataInfoIndex(&specifier, what, dataInfoIndex, cookie, result)) {
							CDataInfo *dataInfo = dataInfoList.ItemAt(dataInfoIndex);
							
							if(dataInfo == NULL) {
								result = B_BAD_INDEX;
								break;
							} else {
								int32 index;
								BMessage specifier;
								int32 what;
								const char *property;
							
								if((result = msg->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
									result = dataInfo->SetProperty(property, what, msg);
									
									if(result == E_NOT_HANDLED) result = B_BAD_VALUE;
								}
								
							}
							
							if(result != B_OK) {
								// don't process other items if error occured.
								break;
							} else {
								// Change was successful. Send notification
								SendNotify_DataInfoChanged(dataInfoIndex);
							}
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_AUTO_SCALE) == 0) {
						bool newValue;
						
						if((result = msg->FindBool("data", &newValue)) == B_OK) {
							autoScale = newValue;
							Invalidate();
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_POINT_DISTANCE) == 0) {
						int32 newValue;
						
						if((result = msg->FindInt32("data", &newValue)) == B_OK) {
							if(newValue <= 0) {
								// negative values and zero are invalid.
								result = B_BAD_VALUE;
							} else {
								distance = newValue;
								Invalidate();
							}
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_GRID_SPACE) == 0) {
						int32 newValue;
						
						if((result = msg->FindInt32("data", &newValue)) == B_OK) {
							if(newValue <= 0) {
								// negative values and zero are invalid.
								result = B_BAD_VALUE;
							} else {
								gridSpace = newValue;
								Invalidate();
							}
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_GRID_COLOR) == 0) {
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							gridColor = newValue;
							Invalidate();
						}
					} else if(strcmp(property, GRAPH_VIEW_PROP_MAX_VALUE) == 0) {
						float newValue;
						
						if((result = msg->FindFloat("data", &newValue)) == B_OK) {
							int32 oldValue = maxValue;
							
							maxValue = (int32)(newValue + 0.5);
							
							if(maxValue == 0) {
								// A maxValue of zero causes a division by
								// zero. The point coordinates get infinite and
								// the app server crashes....
								maxValue = oldValue;
								result = B_BAD_VALUE;
							} else {							
								Invalidate();
							}
						}
					} else {
						CPulseView::MessageReceived(msg);
						
						return;
					}
					
					msg->PopSpecifier();										
						
					send_script_reply(reply, result, msg);
				}
			}
			break;
		case B_DELETE_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, GRAPH_VIEW_PROP_DATA_INFO) == 0) {
						// DELETE_PROPERTY for 'DataInfo' property.

						int32 dataInfoIndex;
						int32 cookie = 0;
						BMessage reply(B_REPLY);
						status_t result;
						
						// Only use first item of RANGE_SPECIFIER.
						if(GetNextDataInfoIndex(&specifier, what, dataInfoIndex, cookie, result)) {
							CDataInfo *dataInfo = dataInfoList.RemoveItem(dataInfoIndex);
							
							if(dataInfo) {
								delete dataInfo;
								Invalidate();
								
								SendNotify_DataInfoDeleted(dataInfoIndex);
							} else {
								result = B_BAD_INDEX;
							}
						}

						msg->PopSpecifier();
						
						send_script_reply(reply, result, msg);
					} else {
						CPulseView::MessageReceived(msg);
					}
				}
			}
			break;
		case MSG_CONTEXT_MENU:
			if(IsReplicant()) {
				BPoint point = msg->FindPoint("where");
			
				// display context menu.
				BPopUpMenu *contextMenu = ContextMenu();
				
				ConvertToScreen(&point);
				
				// allow the user to move the mouse by 3 points in every direction
				// without closing the menu, when the mouse is released.
				BRect clickToOpen(point.x-3, point.y-3, point.x+3, point.y+3);
		
				contextMenu->SetAsyncAutoDestruct(true);
					
				// open menu (asynchonous)
				contextMenu->Go(point, true, true, clickToOpen, true);
			}
			break;
		default:
			CPulseView::MessageReceived(msg);
	}
}

// ==== CDataInfo ====

//! Constructor
//!param	_view			-	The view displaying this item
//!param	_valueCount		-	The number of stores samples
//!param	provider		-	The data provider used to get the samples
//!param	_color			-	The color used to display this item
//!param	_scale			-	The scale factor applied the samples before they are displayed
CDataInfo::CDataInfo(
	CPulseView *_view, 
	int32 _valueCount, 
	IDataProvider *provider,
	rgb_color _color,
	float _scale) :
	BHandler(provider ? provider->DisplayName().String() : "")
{
	view	     = _view;
	valueCount	 = _valueCount;
	color		 = _color;
	scale		 = _scale;
	dataProvider = provider;

	Init();
	Clear();
}

CDataInfo::CDataInfo(BMessage *archive) : 
	BHandler(archive)
{
	view	   = NULL;
	valueCount = archive->FindInt32(DATA_INFO_ARCHIVE_VALUE_COUNT);
	scale	   = archive->FindFloat(DATA_INFO_ARCHIVE_SCALE);
	color      = FindColor(archive, DATA_INFO_ARCHIVE_LINE_COLOR);
	
	BMessage dataProviderArchive;

	if(archive->FindMessage(DATA_INFO_ARCHIVE_DATA_PROVIDER, &dataProviderArchive) == B_OK) {
		dataProvider = dynamic_cast<IDataProvider *>(instantiate_object(&dataProviderArchive));
		
		if(dataProvider == NULL) {
			BString message;

			const char *className	= dataProviderArchive.FindString("class");
			const char *add_on		= dataProviderArchive.FindString("add_on");
			
			message << "Instantiation of class '" << className << "' failed.\n"
					<< "Make sure that the add_on '" << add_on
					<< "' is available and correctly registered in the mime database.";
			
			// TODO: add help
			show_alert(message);
		}
	} else {
		dataProvider = NULL;
	}

	Init();
	Clear();
}

//! Initializes the members of this object.
void CDataInfo::Init()
{
	valueArray = new float[valueCount];
	
	insertPoint   = 0;
	avgValueCount = 0;

	avg = max = 0.0;
}

//! Destructor
CDataInfo::~CDataInfo()
{
	delete valueArray;
	delete dataProvider;
}

//: Archives this object.
// Archives the CDataInfo by recording all its data in a BMessage archive. 
// If the deep flag is true, this function also archives the IDataProvider.
// If the flag is false, only the CDataInfo object is archived.
status_t CDataInfo::Archive(BMessage *archive, bool deep = true) const
{
	RETURN_IF_FAILED( BHandler::Archive(archive, deep) );

	archive->AddString("add_on", APP_SIGNATURE);
	archive->AddInt32(DATA_INFO_ARCHIVE_VALUE_COUNT, valueCount);
	archive->AddFloat(DATA_INFO_ARCHIVE_SCALE, scale);
	archive->AddData(DATA_INFO_ARCHIVE_LINE_COLOR, B_RGB_COLOR_TYPE, &color, sizeof(rgb_color));
	
	if(deep) {
		BMessage providerArchive;
		
		BArchivable *archivable = dynamic_cast<BArchivable *>(dataProvider);
		
		if(archivable && archivable->Archive(&providerArchive, deep) == B_OK)
			archive->AddMessage(DATA_INFO_ARCHIVE_DATA_PROVIDER, &providerArchive);
	}
	
	return B_OK;
}

//: Set the data provider.
// Deletes the old object before replacing it.
void CDataInfo::SetDataProvider(IDataProvider *provider)
{
	delete dataProvider; 
	dataProvider = provider;
}

//! BeOS hook function.
status_t CDataInfo::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)DATA_INFO_PROP_COLOR,			// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)DATA_INFO_PROP_SCALE,			// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 3rd property
			(char *)DATA_INFO_PROP_MAX,				// name
			{ B_GET_PROPERTY, 0 },					// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 4th property
			(char *)DATA_INFO_PROP_AVG,				// name
			{ B_GET_PROPERTY, 0 },					// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 5th property
			(char *)DATA_INFO_PROP_CURRENT,			// name
			{ B_GET_PROPERTY, 0 },					// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 6th property
			(char *)DATA_INFO_PROP_DATA_PROVIDER,	// name
			{ B_GET_PROPERTY, 0 },					// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-data-info");
	BPropertyInfo prop_info(prop_list);
	
	message->AddFlat("messages", &prop_info);
	
	return BHandler::GetSupportedSuites(message);
}

//! BeOS hook function.
BHandler *CDataInfo::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_SET_PROPERTY:
			if(what == B_DIRECT_SPECIFIER) {
				if( strcmp(property, DATA_INFO_PROP_COLOR) == 0 ||
					strcmp(property, DATA_INFO_PROP_SCALE) == 0) {
					return this;
				}
			}
			break;
		case B_GET_PROPERTY:
			if(what == B_DIRECT_SPECIFIER) {
				if( strcmp(property, DATA_INFO_PROP_COLOR) == 0 ||
					strcmp(property, DATA_INFO_PROP_SCALE) == 0 ||
					strcmp(property, DATA_INFO_PROP_DATA_PROVIDER) == 0 ||
					strcmp(property, DATA_INFO_PROP_MAX) == 0 ||
					strcmp(property, DATA_INFO_PROP_AVG) == 0 ||
					strcmp(property, DATA_INFO_PROP_CURRENT) == 0) {
					return this;
				}
			}
			break;
	}
	
	return BHandler::ResolveSpecifier(message, index, specifier, what, property);
}

status_t CDataInfo::SetProperty(const char *property, int32 what, BMessage *msg)
{
	status_t result;

	if(strcmp(property, DATA_INFO_PROP_COLOR) == 0 && what == B_DIRECT_SPECIFIER) {
		// SET_PROPERTY for 'Color' property.
		rgb_color color;
		
		if((result = FindColor(msg, "data", color)) == B_OK) {
			SetColor(color);
			if(view) { view->Invalidate(); }
		}
		
	} else if(strcmp(property, DATA_INFO_PROP_SCALE) == 0 && what == B_DIRECT_SPECIFIER) {
		// SET_PROPERTY for 'Scale' property.
		float scale;
		
		if((result = msg->FindFloat("data", &scale)) == B_OK) {
			SetScale(scale);
			if(view) { view->Invalidate(); }
		}
	} else {
		result = E_NOT_HANDLED;
	}
	
	return result;
}

status_t CDataInfo::GetProperty(const char *property, int32 what, BMessage &reply)
{
	status_t result;
	
	if(strcmp(property, DATA_INFO_PROP_COLOR) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'Color' property.
		result = AddColor(&reply, "result", Color());
	} else if(strcmp(property, DATA_INFO_PROP_SCALE) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'Scale' property.
		result = reply.AddFloat("result", Scale());
	} else if(strcmp(property, DATA_INFO_PROP_DATA_PROVIDER) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'DataProvider' property.
		result = reply.AddPointer("result", DataProvider());
	} else if(strcmp(property, DATA_INFO_PROP_MAX) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'Max' property.
		result = reply.AddFloat("result", Max());
	} else if(strcmp(property, DATA_INFO_PROP_AVG) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'Avg' property.
		result = reply.AddFloat("result", Avg());
	} else if(strcmp(property, DATA_INFO_PROP_CURRENT) == 0 && what == B_DIRECT_SPECIFIER) {
		// GET_PROPERTY for 'Current' property.
		result = reply.AddFloat("result", Cur());
	} else {
		result = E_NOT_HANDLED;
	}

	return result;	
}

//! BeOS hook function.
void CDataInfo::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case B_GET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;

				status_t result;
				
				BMessage reply(B_REPLY);
				
				if((result = message->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
					result = GetProperty(property, what, reply);
				}
				
				if(result != E_NOT_HANDLED) {
					// the script property was handled.
					send_script_reply(reply, result, message);
					return;
				} else {
					// let BHandler handle the message (don't return)
				}
			}
			break;
		case B_SET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				status_t result;
				
				if((result = message->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
					result = SetProperty(property, what, message);
				}
				
				if(result != E_NOT_HANDLED) {
					// the script property was handled.
					BMessage reply(B_REPLY);
					send_script_reply(reply, result, message);
					return;
				}
			}
			break;
	}

	BHandler::MessageReceived(message);
}

BArchivable *CDataInfo::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CDataInfo"))
		return NULL;
		
	return new CDataInfo(archive);
}

//: Get a sample from the sample buffer.
//!param: index - index in the sample buffer.
float CDataInfo::Value(int32 index) const
{
	// array is used as ring buffer
	return valueArray[(insertPoint+index+1)%valueCount] * scale;
}

//: Clear the sample buffer.
void CDataInfo::Clear()
{
	memset(valueArray, 0, sizeof(float)*valueCount);
}

//: Add a new entry to the sample buffer.
bool CDataInfo::Update()
{
	float value=0.0;

	if(dataProvider && dataProvider->GetNextValue(value)) {
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_RELATIVE)
			value /= view->ReplicantPulseRate();
			
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_PERCENT) {
			value *= 100.0;
			
			value = MIN(MAX(value, 0.0), 100.0);
		}
		
		max = MAX(value, max);
		
		if(avgValueCount == 0) {
			avg = value;
			avgValueCount++;
		} else  {
			avg = (avg*avgValueCount + value) / ++avgValueCount;
		}
	}

	// add new value to array
	valueArray[insertPoint--] = value;
		
	if(insertPoint < 0) {
		// array is used as ring buffer
		insertPoint = valueCount-1;
	}
		
	return true;
}

// ==== COverlayGraphView ====

//! Constructor
// In the overlay graph view you can use buffered drawing. If buffered drawing is enabled the view first
// paints it's content into a BBitmap and then on the screen. Otherwise it directly draws on the screen.
// Buffered drawing has the advantage that it reduces flickering - especially when an overlay is
// displayed.
//!param:	frame			-	The frame rectangle
//!param:	title			-	The view's title
//!param:	bufferedDraw	- 	Enables or disables buffered drawing.
//!param:	maxValue		-	The maximum value for the samples displayed by this view.
//!param:	distance		-	The distance between the sample points in pixel.
//!param:	valueCount		-	The number of stored samples per data provider.
COverlayGraphView::COverlayGraphView(BRect frame, const char *title, bool bufferedDraw, int32 maxValue, 
	int32 distance, int32 valueCount) :
	CGraphView(frame, title, maxValue, distance, valueCount)
{
	overlayIndex = -1;

	bufferedDrawing = bufferedDraw;

	Init();
}

COverlayGraphView::COverlayGraphView(BMessage *archive) :
	CGraphView(archive)
{
	overlayIndex 	= archive->FindInt32(OVERLAY_VIEW_ARCHIVE_OVERLAY_INDEX);
	bufferedDrawing	= archive->FindBool(OVERLAY_VIEW_ARCHIVE_BUFFERED_DRAWING);

	Init();
}

//! Destructor
COverlayGraphView::~COverlayGraphView()
{
}

//! BeOS hook function.
void COverlayGraphView::FrameResized(float width, float height)
{
	if(ui)
		ui->FrameResized(this, width, height);
}

BArchivable *COverlayGraphView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "COverlayGraphView"))
		return NULL;
		
	return new COverlayGraphView(archive);
}

status_t COverlayGraphView::Archive(BMessage *data, bool deep) const
{
	RETURN_IF_FAILED(CGraphView::Archive(data, deep));
	
	data->AddInt32(OVERLAY_VIEW_ARCHIVE_OVERLAY_INDEX, overlayIndex);
	data->AddBool(OVERLAY_VIEW_ARCHIVE_BUFFERED_DRAWING, bufferedDrawing);
	
	return B_OK;
}

//! Initializes the member variables.
void COverlayGraphView::Init()
{
	if(bufferedDrawing) {
		SetFlags(Flags() | B_FRAME_EVENTS);
	}
	
	textBackgroundColor	= CColor(0, 128, 0, 128);	// a transludent green
	textColor		 	= CColor::Green;
}

//: UI delegate factory method.
IUI *COverlayGraphView::CreateUI()
{
	IUI *newUI = new COverlayGraphViewUI(this);

	if(bufferedDrawing)
		return new CBufferedUI(this, newUI);
	else		
		return newUI;
}

//! Copies the content of the view.
// Overridden to invalidate the whole view.
void COverlayGraphView::CopyContent()
{
	// I can't blit the content, because of the overlay.
	Invalidate();
}

//! BeOS hook function.
void COverlayGraphView::AttachedToWindow()
{
	CGraphView::AttachedToWindow();
	
	bgColor = ViewColor();
	
	SetViewColor(CColor::Transparent);
}

//! Creates the context menu for this view.
// Called when a MSG_CONTEXT_MENU message is received.
// Returns the context menu.
BPopUpMenu *COverlayGraphView::ContextMenu()
{
	BPopUpMenu *contextMenu = CGraphView::ContextMenu();

	const char *overlayMenuLabel = B_TRANSLATE("Overlay");

	BMenu *overlaySubMenu = new BMenu(overlayMenuLabel);
	
	for(int i=0 ; i<dataInfoList.CountItems() ; i++) {
		CDataInfo *dataInfo = dataInfoList.ItemAt(i);

		if(dataInfo && dataInfo->DataProvider()) {
			BMessage *message = new BMessage(B_SET_PROPERTY);
			message->AddSpecifier(OVERLAY_VIEW_PROP_OVERLAY_INDEX);
			message->AddInt32("data", i);
		
			BMenuItem *item = new CColorSelectMenuItem(
				dataInfo->DataProvider()->DisplayName().String(), 
				dataInfo->Color(),
				message);

			item->SetTarget(this);
			item->SetMarked(i == overlayIndex);
		
			overlaySubMenu->AddItem(item);
		}
	}
	
	contextMenu->AddSeparatorItem();
	contextMenu->AddItem(overlaySubMenu);
	
	return contextMenu;
}

//! BeOS hook function.
status_t COverlayGraphView::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)OVERLAY_VIEW_PROP_OVERLAY_INDEX,// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)OVERLAY_VIEW_PROP_TEXT_COLOR,	// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 3rd property
			(char *)OVERLAY_VIEW_PROP_TEXT_BG_COLOR,// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 4th property
			(char *)OVERLAY_VIEW_PROP_BG_COLOR,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-overlay-graph-view");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return CGraphView::GetSupportedSuites(message);
}

//! BeOS hook function.
BHandler *COverlayGraphView::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_SET_PROPERTY:
			// FALL THROUGH
		case B_GET_PROPERTY:
			if(what == B_DIRECT_SPECIFIER) {
				if( strcmp(property, OVERLAY_VIEW_PROP_OVERLAY_INDEX) == 0 ||
					strcmp(property, OVERLAY_VIEW_PROP_TEXT_COLOR) == 0 ||
					strcmp(property, OVERLAY_VIEW_PROP_TEXT_BG_COLOR) == 0 ||
					strcmp(property, OVERLAY_VIEW_PROP_BG_COLOR) == 0) {
					return this;
				}
			}
			break;
	}
	
	return CGraphView::ResolveSpecifier(message, index, specifier, what, property);
}

//! BeOS hook function.
void COverlayGraphView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case B_GET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					BMessage reply(B_REPLY);
					status_t result;
				
					if(strcmp(property, OVERLAY_VIEW_PROP_OVERLAY_INDEX) == 0) {
						// GET_PROPERTY for 'OverlayIndex' property.
						result = reply.AddInt32("result", overlayIndex);
					} else if(strcmp(property, OVERLAY_VIEW_PROP_TEXT_COLOR) == 0) {
						// GET_PROPERTY for 'TextColor' property.
						result = AddColor(&reply, "result", textColor);
					} else if(strcmp(property, OVERLAY_VIEW_PROP_TEXT_BG_COLOR) == 0) {
						// GET_PROPERTY for 'TextBackgroundColor' property.
						result = AddColor(&reply, "result", textBackgroundColor);
					} else if(strcmp(property, OVERLAY_VIEW_PROP_BG_COLOR) == 0) {
						// GET_PROPERTY for 'BackgroundColor' property.
						result = AddColor(&reply, "result", bgColor);
					} else {
						CGraphView::MessageReceived(msg);
						return;
					}

					send_script_reply(reply, result, msg);
					msg->PopSpecifier();										
				}
			}
			break;
		case B_SET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					status_t result;
					BMessage reply(B_REPLY);
				
					if(strcmp(property, OVERLAY_VIEW_PROP_OVERLAY_INDEX) == 0) {
						// SET_PROPERTY for 'OverlayIndex' property.
						int32 newValue;
						
						if((result = msg->FindInt32("data", &newValue)) == B_OK) {
							overlayIndex = newValue;
							Invalidate();
						}
					} else if(strcmp(property, OVERLAY_VIEW_PROP_TEXT_COLOR) == 0) {
						// SET_PROPERTY for 'TextColor' property.
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							textColor = newValue;
							Invalidate();
						}
					} else if(strcmp(property, OVERLAY_VIEW_PROP_TEXT_BG_COLOR) == 0) {
						// SET_PROPERTY for 'TextBackgroundColor' property.
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							textBackgroundColor = newValue;
							Invalidate();
						}
					} else if(strcmp(property, OVERLAY_VIEW_PROP_BG_COLOR) == 0) {
						// SET_PROPERTY for 'BackgroundColor' property.
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							bgColor = newValue;
							Invalidate();
						}
					} else {
						CGraphView::MessageReceived(msg);
						return;
					}
					
					send_script_reply(reply, result, msg);
					msg->PopSpecifier();										
				}
			}
			break;
		case 'PSTE':
			if(msg->WasDropped()) {
				// a color was droped from the 'ColorSelector' utility
				if(FindColor(msg, "RGBColor", bgColor) == B_OK) {
					Invalidate();
				}
			}
			break;
		default:
			CGraphView::MessageReceived(msg);
	}
}
