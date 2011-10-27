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
 
#ifndef TM_BOX_H
#define TM_BOX_H

class CBox : public BBox
{
	public:
	CBox(BRect frame, const char *name, 
		uint32 resizingMode = B_FOLLOW_NONE, 
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
		border_style border = B_FANCY_BORDER) :
		BBox(frame, name, resizingMode, flags, border) {}
		
	CBox(BMessage *archive) : BBox(archive) {}
	
	virtual float LabelHeight()
	{
		if(Label() != NULL) {
			font_height height;
		
			be_bold_font->GetHeight(&height);
		
			return height.ascent;
		} else if(LabelView() != NULL) {
			return LabelView()->Bounds().Height();
		} 
		
		return BorderSize();
	}
	
	virtual float BorderSize()
	{
		switch(Border()) {
			case B_PLAIN_BORDER:
				return 1;
			case B_FANCY_BORDER:
				return 2;
			case B_NO_BORDER:
			default:
				return 0;
		}
	}
	
	virtual BRect ClientRect()
	{
		return BRect(BorderSize(),
					 LabelHeight(),
					 Bounds().Width()-BorderSize(),
					 Bounds().Height()-BorderSize());
	}
};

#endif // TM_BOX_H