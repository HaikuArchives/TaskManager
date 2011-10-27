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
 
#ifndef BORDER_VIEW_H
#define BORDER_VIEW_H

// ====== Archive Fields ======

// CBorderView 
extern const char * const BORDER_VIEW_ARCHIVE_BORDER_SIZE;			// int32
extern const char * const BORDER_VIEW_ARCHIVE_BG_COLOR;				// rgb_color
extern const char * const BORDER_VIEW_ARCHIVE_BORDER_STYLE;			// int32 (B_NO_BORDER, B_PLAIN_BORDER, B_FANCY_BORDER)

// ====== Class Defs ======

//: This class draws a border around a client area.
// the client area is drawn by derived classes in the
// DrawClient method.
// You can also embed an other view into this one, using
// the "decorator" pattern.
class _EXPORT CBorderView : public BView
{
	public:
	CBorderView(BRect frame, const char *name, int32 _borderSize=1, 
		uint32 resizingMode=B_FOLLOW_ALL, 
		uint32 flags=B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE,
		border_style _borderStyle=B_PLAIN_BORDER);
	CBorderView(BMessage *archive);
	virtual ~CBorderView();

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;

	virtual void DrawClient(BRect updateRect) {}
	
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void SetBgColor(rgb_color color);
	virtual rgb_color BgColor();
	virtual BRect ClientRect();
	
	protected:
	rgb_color bgColor;
	border_style borderStyle;
	int32 borderSize;
};

#endif // BORDER_VIEW_H