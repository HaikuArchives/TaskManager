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
 
#ifndef COLOR_SELECT_MENU_ITEM_H
#define COLOR_SELECT_MENU_ITEM_H

//! file=ColorSelectMenuItem.h

//: A menu item that displays a color as well as a label.
// The color is displayed in a small box in front of the label.
class CColorSelectMenuItem : public BMenuItem
{
	public:
	CColorSelectMenuItem(const char *label, rgb_color color, 
		BMessage *message, char shortcut=0, uint32 modifiers=0);
	virtual ~CColorSelectMenuItem() {}
	
	virtual void DrawContent();
	virtual void GetContentSize(float *width, float *height);
	
	rgb_color Color() { return color; }
	void SetColor(rgb_color c) { color = c; }
	
	protected:
	rgb_color color;
	float boxSize;
};

#endif // COLOR_SELECT_MENU_ITEM_H