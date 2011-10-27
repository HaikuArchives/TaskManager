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
#include "BugfixedDragger.h"

CBugfixedDragger::CBugfixedDragger(BRect frame, BView *target, uint32 resizingMode) :
	BDragger(frame, target, resizingMode)
{
	bgColor = B_TRANSPARENT_COLOR;
}

CBugfixedDragger::CBugfixedDragger(BMessage *archive) :
	BDragger(archive)
{
	bgColor = B_TRANSPARENT_COLOR;
}

BArchivable *CBugfixedDragger::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CBugfixedDragger"))
		return NULL;
		
	return new CBugfixedDragger(archive);
}

status_t CBugfixedDragger::Archive(BMessage *data, bool deep) const
{
	status_t status = BDragger::Archive(data, deep);

	// pretend to be a BDragger. Some baseclass uses RTTI to add the
	// class name to the message. So "class" gets "CBugfixedDragger".
	// This seems to confuse the tacker (but it works with a normal
	// BShelf!).
	data->ReplaceString("class", "BDragger");

	return status;
}

void CBugfixedDragger::AttachedToWindow()
{
	BDragger::AttachedToWindow();

	// store background color
	bgColor = Parent()->ViewColor();
}

void CBugfixedDragger::Draw(BRect updateRect)
{
	// The normal BDragger uses B_TRANSPARENT_COLOR as view color.
	// So the area not covered by the handle icon stays untouched.
	// This works well if the dragger is a child of the target
	// view and B_DRAW_ON_CHILDREN is set for the target.
	// It doesn't work if the dragger is a sibling of the target,
	// because the area not covered by the handle is NEVER redrawn.
	// This bug is fixed by this code.
	
	SetHighColor(bgColor);
	FillRect(BRect(3, 0, Bounds().Width(), Bounds().Height()));
	FillRect(BRect(0, 3, 2, Bounds().Height()));

	BDragger::Draw(updateRect);
}
