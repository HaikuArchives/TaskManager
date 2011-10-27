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

#ifndef FLICKER_FREE_TAB_VIEW_H
#define FLICKER_FREE_TAB_VIEW_H

class CFlickerFreeTabView : public BTabView
{
	public:
	CFlickerFreeTabView(BRect frame,
		const char *name, 
		button_width width = B_WIDTH_AS_USUAL,
		uint32 resizingMode = B_FOLLOW_ALL,
		uint32 flags = B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW | 
			B_NAVIGABLE_JUMP | B_FRAME_EVENTS | B_NAVIGABLE) :
		BTabView(frame, name, width, resizingMode, flags) {}
		
	virtual void AttachedToWindow()
	{
		BTabView::AttachedToWindow();
		SetViewColor(CColor::Transparent);
	}
	
	virtual void Draw(BRect updateRect)
	{
		SetLowColor(CColor::BeBackgroundGray);
		FillRect(updateRect, B_SOLID_LOW);

		SetViewColor(CColor::BeBackgroundGray);
		BTabView::Draw(updateRect);
		SetViewColor(CColor::Transparent);
	}
};

#endif // FLICKER_FREE_TAB_VIEW_H