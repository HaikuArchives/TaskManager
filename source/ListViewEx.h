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

#ifndef LIST_VIEW_EX_H
#define LIST_VIEW_EX_H

class CListViewEx : public BListView
{
	public:
	CListViewEx(BRect frame,
		const char *name, 
		list_view_type type=B_SINGLE_SELECTION_LIST, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS) :
		BListView(frame, name, type, resizingMode, flags) {}
		
	virtual ~CListViewEx()
	{
		BListItem *item;

		// delete memory allocated by list items.
		for(int32 i=0 ; (item = ItemAt(i)) != NULL ; i++)
			delete item;
	}
};

class COutlineListViewEx : public BOutlineListView
{
	public:
	COutlineListViewEx(BRect frame,
		const char *name, 
		list_view_type type=B_SINGLE_SELECTION_LIST, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
	virtual ~COutlineListViewEx();
	
	void FullListSelect(int32 index, bool extend=false);
};

class CFocusListView : public CListViewEx
{
	public:
	CFocusListView(BRect frame,
		const char *name, 
		list_view_type type=B_SINGLE_SELECTION_LIST, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
		
	virtual void MakeFocus(bool focus);
	virtual void MouseDown(BPoint point);
	virtual void WindowActivated(bool activated);
	
	virtual bool AddItem(BListItem *item, int32 index);
	virtual bool AddItem(BListItem *item);
	virtual BListItem *RemoveItem(int32 index);
	virtual bool RemoveItem(BListItem *item);
};

#endif // LIST_VIEW_EX_H