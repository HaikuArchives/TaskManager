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

#ifndef ICON_STRING_ITEM_H
#define ICON_STRING_ITEM_H

class CIconStringItem : public BStringItem
{
	public:
	CIconStringItem(BBitmap *bitmap, const char *text, 
		uint32 level=0, bool expanded=true) :
		BStringItem(text, level, expanded)
	{
		highlightColor.red	 = 216;
		highlightColor.green = 216;
		highlightColor.blue  = 216;
		highlightColor.alpha = 255;
		
		icon = bitmap; 
	}
		
	virtual ~CIconStringItem() { delete icon; }

	virtual void Update(BView *owner, const BFont *font)
	{
		BStringItem::Update(owner, font);
	
		SetHeight(MAX(Height(), icon->Bounds().Height()+4));
	}
	
	virtual void DrawItem(BView *owner, BRect itemRect, bool drawEverything=false)
	{
		rgb_color black = { 0, 0, 0, 255 };
		
		if(IsSelected()) {
			owner->SetHighColor(highlightColor);
			owner->FillRect(itemRect);
			owner->SetLowColor(highlightColor);
		} else {
			if(drawEverything) {
				owner->SetHighColor(owner->ViewColor());
				owner->FillRect(itemRect);
				owner->SetLowColor(owner->ViewColor());
			}
		}
		
		BFont *viewFont = new BFont();
		
		owner->GetFont(viewFont);
		
		font_height fontHeight; 
		
		viewFont->GetHeight(&fontHeight);

		owner->SetHighColor(black);
		owner->MovePenTo(icon->Bounds().Width()+8, 
						 itemRect.bottom - (itemRect.Height() - fontHeight.ascent)/2);
		owner->DrawString(Text());
		
		BPoint bitmapPos;
		
		bitmapPos.x = 4;
		bitmapPos.y = itemRect.top + (itemRect.Height() - icon->Bounds().Height()) / 2;
		
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmap(icon, bitmapPos);
		owner->SetDrawingMode(B_OP_COPY);
	}
	
	protected:
	rgb_color highlightColor;
	BBitmap *icon;
};

#endif // ICON_STRING_ITEM_H