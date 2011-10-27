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
#include "FlickerFreeButton.h"

CFlickerFreeButton::CFlickerFreeButton(BRect frame,
	const char *name,
	const char *label,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags) :
	BButton(frame, name, label, message, resizingMode,
		flags | B_FRAME_EVENTS)
{
	bgColor = CColor::Black;
}

void CFlickerFreeButton::AttachedToWindow()
{
	BButton::AttachedToWindow();

	// The Dano buttons are flicker free anyway.
	// So don't handle the background drawing on Dano.
#if B_BEOS_VERSION < B_BEOS_VERSION_DANO
	if(Parent())
		bgColor = Parent()->ViewColor();
		
	SetViewColor(CColor::Transparent);
#endif // B_BEOS_VERSION_DANO
}

void CFlickerFreeButton::Draw(BRect updateRect)
{
#if B_BEOS_VERSION < B_BEOS_VERSION_DANO
	SetHighColor(bgColor);
	SetLowColor(bgColor);
	StrokeRect(updateRect, B_SOLID_LOW);
#endif // B_BEOS_VERSION_DANO

	BButton::Draw(updateRect);
}

void CFlickerFreeButton::FrameResized(float width, float height)
{
#if B_BEOS_VERSION < B_BEOS_VERSION_DANO
	SetHighColor(bgColor);
	SetLowColor(bgColor);
	Draw(BRect(0, 0, width, height));
#endif // B_BEOS_VERSION_DANO

	BButton::FrameResized(width, height);
}
