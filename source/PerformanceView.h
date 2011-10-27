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

#ifndef PERFORMANCE_VIEW_H
#define PERFORMANCE_VIEW_H

// ===== Includes ======

#include "DialogBaseEx.h"
#include "ListViewEx.h"
#include "MakSplitterView.h"	// splitter view
#include "Singleton.h"

// ===== Message IDs ======

const int32 MSG_VIEW_PERFORMANCE_OBJECTS	= 'mVPO';
const int32 MSG_REMOVE_PERFORMANCE_OBJECT	= 'mRPO';
const int32 MSG_CHANGE_PRERFORMANCE_OBJECT	= 'mCPO';
const int32 MSG_VIEW_COLOR_SELECTOR			= 'mVCS';
const int32 MSG_SCALE_SELECTED				= 'mSSE';
const int32 MSG_SELECT_DATA_PROVIDER		= 'mSED';
const int32 MSG_SELECTION_CHANGED			= 'mSEC';

// ===== Message Fields ======

// MSG_SELECT_DATA_PROVIDER
extern const char * const MESSAGE_DATA_ID_PERF_COUNTER_PATH;		// string (optional)

// ===== Class Defs ======

class CGraphView;
class COverlayGraphView;
class IDataProvider;
class IPerformanceCounter;

class CPerformanceAddWindow : public CSingletonWindow
{
	public:
	static CPerformanceAddWindow *CreateInstance();
	
	virtual ~CPerformanceAddWindow();
	virtual bool QuitRequested();
	virtual void DispatchMessage(BMessage *message, BHandler *target);

	BInvoker *Invoker() {
		BAutolock autoLock(this);
		return dynamic_cast<BInvoker *>(ChildAt(0));
	}
	
	protected:
	CPerformanceAddWindow();
	
	friend CSingleton;
};

class CPerformanceAddView : public CLocalizedDialogBase, public BInvoker
{
	public:
	CPerformanceAddView(BRect frame, BHandler *handler);	
	virtual ~CPerformanceAddView();
	
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage *message);
	virtual void GetPreferredSize(float *width, float *height);
	
	protected:
	void FillTreeView();
	void AddChildren(IPerformanceCounter *counter, BListItem *item);
	void CreateColorMenuField();
	void CreateScaleMenuField();

	virtual bool Ok();

	status_t GetDataProviderInfo(
			IDataProvider *dataProvider,
			bool          &partOfGraphView,
			rgb_color     &dataProviderColor,
			float         &dataProviderScale);

	status_t GetClientProperty(
			const char *propName,
			BMessage *propMsg,
			int32 index=-1);

	rgb_color SelectedColor();
	int32 IndexOfColor(rgb_color color);

	void SelectColor(rgb_color color, bool addColor=false);
	void SelectAutomatic();
	void SelectScale(float scale);

	void LoadUserdefinedColors();
	void SaveUserdefinedColors();
	void AddToUserdefinedColors(const rgb_color &color);
	BMenuItem *CreateColorItem(const rgb_color &color, const char *name=NULL);
	const char *PrefBaseName() { return "UserDefColor_"; }

	class CCounterItem : public BStringItem
	{
		public:
		CCounterItem(IPerformanceCounter *counter);
		virtual ~CCounterItem();

		IDataProvider *DataProvider() { return dataProvider; }
		const char *Path() { return path.String(); }
		
		protected:
		IDataProvider *dataProvider;
		BString path;
	};
	
	static const float dist;
	static const int32 MAX_USERDEFINED_COLORS;

	static int ItemCompareFunc(const BListItem *p1, const BListItem *p2)
	{
		return strcmp(dynamic_cast<const BStringItem *>(p1)->Text(), 
				dynamic_cast<const BStringItem *>(p2)->Text());
	}

	COutlineListViewEx 	*treeView;
	BMenuField 			*colorSelect;
	BMenuField 			*scaleSelect;
	
	rgb_color			 selColor;			// currently selected color
	float				 selScale;			// currently selected scale
	bool				 autoSelectColor;
	CPointerList<CColor> userDefinedColors;
};

class CSplitterView : public MakSplitterView
{
	public:
	CSplitterView(BRect frame, BView *left, BView *right,
		bool horz, uint32 resizingMode, rgb_color bgColor=CColor::BeBackgroundGray);
	virtual ~CSplitterView() {}
	
	virtual void Draw(BRect updateRect);
};

class CLegendListView : public CFocusListView
{
	public:
	CLegendListView(BRect frame, const char *name, 
		list_view_type type=B_SINGLE_SELECTION_LIST,
		uint32 resizingMode=B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags=B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
		
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void FrameResized(float width, float height);
};

class CPerformanceView : public BView
{
	public:
	CPerformanceView(BRect frame);
	virtual ~CPerformanceView() {}
	
	virtual void AttachedToWindow();
	virtual void AllAttached();
	virtual void DetachedFromWindow();
	virtual void MessageReceived(BMessage *message);

	static void DisplayAddDialog(const char *selCounterPath);

	protected:
	COverlayGraphView	*graphView;
	BListView			*listView;
	BView				*leftPane, *rightPane;
	BButton 			*addButton;
	BButton				*removeButton;
};

#endif // PERFORMANCE_VIEW_H