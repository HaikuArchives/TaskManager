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

#ifndef COLOR_SELECT_LIST_ITEM_H
#define COLOR_SELECT_LIST_ITEM_H

class CColorSelectListItem : public BListItem
{
	public:
	CColorSelectListItem(const char *label, rgb_color color, int32 level=0, bool expanded=true);
	virtual ~CColorSelectListItem();
	
	const char *Text();
	virtual void SetText(const char *t);

	rgb_color Color();
	virtual void SetColor(rgb_color c);
	
	virtual void DrawItem(BView *owner, BRect itemRect, bool drawEverything=false);
	virtual void Update(BView *owner, const BFont *font);
	
	protected:
	char *text, *truncString;
	rgb_color color;
};

#endif // COLOR_SELECT_LIST_ITEM_H