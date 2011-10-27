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
#include "ColorSelectListItem.h"

CColorSelectListItem::CColorSelectListItem(const char *label, 
	rgb_color color, int32 level, bool expanded) :
	BListItem(level, expanded)
{
	truncString = NULL;
	text = NULL;

	SetText(label);
	SetColor(color);
}

CColorSelectListItem::~CColorSelectListItem()
{
	delete [] text;
	delete [] truncString;
}

const char *CColorSelectListItem::Text()
{
	return text;
}

void CColorSelectListItem::SetText(const char *t)
{
	if(text) delete [] text;
	text = new char [strlen(t)+1];
	strcpy(text, t);
	
	delete [] truncString;
	truncString = NULL;
}

rgb_color CColorSelectListItem::Color()
{
	return color;
}

void CColorSelectListItem::SetColor(rgb_color c)
{
	color = c;
}

void CColorSelectListItem::DrawItem(BView *owner, BRect itemRect, bool drawEverything)
{
	if(drawEverything) {
		rgb_color color;
		
		if(IsSelected())
			color = CColor::BeListSelectGray;
		else
			color = owner->ViewColor();
	
		owner->SetHighColor(color);
		owner->FillRect(itemRect);
	}

	if(owner->IsFocus() && owner->Window()->IsActive() && IsSelected()) {
		owner->SetHighColor(CColor::BeFocusBlue);
		owner->StrokeRect(itemRect);
	}

	BFont ownerFont;
	font_height fh;

	owner->GetFont(&ownerFont);

	ownerFont.GetHeight(&fh);
	
	BRect colorBox;
	
	float dist = (itemRect.Height()-fh.ascent)/2;
	
	colorBox.SetLeftTop(BPoint(itemRect.left+dist, itemRect.top+dist));
	colorBox.right  = colorBox.left + fh.ascent;
	colorBox.bottom = colorBox.top + fh.ascent;
	
	owner->SetHighColor(CColor::Black);
	owner->StrokeRect(colorBox);
	owner->SetHighColor(color);
	owner->FillRect(colorBox.InsetByCopy(1, 1));
	
	owner->SetHighColor(CColor::Black);
	owner->SetLowColor(owner->ViewColor());

	float str_x = colorBox.right + 4.0;
	float str_y = fh.ascent + 2.0 + itemRect.top;

	if(truncString == NULL) {
		truncString = new char[strlen(text)+3];
	
		ownerFont.GetTruncatedStrings((const char **)&text, 1, 
			B_TRUNCATE_END, Width()-2.0-str_x, &truncString);
	}

	owner->DrawString(truncString, BPoint(str_x, str_y));
}

void CColorSelectListItem::Update(BView *owner, const BFont *font)
{
	delete truncString;
	truncString = NULL;
	
	font_height fh;
	font->GetHeight(&fh);	

	SetWidth(owner->Bounds().Width());
	SetHeight(fh.ascent+fh.descent+4.0);
}