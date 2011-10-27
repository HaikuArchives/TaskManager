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

#ifndef FILE_GLYPH_MENU_ITEM_H
#define FILE_GLYPH_MENU_ITEM_H

#include "GlyphMenuItem.h"

class CFileGlyphMenuItem : public CGlyphMenuItem
{
	public:
	CFileGlyphMenuItem(
		const char *label,
		const BPath *path,
		bool isDefaultEntry=false,
		BMessage *message=NULL,
		char shortcut=0,
		uint32 modifiers=0,
		BPoint offset=B_ORIGIN) :
	CGlyphMenuItem(label, NULL, isDefaultEntry, message, 
		shortcut, modifiers, offset)
	{
		entry_ref entryRef;
		BEntry entry(path->Path(), true);
		
		entry.GetRef(&entryRef);
		
		InitIcon(&entryRef);
		
		if(label == NULL) {
			SetLabel(path->Leaf());
		}
	}	
	
	CFileGlyphMenuItem(
		const char *label,
		const BEntry *entry,
		bool isDefaultEntry=false,
		BMessage *message=NULL,
		char shortcut=0,
		uint32 modifiers=0,
		BPoint offset=B_ORIGIN) :
	CGlyphMenuItem(label, NULL, isDefaultEntry, message, 
		shortcut, modifiers, offset)
	{
		entry_ref entryRef;
	
		// get entry_ref for file
		entry->GetRef(&entryRef);

		InitIcon(&entryRef);
		
		if(label == NULL) {
			BPath path;
			
			entry->GetPath(&path);
			
			SetLabel(path.Leaf());
		}
	}
	
	protected:
	
	void InitIcon(const entry_ref *entryRef)
	{
		BBitmap *icon = new BBitmap(BRect(0,0,15,15), B_COLOR_8_BIT);
	
		BNode node(entryRef);
		BNodeInfo nodeInfo(&node);
	
		// load icon for file
		if(nodeInfo.GetTrackerIcon(icon, B_MINI_ICON) != B_OK) {
			delete icon;
			icon = NULL;
		}
	
		glyph = icon;
	}
};

#endif // FILE_GLYPH_MENU_ITEM_H