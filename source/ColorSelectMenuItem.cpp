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
#include "ColorSelectMenuItem.h"

//: Constructor
//!param: label - The label.
//!param: color - The displayed color.
//!param: message - The invocation message.
//!param: shortcut - The shortcut key.
//!param: modifiers - The shortcut modifiers.
CColorSelectMenuItem::CColorSelectMenuItem(const char *label, rgb_color _color, 
	BMessage *message, char shortcut, uint32 modifiers) :
	BMenuItem(label, message, shortcut, modifiers)
{
	color = _color;	
}

//: BeOS hook function.
void CColorSelectMenuItem::DrawContent()
{
	BPoint pos = ContentLocation();

	Menu()->MovePenTo(pos.x+boxSize+10.0, pos.y);
	BMenuItem::DrawContent();

	rgb_color oldColor  = Menu()->HighColor();
	
	if(IsEnabled())
		Menu()->SetHighColor(CColor::Black);
	else
		Menu()->SetHighColor(CColor::BeDarkShadow);

	Menu()->StrokeRect(BRect(pos.x+2, pos.y+2, pos.x+boxSize+2, pos.y+boxSize+2));

	if(IsEnabled())
		Menu()->SetHighColor(color);
	else
		Menu()->SetHighColor(CColor::BeInactiveGray);

	Menu()->FillRect(BRect(pos.x+3, pos.y+3, pos.x+boxSize+1, pos.y+boxSize+1));
	
	Menu()->SetHighColor(oldColor);
}

//: BeOS hook function.
void CColorSelectMenuItem::GetContentSize(float *width, float *height)
{
	BMenuItem::GetContentSize(width, height);

	*width += 10.0;

	boxSize = (*height)-4.0;
}
