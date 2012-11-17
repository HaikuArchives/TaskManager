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
#include "help.h"
#include "URLTextView.h"

#include "my_assert.h"

// ==== globals ====

// list of URL prefixes
const char * const urlPrefixList[] = {
	"http://",
	"file://",
	"mailto:",
	""
};

// BeOS hand cursor with a little arrow.
// Like the one that Net+ displays when the mouse 
// hovers over a link.
const unsigned char url_hover_cursor[] =
{
	16,			// width and height
	1,			// bitplanes
	1, 0,		// hotspot
	
	0x70, 0x00,	// bitmap data
	0x48, 0x00,
	0x48, 0x00,
	0x27, 0xC0,
	0x24, 0xB8,
	0x12, 0x54,
	0x10, 0x02,
	0x78, 0x42,
	0x98, 0xE2,
	0x81, 0xF2,
	0x63, 0xFA,
	0x18, 0x02,
	0x04, 0x00,
	0x02, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	
	0x70, 0x00,	// mask
	0x78, 0x00,
	0x78, 0x00,
	0x3F, 0xC0,
	0x3F, 0xF8,
	0x1F, 0xFC,
	0x1F, 0xFE,
	0x7F, 0xFE,
	0xFF, 0xFE,
	0xFF, 0xFE,
	0x7F, 0xFE,
	0x1F, 0xFE,
	0x07, 0xF0,
	0x03, 0x80,
	0x00, 0x00,
	0x00, 0x00,
};

// ==== CURLTextView ====

CURLTextView::CURLTextView(BRect frame, const char *name, BRect textRect,
	uint32 resizingMode, uint32 flags) :
	BTextView(frame, name, textRect, resizingMode, flags)
{
	SetFontAndColor(be_plain_font, B_FONT_ALL, &CColor::Black);
	SetStylable(true);
	MakeEditable(false);
}

void CURLTextView::MouseMoved(BPoint point, uint32 transit, 
	const BMessage *message)
{
	bool overURL = false;

	if(transit == B_INSIDE_VIEW) {
		for(int i=0 ; i<urlInfoList.CountItems() ; i++) {
			if(urlInfoList.ItemAt(i)->Contains(this, point)) {
				// Mouse is over a link. Display "URL hover cursor".
				// That cursor is identical to the BeOS hand cursor
				// but contains a little arrow (like in Net+).
#if B_BEOS_VERSION >= B_BEOS_VERSION_5
				const BCursor URLHoverCursor((const void *)url_hover_cursor);
				
				SetViewCursor(&URLHoverCursor);
#else
				be_app->SetCursor((const void *)url_hover_cursor);
#endif
				
				overURL = true;
				
				break;
			}
		}
		
#if B_BEOS_VERSION >= B_BEOS_VERSION_5
		if(!overURL) {
			SetViewCursor(B_CURSOR_I_BEAM);
		}
#endif
	}
	
	if(!overURL)
		BTextView::MouseMoved(point, transit, message);
}

void CURLTextView::MouseDown(BPoint point)
{
	for(int i=0 ; i<urlInfoList.CountItems() ; i++) {
		if(urlInfoList.ItemAt(i)->Contains(this, point)) {
			// User clicked on an anchor. Open webbrowser or
			// eMail client.
			urlInfoList.ItemAt(i)->Open(this);
			return;
		}
	}
	
	BTextView::MouseDown(point);
}

void CURLTextView::AddText(const char *text)
{
	int32 pos = strlen(Text());
	
	// Add text on the end.
	Insert(pos, text, strlen(text), NULL);
}

void CURLTextView::AddAnchor(const char *anchorText, const char *href)
{
	int32 pos = strlen(Text());
	
	// Add anchor on the end
	InsertAnchor(anchorText, href, pos);
}


void CURLTextView::InsertAnchor(const char *anchorText, const char *href, 
	int32 offset)
{
	CPointerList<CTextRun> textRuns;
	
	rgb_color anchorColor;
	BFont anchorFont;

	GetAnchorInfo(anchorFont, anchorColor);
	
	textRuns.AddItem(new CTextRun(0, anchorFont, anchorColor));
	textRuns.AddItem(new CTextRun(strlen(anchorText), be_plain_font, CColor::Black));

	text_run_array *textRunArray = CreateTextRunArray(textRuns);

	Insert(offset, anchorText, strlen(anchorText), textRunArray);
	
	delete textRunArray;
	
	urlInfoList.AddItem(new CURLInfo(offset, strlen(anchorText)+offset, href));
}

void CURLTextView::GetAnchorInfo(BFont &font, rgb_color &color)
{
	// color for anchor
	color.red	= 0;
	color.green	= 0;
	color.blue	= 0xFF;
	color.alpha	= 0xFF;

	// font for anchor
	font = be_plain_font;
	font.SetFace(B_UNDERSCORE_FACE | font.Face());
}

void CURLTextView::InsertText(const char *text, int32 length,
	int32 offset, const text_run_array *runs)
{
	// Called whenever text is inserted in the text view.
	// If data is passed in the 'runs' array, the first entry
	// is used (if it starts with offset 0) as inital color.
	// All other entries are ignored.
	// It also ignores the current font and color settings.
	// Normal text is always display in black, be_plain_font.
	// URLs are always displayed in blue, be_plain_font.

	// List of text_run structures.
	CPointerList<CTextRun> urlRuns;

	rgb_color urlColor;
	BFont urlFont;

	GetAnchorInfo(urlFont, urlColor);

	if(runs) {
		// A text_run_array was specified. Copy first entry if
		// it specifies the settings for the first char.
		if(runs->runs[0].offset == 0) {
			urlRuns.AddItem(new CTextRun(runs->runs[0]));
		}
	} else {
		// The first run must begin with offset 0.
		urlRuns.AddItem(new CTextRun(0, be_plain_font, CColor::Black));
	}

	int32 wordStart = 0;	// start of the last word
	bool wordEnd = false;	// reached end of word??

	for(int i=0 ; i<length ; i++) {
		switch(text[i]) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case ')':
			case '(':
				wordEnd = true;
		}

		bool isLastChar = (i == length-1);
		
		if(wordEnd || isLastChar) {
			if(i - wordStart >= 1) {
				enumUrlType urlType = CURLInfo::URLType(text, wordStart, i);
			
				if(urlType != URL_TYPE_NONE) {
					// We have found an URL.

					// Switch display to URLColor and URLFont. 
					urlRuns.AddItem(new CTextRun(wordStart, urlFont, urlColor));
					
					// Reset to default font and color.
					urlRuns.AddItem(new CTextRun(i, be_plain_font, CColor::Black));
					
					// Create URL info object.
					urlInfoList.AddItem(new CURLInfo(offset+wordStart, offset+i, urlType));
				}
			}
		
			wordStart = i+1;
			wordEnd = false;
		}
	}

	// Copy the data in the urlRuns list into a text_run_array.
	text_run_array *urlRunArray = CreateTextRunArray(urlRuns);
	
	// Now insert the text.
	BTextView::InsertText(text, length, offset, urlRunArray);
		
	delete urlRunArray;
}

text_run_array *CURLTextView::CreateTextRunArray(const BList &textRuns)
{
	// Copy the data in the textRuns list into a text_run_array.
	
	int32 count = textRuns.CountItems();

	// The different text_runs directly follow the text_run_array header.
	// (text_run_array::runs isn't a pointer. It's simply a placeholder for
	// easy access).
	text_run_array *textRunArray = 
		(text_run_array *)new uchar [sizeof(text_run_array) + sizeof(text_run) * (count-1)];

	textRunArray->count = count;

	text_run *start = (text_run *)(&(textRunArray->runs));

	for(int i=0 ; i<count ; i++) {
		memcpy(&start[i], textRuns.ItemAt(i), sizeof(text_run));
	}
	
	return textRunArray;
}

// ==== CURLTextView::CURLInfo ====

// static function
enumUrlType CURLTextView::CURLInfo::URLType(const char *text, int32 start, int32 end)
{
	// returns:
	// - URL_TYPE_NONE if the text between start and end isn't an URL.
	// - URL_TYPE_MAIL if the text begins with mailto:
	// - URL_TYPE_HTTP if the text begins with http://

	int32 length = end-start;

	for(int prefixNum = 0 ; urlPrefixList[prefixNum][0] != '\0' ; prefixNum++) {
		const char *urlPrefix = urlPrefixList[prefixNum];
	
		int32 prefixLen = strlen(urlPrefix);
	
		if(prefixLen < length && strncmp(&text[start], urlPrefix, prefixLen) == 0) {
			// is a good candidate. Test for invalid chars
			for(int32 c=start ; c<end ; c++) {
				if(!isalnum(text[c])) {
					switch(text[c]) {
						case '/':
						case '_':
						case '-':
						case '.':
						case '%':
						case '@':
						case '?':
						case '=':
						case ':':
							// valid URL char
							break;
						default:
							// invalid url char
							return URL_TYPE_NONE;
					}
				}
			}
			
			switch(prefixNum) {
				case 0:
					return URL_TYPE_HTTP;
				case 1:
					return URL_TYPE_FILE;
				case 2:
					return URL_TYPE_MAIL;
				default:
					MY_ASSERT(!"invalid prefixNum");
			}
		}
	}
	
	return URL_TYPE_NONE;
}

CURLTextView::CURLInfo::CURLInfo(int32 _start, int32 _end, const char *_href) :
	start(_start), end(_end)
{
	href = new char [strlen(_href)+1];
	
	strcpy(href, _href);
	
	type = URLType(href, 0, strlen(href));
}

CURLTextView::CURLInfo::CURLInfo(int32 _start, int32 _end, enumUrlType _type) :
	start(_start), end(_end), type(_type)
{
	href = NULL;
}

CURLTextView::CURLInfo::~CURLInfo()
{
	if(href)
		delete [] href;
}

void CURLTextView::CURLInfo::Open(CURLTextView *view)
{
	// Open this URL.

	char *argv[1];
	
	char *url = NULL;
	
	if(href) {
		url = new char[strlen(href)+1];
		
		strcpy(url, href);
	} else {
		url = new char[end-start+1];

		// Copy URL string from TextView.
		view->GetText(start, end-start, url);
	}

	MY_ASSERT(url != NULL);
	
	argv[0] = url;

	BString appMime="";
	
	if(type == URL_TYPE_MAIL) { 
		// Mail URL. Open email client.
		appMime = "application/x-vnd.Be-MAIL";
	} else if(type == URL_TYPE_HTTP || type == URL_TYPE_FILE) {
		// Website. Open NetPositive.
		appMime = get_preferred_browser();
	} else {
		// Error
		MY_ASSERT(!"Invalid URL type");
	}

	// Launch webbrowser or email client.
	be_roster->Launch(appMime.String(), 1, argv, NULL);
	
	delete url;	
}

bool CURLTextView::CURLInfo::Contains(CURLTextView *view, BPoint point)
{
	// Returns true, if the point is over this URL.
	
	BRegion region;

	view->GetTextRegion(start, end, &region);
	
	return region.Contains(point);
}

// ==== CURLTextView::CTextRun ====

CURLTextView::CTextRun::CTextRun(int32 _offset, BFont _font, rgb_color _color) 
{
	offset	= _offset;
	font	= _font;
	color	= _color;
}
