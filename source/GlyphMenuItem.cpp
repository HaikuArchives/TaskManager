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
#include "GlyphMenuItem.h"

//: Constructor
// The CGlyphMenuItem takes the normal BMenuItem constructor
// parameters and the additional parameters mentioned in the parameter
// section.
//!param: glyph_bm - Icon displayed in front of the label. This class
//!param: takes responsibilty for the passed object. The caller
//!param: should not modify the bitmap after it was passed to
//!param: this method.
//!param: isDefaultEntry - If set to true, the label is displayed 
//!param: using a bold font.
//!param: _offset - Offset of the label from the default location.
CGlyphMenuItem::CGlyphMenuItem(const char *label, BBitmap *glyph_bm,
	bool isDefaultEntry, BMessage *message, char shortcut, 
	uint32 modifiers, BPoint _offset) :
	BMenuItem(label, message, shortcut, modifiers)
{
	glyph		= glyph_bm;
	isDefault	= isDefaultEntry;
	offset      = _offset;
}

CGlyphMenuItem::~CGlyphMenuItem()
{
	if(glyph) delete glyph;
}

//: BeOS hook function
void CGlyphMenuItem::DrawContent()
{
	BPoint pos = ContentLocation();

	pos += offset;

	BFont oldFont = SetDrawFont();

	if(glyph) {
		Menu()->SetDrawingMode(B_OP_COPY);
		Menu()->MovePenTo(pos.x+glyph->Bounds().Width()+5.0, pos.y);
		BMenuItem::DrawContent();
	
		Menu()->SetDrawingMode(B_OP_OVER);
		Menu()->DrawBitmap(glyph, pos);
	} else {
		BMenuItem::DrawContent();
	}
	
	Menu()->SetFont(&oldFont);
}

//: BeOS hook function
void CGlyphMenuItem::GetContentSize(float *width, float *height)
{
	BFont oldFont;
	
	oldFont = SetDrawFont();

	BMenuItem::GetContentSize(width, height);

	Menu()->SetFont(&oldFont);
	
	if(glyph) {
		*width += glyph->Bounds().Width() + 5.0 + offset.x;
		*height = MAX(*height, glyph->Bounds().Height()) + offset.y;
	}
}

//: Sets the correct drawing font
// This method sets the correct drawing font for the menu.
// It returns the previously selected font.
BFont CGlyphMenuItem::SetDrawFont()
{
	BFont oldFont;
	
	Menu()->GetFont(&oldFont);

	if(isDefault) {
		BFont boldFont = oldFont;
		
		boldFont.SetFace(B_BOLD_FACE);
		
		Menu()->SetFont(&boldFont);
	}
	
	return oldFont;
}