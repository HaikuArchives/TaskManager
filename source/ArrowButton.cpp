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
#include "ArrowButton.h"

CArrowButton::CArrowButton(BRect frame, const char *name, enumArrowDirection arrowDir,
	BMessage *message, uint32 resizingMode, uint32 flags) :
	CFlickerFreeButton(frame, name, "", message, resizingMode, flags)
{
	// The BButton constructor only creates buttons at least 22 pixels wide
	// and high. Resize button to match correct size.
	ResizeTo(frame.Width(), frame.Height());

	arrowDirection = arrowDir;
	
	highlighted = false;
}

void CArrowButton::Draw(BRect updateRect)
{
	const float dist = 7.0;
	const float focusFrameDist=4.0;

	if(IsFocusChanging()) {
		BRect windowRect = Bounds();

		if(IsFocus()) {
			// Draw blue focus frame
			SetHighColor(CColor::BeFocusBlue);
			StrokeRect(windowRect.InsetByCopy(focusFrameDist,focusFrameDist));
		} else {
			// Redraw orignal frame		
			SetHighColor(CColor::BeButtonGray);
			
			MovePenTo(BPoint(windowRect.left+focusFrameDist, windowRect.bottom-focusFrameDist));
			StrokeLine(BPoint(windowRect.right-focusFrameDist, windowRect.bottom-focusFrameDist));
			StrokeLine(BPoint(windowRect.right-focusFrameDist, windowRect.top+focusFrameDist));
	
			SetHighColor(CColor::BeHighlight);
			
			MovePenTo(BPoint(windowRect.left+focusFrameDist, windowRect.bottom-focusFrameDist));
			StrokeLine(BPoint(windowRect.left+focusFrameDist, windowRect.top+focusFrameDist));
			StrokeLine(BPoint(windowRect.right-focusFrameDist, windowRect.top+focusFrameDist));
		}
	} else {
		CFlickerFreeButton::Draw(updateRect);
		
		BPoint p[3];
		
		if(IsEnabled()) {
			if(IsHighlighted())
				SetHighColor(CColor::White);
			else
				SetHighColor(CColor::Black);
		} else
			SetHighColor(CColor::BeInactiveGray);
		
		switch(arrowDirection) {
			case ARROW_UP:
				p[0] = BPoint(Bounds().Width()/2, dist);
				p[1] = BPoint(Bounds().right-dist, Bounds().bottom-dist);
				p[2] = BPoint(dist, Bounds().bottom-dist);
				break;
			case ARROW_DOWN:
				p[0] = BPoint(Bounds().Width()/2, Bounds().bottom-dist);
				p[1] = BPoint(Bounds().right-dist, dist);
				p[2] = BPoint(dist, dist);
				break;
			case ARROW_LEFT:
				p[0] = BPoint(Bounds().right-dist, dist);
				p[1] = BPoint(dist, Bounds().Height()/2);
				p[2] = BPoint(Bounds().right-dist, Bounds().bottom-dist);
				break;
			case ARROW_RIGHT:
				p[0] = BPoint(dist, dist);
				p[1] = BPoint(Bounds().right-dist, Bounds().Height()/2);
				p[2] = BPoint(dist, Bounds().bottom-dist);
				break;
		}
		
		FillPolygon(p, 3);
		
		if(IsFocus() && !IsHighlighted()) {
			SetHighColor(CColor::BeFocusBlue);
			
			StrokeRect(Bounds().InsetByCopy(focusFrameDist,focusFrameDist));
		}
	}
}