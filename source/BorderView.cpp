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
#include "signature.h"
#include "BorderView.h"
#include "msg_helper.h"

// archive fields
const char * const BORDER_VIEW_ARCHIVE_BORDER_SIZE		= "BORDERVIEW:BorderSize";
const char * const BORDER_VIEW_ARCHIVE_BG_COLOR			= "BORDERVIEW:BgColor";
const char * const BORDER_VIEW_ARCHIVE_BORDER_STYLE		= "BORDERVIEW:BorderStyle";

CBorderView::CBorderView(BRect frame, const char *name,	int32 _borderSize,
	uint32 resizingMode, uint32 flags, border_style _borderStyle) :
	BView(frame, name, resizingMode, flags),
	borderStyle(_borderStyle), borderSize(_borderSize)
{
	bgColor = CColor::Transparent;
}

CBorderView::CBorderView(BMessage *archive) : 
	BView(archive)
{
	if(archive->FindInt32(BORDER_VIEW_ARCHIVE_BORDER_SIZE, &borderSize) != B_OK) {
		borderSize = 0;
	}

	if(archive->FindInt32(BORDER_VIEW_ARCHIVE_BORDER_STYLE, (int32 *)&borderStyle) != B_OK) {
		borderStyle = B_PLAIN_BORDER;
	}
	
	bgColor = FindColor(archive, BORDER_VIEW_ARCHIVE_BG_COLOR);
}

status_t CBorderView::Archive(BMessage *data, bool deep) const
{
	status_t status = BView::Archive(data, deep);

	if(status == B_OK) {
		data->AddString("add_on", APP_SIGNATURE);
		data->AddInt32(BORDER_VIEW_ARCHIVE_BORDER_SIZE, borderSize);
		data->AddInt32(BORDER_VIEW_ARCHIVE_BORDER_STYLE, borderStyle);
		data->AddData(BORDER_VIEW_ARCHIVE_BG_COLOR, B_RGB_COLOR_TYPE, &bgColor, sizeof(rgb_color));
	}

	return status;
}

BArchivable *CBorderView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CBorderView"))
		return NULL;
		
	return new CBorderView(archive);
}

CBorderView::~CBorderView()
{
}

void CBorderView::AttachedToWindow()
{
	SetViewColor(CColor::Transparent);
}

void CBorderView::Draw(BRect updateRect)
{
	// --- Draw border

	float windowWidth  = Bounds().Width();
	float windowHeight = Bounds().Height();

	switch(borderStyle) {
		case B_PLAIN_BORDER:
			for(int i=0 ; i<borderSize ; i++) {
				SetHighColor(CColor::BeShadow);
				MovePenTo(BPoint(windowWidth-i, i));
				StrokeLine(BPoint(i, i));
				StrokeLine(BPoint(i, windowHeight-i));
			
				SetHighColor(CColor::BeHighlight);
				MovePenTo(BPoint(windowWidth-i, i+1));
				StrokeLine(BPoint(windowWidth-i, windowHeight-i));
				StrokeLine(BPoint(i+1, windowHeight-i));
			}
			break;
		case B_FANCY_BORDER:
			for(int i=0 ; i<borderSize ; i++) {
				rgb_color light, shadow;
			
				if(i == 0) {
					light  = CColor::BeHighlight;
					shadow = CColor::BeShadow;
				} else {
					light  = CColor::BeLightShadow;
					shadow = CColor::BeDarkShadow;
				}
				
				SetHighColor(shadow);
				MovePenTo(BPoint(windowWidth-i, i));
				StrokeLine(BPoint(i, i));
				StrokeLine(BPoint(i, windowHeight-i));
			
				SetHighColor(light);
				MovePenTo(BPoint(windowWidth-i, i+1));
				StrokeLine(BPoint(windowWidth-i, windowHeight-i));
				StrokeLine(BPoint(i+1, windowHeight-i));
			}
			break;
		case B_NO_BORDER:
			// nothing to do.
			break;
	}


	// --- Fill background

	if(bgColor != CColor::Transparent) {
		SetHighColor(bgColor);
		FillRect(ClientRect());
	}
	
	// --- Draw client area

	BRect clientRect = ClientRect();

	BRegion clientRegion;
	clientRegion.Set(clientRect);
	
	// only permitt drawing inside the client area
	ConstrainClippingRegion(&clientRegion);

	DrawClient(updateRect & clientRect);
}

void CBorderView::SetBgColor(rgb_color color)
{
	bgColor = color;
}

rgb_color CBorderView::BgColor()
{
	return bgColor;
}

BRect CBorderView::ClientRect()
{
	if(borderStyle != B_NO_BORDER)
		return BRect(borderSize, borderSize, Bounds().Width()-borderSize, Bounds().Height()-borderSize);
	else
		return Bounds();
}
