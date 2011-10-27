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

#ifndef GRAPH_VIEW_H
#define GRAPH_VIEW_H

//! file=GraphView.h

// ====== Includes ======

#include "PulseView.h"
#include "PointerList.h"

// ====== Archive Fields ======

// CGraphView
extern const char * const GRAPH_VIEW_ARCHIVE_VALUE_COUNT;			// int32
extern const char * const GRAPH_VIEW_ARCHIVE_DISTANCE;				// int32
extern const char * const GRAPH_VIEW_ARCHIVE_MAX_VALUE;				// int32
extern const char * const GRAPH_VIEW_ARCHIVE_GRID_SPACE;			// int32
extern const char * const GRAPH_VIEW_ARCHIVE_GRID_COLOR;			// rgb_color
extern const char * const GRAPH_VIEW_ARCHVIE_DATA_INFO_LIST;		// CDataInfo[]
extern const char * const GRAPH_VIEW_ARCHIVE_AUTO_SCALE;			// bool

// CDataInfo
extern const char * const DATA_INFO_ARCHIVE_VALUE_COUNT;			// int32
extern const char * const DATA_INFO_ARCHIVE_LINE_COLOR;				// rgb_color
extern const char * const DATA_INFO_ARCHIVE_SCALE;					// float
extern const char * const DATA_INFO_ARCHIVE_DATA_PROVIDER;			// CArchivableDataProvider

// COverlayGraphView
extern const char * const OVERLAY_VIEW_ARCHIVE_OVERLAY_INDEX;		// int32
extern const char * const OVERLAY_VIEW_ARCHIVE_BUFFERED_DRAWING;	// bool

// ====== Scripting Properties ======

// CGraphView
extern const char * const GRAPH_VIEW_PROP_DATA_INFO;				// DataInfo "object"
extern const char * const GRAPH_VIEW_PROP_AUTO_SCALE;				// bool
extern const char * const GRAPH_VIEW_PROP_POINT_DISTANCE;			// int32
extern const char * const GRAPH_VIEW_PROP_GRID_SPACE;				// int32
extern const char * const GRAPH_VIEW_PROP_GRID_COLOR;				// rgb_color
extern const char * const GRAPH_VIEW_PROP_MAX_VALUE;				// float

// CDataInfo
extern const char * const DATA_INFO_PROP_COLOR;						// rgb_color
extern const char * const DATA_INFO_PROP_SCALE;						// float
extern const char * const DATA_INFO_PROP_MAX;						// float
extern const char * const DATA_INFO_PROP_AVG;						// float
extern const char * const DATA_INFO_PROP_CURRENT;					// float
extern const char * const DATA_INFO_PROP_DATA_PROVIDER;				// IDataProvider *

// COverlayGraphView
extern const char * const OVERLAY_VIEW_PROP_OVERLAY_INDEX;			// int32
extern const char * const OVERLAY_VIEW_PROP_TEXT_COLOR;				// rgb_color
extern const char * const OVERLAY_VIEW_PROP_TEXT_BG_COLOR;			// rgb_color
extern const char * const OVERLAY_VIEW_PROP_BG_COLOR;				// rgb_color

// ====== Message IDs ======

//: Add a data provider to the view.
// The message has the identical effect to a AddDataProvider call.
// Message fields:
// <UL>
// <LI>MESSAGE_DATA_ID_DATA_PROVIDER - The IDataProvider object to add.
// <LI>MESSAGE_DATA_ID_SCALE - The scaling applied to the samples.
// <LI>MESSAGE_DATA_ID_COLOR - The color.
// </UL>
const int32 MSG_ADD_DATA_PROVIDER			= 'mADP';

//: Notification sent when a new dataprovider was added.
// This message is sent to the registered notification handler.
// Message fields:
// <UL>
// <LI>MESSAGE_DATA_ID_DATA_PROVIDER - The added IDataProvider.
// <LI>MESSAGE_DATA_ID_SCALE - The scaling applied to the samples.
// <LI>MESSAGE_DATA_ID_COLOR - The color.
// </UL>
const int32 MSG_NOTIFY_DATA_PROVIDER_ADDED	= 'mNDP';

//: Notification sent CDataInfo object was changed.
// This message is sent to the registered notification handler.
// Message fields:
// <UL>
// <LI>MESSAGE_DATA_ID_DATA_PROVIDER - The changed IDataProvider.
// <LI>MESSAGE_DATA_ID_SCALE - The scaling applied to the samples.
// <LI>MESSAGE_DATA_ID_COLOR - The color.
// </UL>
const int32 MSG_NOTIFY_DATA_INFO_CHANGED	= 'mDIC';

//: Notification sent CDataInfo object was removed.
// This message is sent to the registered notification handler.
// Message fields:
// <UL>
// <LI>MESSAGE_DATA_ID_INDEX - Index of the removed object.
// </UL>
const int32 MSG_NOTIFY_DATA_INFO_DELETED	= 'mDID';

// ====== Message Fields ======

// MSG_ADD_DATA_PROVIDER and MSG_SELECT_DATA_PROVIDER
extern const char * const MESSAGE_DATA_ID_DATA_PROVIDER;			// IDataProvider *

// MSG_NOTIFY_DATA_INFO_CHANGED, MSG_NOTIFY_DATA_PROVIDER_ADDED and
// MSG_ADD_DATA_PROVIDER
extern const char * const MESSAGE_DATA_ID_SCALE;					// float

// ====== Class Defs ======

class CGraphView;
class COverlayGraphView;
class CContextMenuDetector;
class IDataProvider;

//: Interface for UI delegates.
// A UI delegate is responsible for drawing the attached view either
// into the view itself or into some offscreen buffer.
class IUI
{
	public:
	IUI() {}
	virtual ~IUI() {}
	
	//: Called to draw the attached view.
	//!param: view - The view used for drawing.
	//!param: updateRect - The update rect.
	virtual void Draw(BView *view, const BRect &updateRect) = 0;
	
	//: Called when the attached view is resized.
	//!param:  view - The resized view.
	//!param:  width - New width of the view.
	//!param:  height - New height of the view.
	virtual void FrameResized(BView *view, float width, float height) = 0;
};

//: Information about a data provider added to a CGraphView.
class _EXPORT CDataInfo : public BHandler
{
	public:
	CDataInfo(CPulseView *view,
		int32 _valueCount,
		IDataProvider *provider,
		rgb_color _color,
		float _scale=1.0);
	CDataInfo(BMessage *archive);
	virtual ~CDataInfo();		

	virtual status_t Archive(BMessage *archive, bool deep) const;
	static BArchivable *Instantiate(BMessage *archive);

	virtual void MessageReceived(BMessage *message);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
		BMessage *specifier, int32 what, const char *property);
	virtual status_t GetSupportedSuites(BMessage *message);
	virtual status_t SetProperty(const char *property, int32 what, BMessage *message);
	virtual status_t GetProperty(const char *property, int32 what, BMessage &reply);

	void SetView(CPulseView *_view) { view = _view; }
	void SetDataProvider(IDataProvider *provider);
	void SetColor(rgb_color c) { color = c; }
	void SetScale(float s) { scale = s; }

	IDataProvider *DataProvider() const { return dataProvider; }
	rgb_color Color() const { return color; }
	float Scale() const { return scale; }
	float Max() const { return max; }
	float Cur() const { return valueArray[(insertPoint+1)%valueCount]; }
	float Avg() const { return avg; }

	float Value(int32 index) const;
	bool Update();
	void Clear();

	protected:
	void Init();

	CPulseView		 	 *view;				// GraphView which this object is attached to.
	float 				 *valueArray;		// Array of values (used as ring buffer)
	IDataProvider 		 *dataProvider;	
	int32 				  valueCount;		// Size of 'valueArray'.
	int32 				  insertPoint;		// Current insertion point into ring buffer.
	float 				  scale;
	float				  max;				// Maximum value in 'valueArray'
	float				  avg;				// Average
	int32				  avgValueCount;	// Number of values used to calc avg.
	rgb_color			  color;
};

//: UI delegate for CGraphView
class CGraphViewUI : public IUI
{
	public:
	CGraphViewUI(CGraphView *_graphView);
	
	virtual void Draw(BView *view, const BRect &updateRect);
	virtual void FrameResized(BView *view, float width, float height) {}
	
	protected:
	CGraphView *graphView;
};

//: UI delegate for COverlayGraphView
class COverlayGraphViewUI : public CGraphViewUI
{
	public:
	COverlayGraphViewUI(COverlayGraphView *_graphView);
	
	virtual void Draw(BView *view, const BRect &updateRect);
	
	protected:
	float maxStringWidth;
};

//: UI delegate using double buffered drawing.
class CBufferedUI : public IUI
{
	public:
	CBufferedUI(BView *view, IUI *ui);
	virtual ~CBufferedUI();
	
	virtual void Draw(BView *view, const BRect &updateRect);
	virtual void FrameResized(BView *view, float width, float heigth);
	
	protected:
	IUI 		*ui;
	BBitmap		*buffer;
};

class _EXPORT CGraphView : public CPulseView
{
	public:
	CGraphView(BRect frame, const char *title, int32 _maxValue, 
		int32 _distance=3, int32 _valueCount=512, int32 resizeMode=B_FOLLOW_ALL_SIDES, int32 flags=0);
	CGraphView(BMessage *archive);
	virtual ~CGraphView();

	static BArchivable *Instantiate(BMessage *archive);
	
	virtual	status_t Archive(BMessage *data, bool deep = true) const;

	virtual void Draw(BRect updateRect);
	virtual void AttachedToWindow();
	virtual void Pulse();
	virtual void MessageReceived(BMessage *msg);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
		BMessage *specifier, int32 what, const char *property);
	virtual status_t GetSupportedSuites(BMessage *message);

	virtual void CopyContent();
	
	int32 MaxValue() { return maxValue; }
	void  SetMaxValue(int32 mv) { maxValue = mv; }
	
	int32 GridSpace()   	{ return gridSpace; }
	int32 GridOffset()  	{ return gridOffset; }
	int32 ValueCount()		{ return valueCount; }
	int32 PointDistance()	{ return distance; }

	int32 AddDataProvider(IDataProvider *provider, 
				rgb_color color, float scale=1.0, 
				bool *updated=NULL);
	
	const CDataInfo *DataProviderAt(int32 index) const;
	int32 CountDataProvider() const;
	
	void SetGridColor(rgb_color color) { gridColor = color; }
	rgb_color GridColor() const { return gridColor; }

	void SetAutoScale(bool as) { autoScale = as; }
	bool AutoScale() { return autoScale; }

	void SetNotification(BHandler *handler, BMessage *message=NULL);

	void SendNotify_DataInfoChanged(int32 dataInfoIndex);
	void SendNotify_DataProviderAdded(int32 dataInfoIndex);
	void SendNotify_DataInfoDeleted(int32 dataInfoIndex);

	protected:
	void Init();

	virtual BPopUpMenu *ContextMenu();
	virtual IUI *CreateUI();

	bool GetNextDataInfoIndex(
			BMessage *specifier, 
			int32 what, 
			int32 /*[out]*/ &dataInfoIndex,
			int32 /*[in, out]*/ &cookie,
			status_t /*[out]*/ &result);
			
	int32 distance;
	int32 maxValue;
	int32 gridSpace;
	int32 gridOffset;
	int32 valueCount;

	bool autoScale;

	rgb_color gridColor;

	BMessenger *notifyMessenger;	// Target for notifications
	BMessage *notifyMessage;

	IUI *ui;

	CContextMenuDetector *detector;
	
	CPointerList<CDataInfo> dataInfoList;
};

class _EXPORT COverlayGraphView : public CGraphView
{
	public:
	COverlayGraphView(BRect frame, const char *title, bool bufferedDraw, int32 maxValue, int32 distance=3, int32 valueCount=512);
	COverlayGraphView(BMessage *archive);
	virtual ~COverlayGraphView();

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;

	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage *msg);
	virtual void FrameResized(float width, float height);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
		BMessage *specifier, int32 what, const char *property);
	virtual status_t GetSupportedSuites(BMessage *message);

	virtual void CopyContent();

	void SetOverlayIndex(int32 dataInfoIndex) { overlayIndex = dataInfoIndex; }
	int32 OverlayIndex() const { return overlayIndex; }
	
	rgb_color OverlayTextColor() const			{ return textColor; }
	rgb_color OverlayBackgroundColor() const	{ return textBackgroundColor; }
	rgb_color BackgroundColor() const			{ return bgColor; }

	protected:
	virtual BPopUpMenu *ContextMenu();
	virtual IUI *CreateUI();

	void Init();
	
	int32 		 overlayIndex;
	bool		 bufferedDrawing;
	rgb_color	 bgColor;
	rgb_color	 textBackgroundColor;
	rgb_color	 textColor;
};

#endif // GRAPH_VIEW_H