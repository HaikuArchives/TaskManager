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
#include "MenuField.h"

float estimate_menu_field_width(const char *label, BMenu *menu)
{
	const float dist = 10;

	float menuWidth = 0.0;	// Estimated width of the menu.

	for(int32 i=0 ; i<menu->CountItems() ; i++) {
		BMenuItem *item = menu->ItemAt(i);
		
		menuWidth = MAX(menuWidth, be_plain_font->StringWidth(item->Label()));
	}
	
	float labelWidth = be_plain_font->StringWidth(label)+dist;
	
	// The 30 pixels are font size independet. They are added
	// because of:
	// * the borders around the menu.
	// * the small arrow right of the menu.
	// * other distances.
	float totalWidth  = menuWidth+labelWidth+3*dist;

	return totalWidth;
}

BMenuField *create_menu_field(BPoint position, const char *name, const char *label, BMenu *menu, uint32 resizingMode)
{
	// Menufields don't automatically calculate the size of the menu entries.
	// And they aren't correctly horizonatlly resizable. This means
	// I have to create the BMenuField with the width of the widest entry
	// plus the size of the label.
	
	const float dist = 10;
	float labelWidth = be_plain_font->StringWidth(label)+dist;
	float totalWidth = estimate_menu_field_width(label, menu);
	
	BRect rect(	position.x, position.y, 
				position.x+totalWidth,
				position.y+10 );
	
	BMenuField *menuField = new BMenuField(rect, name, label, menu, resizingMode);
	
	menuField->SetDivider(labelWidth);
	
	// The height of the menufields is correctly set by
	// ResizeToPreferred.
	menuField->ResizeToPreferred();
	
	return menuField;
}