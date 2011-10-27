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

/* 
 * Extends the column listview from "Santas Gift Bar" by
 * - adding scripting properties
 * - extending the default items by painters
 * - double buffered display of the listview to avoid flicker
 */

#ifndef COLUMN_LIST_VIEW_EX_H
#define COLUMN_LIST_VIEW_EX_H

//! file=ColumnListViewEx.h

// ====== Scripting Properties ======

// CColumnListViewEx
extern const char * const COLUMN_LIST_VIEW_PROP_COLUMN;				// Column "object"
extern const char * const COLUMN_LIST_VIEW_PROP_COLUMN_VISIBLE;		// bool
extern const char * const COLUMN_LIST_VIEW_PROP_COLUMN_WIDTH;		// float
extern const char * const COLUMN_LIST_VIEW_PROP_COLUMN_LABEL;		// string (read only)
extern const char * const COLUMN_LIST_VIEW_PROP_COLUMN_SORT_MODE;	// int32 0=ascending, 1=descending, 2=no sort

// ====== Includes ======

#include "CLVEasyItem.h"
#include "ColumnListView.h"

// ====== Enumerations ======

enum ExTypes
{
	CLVColPainter = 5,
};

// ====== Class Defs ======

class CContextMenuDetector;

// interface for painter objects
class CLVPainter
{
	public:
	CLVPainter() {}
	virtual ~CLVPainter() {}
	
	virtual void DrawItemColumn(BView *owner, BRect item_column_rect, bool complete) = 0;	
	virtual void ColumnWidthChanged(float column_width, ColumnListView* the_view) = 0;
	virtual void FrameChanged(BRect new_frame, ColumnListView *the_view) = 0;
	
	virtual float ContentWidth(BView *owner, BFont *font) = 0;
};

// painter which can display text right and left aligned in a column
class CLVTextPainter : public CLVPainter
{
	public:
	CLVTextPainter(const char *t, bool _rightAlign=false, 
					const rgb_color &_textColor=CColor::Black);
	virtual ~CLVTextPainter();

	virtual void DrawItemColumn(BView *owner, BRect item_column_rect, bool complete);	
	virtual void ColumnWidthChanged(float column_width, ColumnListView* the_view);
	virtual void FrameChanged(BRect new_frame, ColumnListView *the_view) {}

	virtual float ContentWidth(BView *owner, BFont *font);
	
	virtual const char *Text() const;
	virtual void SetText(const char *t);

	protected:
	bool  		mustTruncate, rightAlign;
	rgb_color	textColor;
	char *		text;
	char *		truncText;
}; 

// Can contain bitmaps, strings and painter objects as column content.
// Painter objects are responsible for displaying the content of one
// column. They allow a more flexible use of the column list view.
class CLVEasyItemEx : public CLVEasyItem
{
	public:
	CLVEasyItemEx(uint32 level = 0, bool superitem = false, bool expanded = false, float minheight = 0.0);
	virtual ~CLVEasyItemEx();

	virtual void FrameChanged(int32 column_index, BRect new_frame, ColumnListView *the_view);
	virtual void ColumnWidthChanged(int32 column_index, float column_width, ColumnListView* the_view);
	virtual void DrawItemColumnInner(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
	
	virtual void SetColumnContent(int column_index, CLVPainter *newPainter);
	virtual void SetColumnContent(int column_index, const char *text, bool truncate = true);
	virtual void SetColumnContent(int column_index, const BBitmap *bitmap, float horizontal_offset = 2.0, bool copy = true);

	virtual float GetColumnContentWidth(BView *owner, BFont *font, int column_index) const;

	virtual const char* GetColumnContentText(int column_index) const;
	virtual CLVPainter *GetColumnContentPainter(int column_index) const;
	
	virtual void DisplayContextMenu(BView *owner, BPoint point);
};

// displays a picture inside the view
class CPictureView : public BView
{
	public:
	CPictureView(BRect frame, BPicture *picture=NULL);
	virtual ~CPictureView();
	
	virtual void Draw(BRect updateRect);
	virtual void AttachedToWindow();
	
	BPicture *Picture() { return pict; }
	void SetPicture(BPicture *newPict);
	void SetOffset(BPoint o) { offset = o; }
	
	protected:
	BPicture *	pict; 
	BPoint 		offset;
};

// Extends the normal column list view.
// It renders itself into a offscreen bitmap and then displays the bitmap
// in order to reduce flickering.
class CColumnListViewEx : public ColumnListView
{
	public:
	CColumnListViewEx(	BRect Frame,
						CLVContainerView** ContainerView,						
						const char* Name = NULL,
						uint32 ResizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
						list_view_type Type = B_SINGLE_SELECTION_LIST,
						bool hierarchical = false,
						bool horizontal = true,					
						bool vertical = true,
						bool scroll_view_corner = true,
						border_style border = B_NO_BORDER,
						const BFont* LabelFont = be_plain_font);
	virtual ~CColumnListViewEx();

	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MessageReceived(BMessage *msg);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
						BMessage *specifier, int32 what, const char *property);
	virtual status_t GetSupportedSuites(BMessage *message);

	BRect ColumnRect(int columnIndex) const;

	protected:
	status_t GetColumnIndex(BMessage *specifier, int32 what, int32 &columnIndex);

	BRect CreateOffscreenBuffer();

	float 					 oldWidth, oldHeight;
	BBitmap 				*offscreenBuffer;
	CContextMenuDetector	*detector;
};

#endif // COLUMN_LIST_VIEW_EX_H