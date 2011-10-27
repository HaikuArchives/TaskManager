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

#ifndef GLYPH_MENU_ITEM_H
#define GLYPH_MENU_ITEM_H

//! file=GlyphMenuItem.h

//: Menu item with an icon in front of the label
// Besides of the icon, this class be used to display a 'default' menu
// item. The label of the default item of a menu is displayed using a
// bold font.
class CGlyphMenuItem : public BMenuItem
{
	public:
	CGlyphMenuItem(const char *label, BBitmap *glyph_bm, 
					bool isDefaultEntry, BMessage *message=NULL, 
					char shortcut=0, uint32 modifiers=0,
					BPoint _offset=B_ORIGIN);

	//: Destructor
	virtual ~CGlyphMenuItem();
	
	protected:
	virtual void DrawContent();
	virtual void GetContentSize(float *width, float *height);
	
	BFont SetDrawFont();
	
	//: Icon displayed in front of the label.
	BBitmap *glyph;
	
	//: If this member is 'true', the label is drawn using a bold font.
	bool isDefault;
	
	//: Offset from the content location.
	BPoint offset;		
};

#endif // GLYPH_MENU_ITEM_H