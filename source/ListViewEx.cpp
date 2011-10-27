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
#include "ListViewEx.h"

// ==== COutlineListViewEx ====

COutlineListViewEx::COutlineListViewEx(BRect frame, const char *name, 
	list_view_type type, uint32 resizingMode, uint32 flags) :
	BOutlineListView(frame, name, type, resizingMode, flags)
{
}
	
COutlineListViewEx::~COutlineListViewEx()
{
	BListItem *item;

	// delete memory allocated by list items.
	for(int32 i=0 ; (item = FullListItemAt(i)) != NULL ; i++)
		delete item;
}

void COutlineListViewEx::FullListSelect(int32 index, bool extend)
{
	BListItem *selectItem = FullListItemAt(index);
	
	if(!selectItem)
		return;
	
	// --- expand all parent items

	BList parents;
	BListItem *parent = Superitem(selectItem);
	
	while(parent) {
		parents.AddItem(parent);
		parent = Superitem(parent);
	}
	
	// Expand starting at root. Otherwise the list view corrupts the
	// list. (Another BeOS bug...)
	for(int i=parents.CountItems()-1 ; i>= 0 ; i--) {
		parent = reinterpret_cast<BListItem *>(parents.ItemAt(i));
		
		Expand(parent);
	}
	
	// --- select item
	
	// I can't use 'index' here, because that's an index in the FULL list.
	int32 listIndex = IndexOf(selectItem);
	
	Select(listIndex, extend);
}

// ==== CFocusListView ====

// A CFocusListView isn't navigable, if it's empty. Therfore the constructor
// clears the B_NAVIGABLE flag. The flag is set, when entries are added
// through AddItem.
CFocusListView::CFocusListView(
	BRect frame,
	const char *name,
	list_view_type type,
	uint32 resizingMode,
	uint32 flags) :
	CListViewEx(frame, name, type, resizingMode, flags & ~B_NAVIGABLE)
{
}

void CFocusListView::WindowActivated(bool activated)
{
	// remove focus mark is window is deactivated and redraw
	// it, if it's reactivated.
	int32 selIndex, i=0;

	while((selIndex = CurrentSelection(i++)) >= 0) {
		InvalidateItem(selIndex);
	}
	
	BListView::WindowActivated(activated);
}

void CFocusListView::MakeFocus(bool focus)
{
	int32 selIndex, i=0;

	if(CurrentSelection(0) < 0 && focus) {
		// No selection. If no item is selected I don't display
		// a focus mark.
		Select(0);
	}

	while((selIndex = CurrentSelection(i++)) >= 0) {
		InvalidateItem(selIndex);
	}
	
	CListViewEx::MakeFocus(focus);
}

void CFocusListView::MouseDown(BPoint point)
{
	// MakeFocus(true);

	CListViewEx::MouseDown(point);
}

bool CFocusListView::AddItem(BListItem *item)
{
	bool wasEmpty = IsEmpty();
	
	bool success = CListViewEx::AddItem(item);

	if(wasEmpty && success) {
		// Set navigable flag
		SetFlags(Flags() | B_NAVIGABLE);
		Select(CountItems()-1);
	}
	
	return success;
}

bool CFocusListView::AddItem(BListItem *item, int32 index)
{
	bool wasEmpty = IsEmpty();
	
	bool success = CListViewEx::AddItem(item, index);

	if(wasEmpty && success) {
		// Set navigable flag
		SetFlags(Flags() | B_NAVIGABLE);
		Select(index);
	}
	
	return success;
}

bool CFocusListView::RemoveItem(BListItem *item)
{
	return RemoveItem(IndexOf(item)) != NULL;
}

BListItem *CFocusListView::RemoveItem(int32 index)
{
	BListItem *item = CListViewEx::RemoveItem(index);
	
	if(IsEmpty()) {
		// Clear navigable flag
		SetFlags(Flags() & ~B_NAVIGABLE);
	}
	
	return item;
}
