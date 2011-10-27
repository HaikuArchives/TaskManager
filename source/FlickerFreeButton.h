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

#ifndef FLICKER_FREE_BUTTON_H
#define FLICKER_FREE_BUTTON_H

class CFlickerFreeButton : public BButton
{
	public:
	CFlickerFreeButton(BRect frame,
		const char *name,
		const char *label,
		BMessage *message,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
		
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);

	virtual void FrameResized(float width, float height);
	
	protected:
	rgb_color bgColor;
};

#endif // FLICKER_FREE_BUTTON_H