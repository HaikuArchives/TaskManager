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

#ifndef URL_TEXT_VIEW_H
#define URL_TEXT_VIEW_H

#include "PointerList.h"

enum enumUrlType {
	URL_TYPE_NONE,		// not an url
	URL_TYPE_MAIL,		// mailto:...
	URL_TYPE_HTTP,		// http://...
	URL_TYPE_FILE,		// file://...
};

extern const unsigned char url_hover_cursor[];

// Very simple class which displays URLs blue and (if the
// right font exists) underlined. It's only useful for
// static views. So don't expect the class to detect an
// URL during typing.
class CURLTextView : public BTextView
{
	public:
	CURLTextView(BRect frame, const char *name, BRect textRect,
					uint32 resizingMode, uint32 flags);

	virtual void MouseMoved(BPoint point, uint32 transit, 
					const BMessage *message);
	virtual void MouseDown(BPoint point);

	virtual void InsertAnchor(const char *anchorText, 
					const char *href, int32 offset);

	void AddText(const char *text);
	void AddAnchor(const char *anchorText, const char *href);
	
	protected:
	// This class holds information about an URL. It holds the
	// position of the anchor inside the text view and the
	// href. If href is NULL, than the text of the anchor also
	// contains the href.
	class CURLInfo
	{
		public:
		CURLInfo(int32 _start, int32 _end, const char *href);
		CURLInfo(int32 _start, int32 _end, enumUrlType _type);
		~CURLInfo();
		
		void Open(CURLTextView *view);
		bool Contains(CURLTextView *view, BPoint point);
		
		static enumUrlType URLType(const char *text, int32 start, int32 end);
		
		protected:
		int32 start, end;	// start and end index inside the text view
		enumUrlType type;
		char *href;			// Can be NULL. Then the anchor is identical to href.
	};
	
	// simple wrapper class for text_run
	class CTextRun : public text_run
	{
		public:
		CTextRun(const text_run &other) { (text_run &)(*this) = other; }
		CTextRun(int32 _offset, BFont _font, rgb_color _color);
	};

	// Automatically detects URLs inside the text and displays
	// them blue and underlined.
	virtual void InsertText(const char *text, int32 length,
					int32 offset, const text_run_array *runs);
	
	virtual void GetAnchorInfo(BFont &font, rgb_color &color);

	// Create a text_run_array struct from a list of text_run
	// structs. The text_run_array belongs to the caller.
	text_run_array *CreateTextRunArray(const BList &textRuns);
					
	CPointerList<CURLInfo> urlInfoList;
};

#endif // URL_TEXT_VIEW_H