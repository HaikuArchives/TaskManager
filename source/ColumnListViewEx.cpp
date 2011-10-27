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
#include "my_assert.h"
#include "msg_helper.h"
#include "Detector.h"

#include "ColumnListViewEx.h"

// ====== globals ======

const char * const COLUMN_LIST_VIEW_PROP_COLUMN				= "Column";
const char * const COLUMN_LIST_VIEW_PROP_COLUMN_VISIBLE		= "Visible";
const char * const COLUMN_LIST_VIEW_PROP_COLUMN_WIDTH		= "Width";
const char * const COLUMN_LIST_VIEW_PROP_COLUMN_LABEL		= "Label";
const char * const COLUMN_LIST_VIEW_PROP_COLUMN_SORT_MODE	= "SortMode";

// ====== CLVTextPainter ======

CLVTextPainter::CLVTextPainter(const char *t, bool _rightAlign,
	const rgb_color &_textColor)
{
	text = new char [strlen(t)+1];
	strcpy(text, t);
	
	rightAlign	= _rightAlign;
	textColor	= _textColor;
	
	truncText = NULL;
	
	mustTruncate = true;
}

CLVTextPainter::~CLVTextPainter()
{
	if(text)		delete [] text;
	if(truncText)	delete [] truncText;
}

void CLVTextPainter::DrawItemColumn(BView *owner, BRect item_column_rect, bool complete)
{
	BFont owner_font;
	owner->GetFont(&owner_font);

	font_height fontAttributes;
	owner_font.GetHeight(&fontAttributes);
	float fontHeight = ceil(fontAttributes.ascent) + ceil(fontAttributes.descent);
	float text_offset = ceil(fontAttributes.ascent) + (item_column_rect.Height()-fontHeight)/2.0;

	float text_width = owner_font.StringWidth(text);
	
	owner->SetHighColor(textColor);
	
	if(text_width+4 > item_column_rect.Width()) {
		// Normal string is too long. Display truncated string.
		if(mustTruncate) {
			// mustTruncate is set when the column width changes.
			if(truncText)
				delete [] truncText;
	
			truncText = new char [strlen(text)+3];
	
			owner_font.GetTruncatedStrings((const char **)&text, 1, rightAlign ? B_TRUNCATE_BEGINNING : B_TRUNCATE_END, 
							item_column_rect.Width()-4, &truncText); 
							
			mustTruncate = false;
		}
		
		if(rightAlign) {
			text_width = owner_font.StringWidth(truncText);
			
			owner->DrawString(truncText,BPoint(item_column_rect.right-text_width-2.0,item_column_rect.top+text_offset));
		} else
			owner->DrawString(truncText,BPoint(item_column_rect.left+2.0,item_column_rect.top+text_offset));
				
	} else {
		// Normal (non-truncated) string fits into the column.
		if(rightAlign)
			owner->DrawString(text,BPoint(item_column_rect.right-text_width-2.0,item_column_rect.top+text_offset));
		else
			owner->DrawString(text,BPoint(item_column_rect.left+2.0,item_column_rect.top+text_offset));	
	}
}

void CLVTextPainter::ColumnWidthChanged(float column_width, ColumnListView* the_view)
{
	mustTruncate = true;
}

const char *CLVTextPainter::Text() const
{
	return text;
}

void CLVTextPainter::SetText(const char *t)
{
	if(text) delete [] text;
	
	text = new char [strlen(t)+1];
	strcpy(text, t);
	
	mustTruncate = true;
	
	// don't delete truncated string. This is done during truncation.
}

float CLVTextPainter::ContentWidth(BView *owner, BFont *font)
{
	// return (untruncated) width of text
	return font->StringWidth(text);
}

// ====== CLVEasyItemEx ======

CLVEasyItemEx::CLVEasyItemEx(uint32 level, bool superitem, bool expanded, float minheight) :
	CLVEasyItem(level, superitem, expanded, minheight)
{
}

CLVEasyItemEx::~CLVEasyItemEx()
{
	int num_columns = m_column_types.CountItems();

	// delete painters
	for(int column = 0; column < num_columns; column++)
	{
		if((int32)m_column_types.ItemAt(column) == CLVColPainter)
			delete (CLVPainter *)(m_column_content.ItemAt(column));
	}
}

void CLVEasyItemEx::ColumnWidthChanged(int32 column_index, float column_width, ColumnListView* the_view)
{
	CLVPainter *painter = GetColumnContentPainter(column_index);

	if(painter) {
		// This column contains a painter. Notify painter of change.
	
		BRect* cached_rect = (BRect*)m_cached_rects.ItemAt(column_index);
		
		if(cached_rect == NULL || *cached_rect == BRect(-1,-1,-1,-1))
			return;
			
		float width_delta = column_width-(cached_rect->right-cached_rect->left);		
		cached_rect->right += width_delta;
	
		int num_columns = m_cached_rects.CountItems();
		for(int column = 0; column < num_columns; column++) {
			if(column != column_index) {
				BRect* other_rect = (BRect*)m_cached_rects.ItemAt(column);
				if(other_rect->left > cached_rect->left)
					other_rect->OffsetBy(width_delta,0);
			}
		}

		painter->ColumnWidthChanged(column_width, the_view);
				
		the_view->Invalidate(*cached_rect);
	} else {
		CLVEasyItem::ColumnWidthChanged(column_index, column_width, the_view);
	}
}

void CLVEasyItemEx::FrameChanged(int32 column_index, BRect new_frame, ColumnListView *the_view)
{
	CLVPainter *painter = GetColumnContentPainter(column_index);

	if(painter)
		painter->FrameChanged(new_frame, the_view);
		
	CLVEasyItem::FrameChanged(column_index, new_frame, the_view);
}

void CLVEasyItemEx::DrawItemColumnInner(BView* owner, BRect item_column_rect, int32 column_index, bool complete)
{
	CLVPainter *painter = GetColumnContentPainter(column_index);

	if(painter) {
		painter->DrawItemColumn(owner, item_column_rect, complete);
	} else {
		CLVEasyItem::DrawItemColumnInner(owner, item_column_rect, column_index, complete);
	}
}

void CLVEasyItemEx::SetColumnContent(int column_index, CLVPainter *newPainter)
{
	bool delete_old = (m_column_types.CountItems() >= column_index-1);
	
	if(delete_old && GetColumnContentPainter(column_index) != NULL) {
		delete GetColumnContentPainter(column_index);
	} 

	PrepListsForSet(column_index);

	((int32*)m_column_types.Items())[column_index] = CLVColPainter;
	((const CLVPainter **)m_column_content.Items())[column_index] = newPainter;
	((char**)m_aux_content.Items())[column_index] = NULL;
}

void CLVEasyItemEx::SetColumnContent(int column_index, const char *text, bool truncate)
{
	CLVTextPainter *textPainter = NULL;

	if(m_column_types.CountItems() > column_index && 
		(int32)m_column_types.ItemAt(column_index) == CLVColPainter) {
		textPainter = dynamic_cast<CLVTextPainter *>(GetColumnContentPainter(column_index));
	}
	
	if(textPainter) {
		textPainter->SetText(text);
	} else {
		CLVEasyItem::SetColumnContent(column_index, text, truncate);
	}
}

void CLVEasyItemEx::SetColumnContent(int column_index, const BBitmap *bitmap, float horizontal_offset, bool copy)
{
	CLVEasyItem::SetColumnContent(column_index, bitmap, horizontal_offset, copy);
}

CLVPainter *CLVEasyItemEx::GetColumnContentPainter(int column_index) const
{
	if((int32)m_column_types.ItemAt(column_index) == CLVColPainter) {
		return static_cast<CLVPainter *>((m_column_content.ItemAt(column_index)));
	}
	
	return NULL;
}

float CLVEasyItemEx::GetColumnContentWidth(BView *owner, BFont *font, int column_index) const
{
	if((int32)m_column_types.ItemAt(column_index) == CLVColPainter) {
		return GetColumnContentPainter(column_index)->ContentWidth(owner, font);
	}
	
	return CLVEasyItem::GetColumnContentWidth(owner, font, column_index);
}

const char* CLVEasyItemEx::GetColumnContentText(int column_index) const
{
	CLVTextPainter *textPainter = NULL;

	if(m_column_types.CountItems() > column_index && 
		(int32)m_column_types.ItemAt(column_index) == CLVColPainter) {
		textPainter = dynamic_cast<CLVTextPainter *>(GetColumnContentPainter(column_index));
	}
	
	if(textPainter) {
		return textPainter->Text();
	} else {
		return CLVEasyItem::GetColumnContentText(column_index);
	}
}

void CLVEasyItemEx::DisplayContextMenu(BView *owner, BPoint point)
{
}

// ====== CColumnListViewEx ======

CColumnListViewEx::CColumnListViewEx(
	BRect Frame, 
	CLVContainerView** ContainerView,						
	const char* Name,
	uint32 ResizingMode,
	uint32 flags,
	list_view_type Type,
	bool hierarchical,
	bool horizontal,					
	bool vertical,
	bool scroll_view_corner,
	border_style border,
	const BFont* LabelFont) :
ColumnListView(Frame, ContainerView, Name, ResizingMode, 
	flags, Type, hierarchical, horizontal, vertical,
	scroll_view_corner, border, LabelFont)
{
	offscreenBuffer = NULL;
	
	oldWidth = oldHeight = 0.0;
	
	detector = NULL;
}

CColumnListViewEx::~CColumnListViewEx()
{
	// Delete all items
	// This makes the CColumnListViewEx incompatible to BListView
	// (which doesn't delete the entries). But I don't want to worry
	// about the enties during shutdown.
	int32 ItemCount = CountItems();
	for(int32 Counter = 0 ; Counter < ItemCount ; Counter++)
	{
		delete ItemAt(Counter);
	}
	
	delete detector;
	delete offscreenBuffer;
}

BRect CColumnListViewEx::ColumnRect(int columnIndex) const
{
	float left  = ColumnAt(columnIndex)->Begin();
	float right = ColumnAt(columnIndex)->End();
		
	return BRect(left, 0, right, Bounds().Height()); 
}

void CColumnListViewEx::AttachedToWindow()
{
	ColumnListView::AttachedToWindow();
	
	detector = new CContextMenuDetector(this);
	
	SetViewColor(CColor::Transparent);
}

void CColumnListViewEx::KeyDown(const char *bytes, int32 numBytes)
{
	if(isalnum(bytes[0])) {
		// User entered alphanumeric char ([A-Z][a-z][0-9])
		// Try to find entry which starts with that char.
		
		CLVEasyItem *item = dynamic_cast<CLVEasyItem *>(ItemAt(0));

		if(item == NULL) {
			// List is empty or there are no CLVEasyItems in the list.
			return;
		}
		
		int32 firstTextColumn=-1;
		
		// Find first text column. I hope that all entries have the same
		// structure.
		for(int32 i=0 ; i<CountColumns() ; i++) {
			if(item->GetColumnContentText(i) != NULL) {
				firstTextColumn = i;
				break;
			}
		}

		if(firstTextColumn < 0) {
			// No text column!
			return;
		}
		
		char findChar = toupper(bytes[0]);
		
		// Start at current selection
		int32 start = CurrentSelection(0)+1;
		
		if(start < 0) start = 0;
		
		for(int32 i=0 ; i<CountItems()-1 ; i++) {
			int32 pos = (i+start) % CountItems();
		
			item = dynamic_cast<CLVEasyItem *>(ItemAt(pos));
		
			if(item != NULL) {
				const char *text = item->GetColumnContentText(firstTextColumn);
				
				if(text != NULL && toupper(text[0]) == findChar) {
					// Found item. Select it.
					Select(pos);
					ScrollToSelection();
					break;
				}
			}
		}
	} else {
		ColumnListView::KeyDown(bytes, numBytes);
	}
}

void CColumnListViewEx::MouseDown(BPoint point)
{
	detector->MouseDown(point);

	ColumnListView::MouseDown(point);	
	
	// This fixes a bug with the event handling in the BListView.
	// With the settings of BListView for the event masks sometimes
	// it looks like the B_MOUSE_UP event is lost.
	if(Window()->Flags() & B_ASYNCHRONOUS_CONTROLS) {
		SetEventMask(0, 0);
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
	}
}

void CColumnListViewEx::MouseUp(BPoint point)
{
	detector->MouseUp(point);
	
	ColumnListView::MouseUp(point);
}

BHandler *CColumnListViewEx::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_COUNT_PROPERTIES:
			if(what == B_DIRECT_SPECIFIER && strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN) == 0) {
				return this;
			}
			break;
		case B_SET_PROPERTY:
			// FALL THROUGH
		case B_GET_PROPERTY:
			if((what == B_INDEX_SPECIFIER || what == B_REVERSE_INDEX_SPECIFIER || 
				what == B_NAME_SPECIFIER) && strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN) == 0) {
				return this;
			}
			break;
	}

	return ColumnListView::ResolveSpecifier(message, index, specifier, what, property);
}

status_t CColumnListViewEx::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)COLUMN_LIST_VIEW_PROP_COLUMN,	// name
			{ B_COUNT_PROPERTIES, 0 },				// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)COLUMN_LIST_VIEW_PROP_COLUMN,	// name
			{ 0 },									// commands
			{										// specifiers
				B_INDEX_SPECIFIER,
				B_REVERSE_INDEX_SPECIFIER,
				B_NAME_SPECIFIER,
				0
			 },				
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-column-list-view");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return ColumnListView::GetSupportedSuites(message);
}

// GetColumnIndex
// Gets the column index from a specifier message. Specifier is the message itself, what its 'what'
// datafield. The index is returned in 'columnIndex'. If this method returns an error, the
// returned index is invalid.
status_t CColumnListViewEx::GetColumnIndex(BMessage *specifier, int32 what, int32 &columnIndex)
{
	status_t result=B_ERROR;

	switch(what)
	{
		case B_INDEX_SPECIFIER:
			result = specifier->FindInt32("index", &columnIndex);
			break;
		case B_REVERSE_INDEX_SPECIFIER:
			result = specifier->FindInt32("index", &columnIndex);
			columnIndex = CountColumns()-columnIndex-1;
			break;
		case B_NAME_SPECIFIER:
			{
				const char *columnName=NULL;
		
				if((result = specifier->FindString("name", &columnName)) == B_OK && columnName != NULL) {
					result = B_BAD_INDEX;
				
					for(int32 i=0 ; i<CountColumns() ; i++) {
						const char *currentName = ColumnAt(i)->GetLabel();
					
						if(currentName != NULL && strcmp(currentName, columnName) == 0) {
						
							columnIndex = i;
							result = B_OK;
							break;
						}
					}
				}
			}
			break;
		default:
			result = B_BAD_VALUE;
	}

	return result;						
}

void CColumnListViewEx::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case B_COUNT_PROPERTIES:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
			
				if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN) == 0 && what == B_DIRECT_SPECIFIER) {
						// Handle COUNT_PROPERTIES for Column Property.
						
						BMessage reply(B_REPLY);
						
						reply.AddInt32("result", CountColumns());
						
						message->PopSpecifier();

						send_script_reply(reply, B_OK, message);
						
						return;
					}
				}
			}
			break;
		case B_SET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
			
				if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN) == 0) {
						// Handle SET_PROPERTY for Column.
					
						int32 columnIndex=-1;
						status_t result;

						// Get column index.
						if((result = GetColumnIndex(&specifier, what, columnIndex)) == B_OK) {
							message->PopSpecifier();
							
							if((result = message->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
								if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_VISIBLE) == 0 && what == B_DIRECT_SPECIFIER) {
									// SET_PROPERTY for Visible property of column
								
									bool visible;
								
									if((result = message->FindBool("data", &visible)) == B_OK) {
										CLVColumn *column = ColumnAt(columnIndex);
									
										if(column != NULL) {
											column->SetShown(visible);
											result = B_OK;
										} else {
											result = B_BAD_INDEX;
										}
									}
								
									message->PopSpecifier();
								} else if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_WIDTH) == 0 && what == B_DIRECT_SPECIFIER) {
									// SET_PROPERTY for Width property of column

									float width;
								
									if((result = message->FindFloat("data", &width)) == B_OK) {
										CLVColumn *column = ColumnAt(columnIndex);
									
										if(column != NULL) {
											column->SetWidth(width);
											result = B_OK;
										} else {
											result = B_BAD_INDEX;
										}
									}
									
									message->PopSpecifier();
								} else if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_SORT_MODE) == 0 && what == B_DIRECT_SPECIFIER) {
									// SET_PROPERTY for SortMode property of column

									int32 sortMode;
								
									if((result = message->FindInt32("data", &sortMode)) == B_OK) {
										// SortMode:
										//   0 = ascending
										//   1 = descending
										//   2 = no sort
									
										if(sortMode >= 0 && sortMode < 3) {
											CLVColumn *column = ColumnAt(columnIndex);
									
											if(column != NULL) {
												column->SetSortMode((CLVSortMode)sortMode);
											
												if(sortMode != NoSort)
													AddSortKey(columnIndex);
											
												result = B_OK;
											} else {
												result = B_BAD_INDEX;
											}
										} else {
											result = B_BAD_VALUE;
										}
									}
									
									message->PopSpecifier();
								} else {
									result = B_BAD_VALUE;
								}
							}
						}
						
						
						BMessage reply(B_REPLY);
						send_script_reply(reply, result, message);
						
						return;
					}
				}
			}
			break;
		case B_GET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
			
				if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN) == 0) {
						// Handle GET_PROPERTY for Column.
					
						int32 columnIndex=-1;
						BMessage reply(B_REPLY);
						status_t result;

						// Get column index.
						if((result = GetColumnIndex(&specifier, what, columnIndex)) == B_OK) {
							message->PopSpecifier();
							
							if((result = message->GetCurrentSpecifier(&index, &specifier, &what, &property)) == B_OK) {
								if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_VISIBLE) == 0 && what == B_DIRECT_SPECIFIER) {
									// GET_PROPERTY for Visible property of column
								
									CLVColumn *column = ColumnAt(columnIndex);
									
									if(column != NULL) {
										bool visible = column->IsShown();
										reply.AddBool("result", visible);
										result = B_OK;
									} else {
										result = B_BAD_INDEX;
									}
								
									message->PopSpecifier();
								} else if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_WIDTH) == 0 && what == B_DIRECT_SPECIFIER) {
									// GET_PROPERTY for Width property of column
								
									CLVColumn *column = ColumnAt(columnIndex);
									
									if(column != NULL) {
										float width = column->Width();
										reply.AddFloat("result", width);
										result = B_OK;
									} else {
										result = B_BAD_INDEX;
									}
									
									message->PopSpecifier();
								} else if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_LABEL) == 0 && what == B_DIRECT_SPECIFIER) {
									// GET_PROPERTY for Label property of column
								
									CLVColumn *column = ColumnAt(columnIndex);
									
									if(column != NULL) {
										const char *label = (column->GetLabel() != NULL) ? column->GetLabel() : "";
										reply.AddString("result", label);
										result = B_OK;
									} else {
										result = B_BAD_INDEX;
									}
									
									message->PopSpecifier();
								} else if(strcmp(property, COLUMN_LIST_VIEW_PROP_COLUMN_SORT_MODE) == 0 && what == B_DIRECT_SPECIFIER) {
									// GET_PROPERTY for SortMode property of column
								
									CLVColumn *column = ColumnAt(columnIndex);
									
									if(column != NULL) {
										int32 sortMode = column->SortMode();
										reply.AddInt32("result", sortMode);
										result = B_OK;
									} else {
										result = B_BAD_INDEX;
									}
									
									message->PopSpecifier();
								} else {
									result = B_BAD_VALUE;
								}
							}
						}
						
						send_script_reply(reply, result, message);
						
						return;
					}
				}
			}
			break;
		case MSG_CONTEXT_MENU:
			{
				BPoint point = message->FindPoint("where");
			
				int32 itemIndex = IndexOf(point);
		
				if(itemIndex >= 0) {
					CLVEasyItemEx *item = dynamic_cast<CLVEasyItemEx *>(ItemAt(itemIndex));
					
					if(item) {
						// Simulate a mouse up event. Otherwise the
						// items under the menu change selection, when
						// the mouse is moved.
						MouseUp(point);
						SetMouseEventMask(0, 0);
					
						Select(itemIndex);
						
						BPoint screenPoint = ConvertToScreen(point);
						
						// display context menu for entry.
						item->DisplayContextMenu(this, screenPoint);
						
						return;
					}
				}
			}
			break;
	}

	ColumnListView::MessageReceived(message);
}

BRect CColumnListViewEx::CreateOffscreenBuffer()
{
	if(offscreenBuffer) {
		float bitmapW = offscreenBuffer->Bounds().Width();
		float windowW = Bounds().Width();

		float bitmapH = offscreenBuffer->Bounds().Height();
		float windowH = Bounds().Height();

		if(bitmapW != windowW || bitmapH != windowH) {
			// the buffer is too small or too big.
			delete offscreenBuffer;
			offscreenBuffer = NULL;
		}
	}
	
	BRect bitmapBounds(0, 0, Bounds().Width(), Bounds().Height());
	
	if(offscreenBuffer == NULL) {
		offscreenBuffer = new BBitmap(bitmapBounds, BScreen(Window()).ColorSpace(), true);
	}
	
	return bitmapBounds;
}

void CColumnListViewEx::Draw(BRect updateRect)
{
	float width  = Bounds().Width();
	float height = Bounds().Height();

	if(width <= 0 || height <= 0 || IsHidden())
		return;

	// fill the new areas with white.
	SetHighColor(CColor::White);

	if(width > oldWidth)
		FillRect(BRect(oldWidth+1, 0, width, height));

	if(height > oldHeight)
		FillRect(BRect(0, oldHeight+1, oldWidth, height));

	oldWidth  = width;
	oldHeight = height;

	Sync();

	BPicture *picture = new BPicture(); 
	
	// draw the update area into a picture object (not on screen).
	BeginPicture(picture);
	
	SetHighColor(CColor::White);
	FillRect(updateRect);

	ColumnListView::Draw(updateRect);

	EndPicture();
	
	BRect bitmapBounds = CreateOffscreenBuffer();
	
	if(offscreenBuffer->Lock()) {	
		if(offscreenBuffer->CountChildren() < 1) {
			offscreenBuffer->AddChild(new CPictureView(bitmapBounds, NULL));
		}
		
		MY_ASSERT(offscreenBuffer->CountChildren() == 1);
		
		CPictureView *offscreenView = dynamic_cast<CPictureView *>(offscreenBuffer->ChildAt(0));
		
		MY_ASSERT(offscreenView);

		BPoint topLeft = Bounds().LeftTop();
		
		// replay metafile (picture) into offscreen bitmap.
		offscreenView->ResizeTo(Bounds().Width(), Bounds().Height());
		offscreenView->SetPicture(picture);
		offscreenView->SetOffset(B_ORIGIN-topLeft);
		offscreenView->Draw(bitmapBounds);
		
		BRect drawRect = updateRect;
		
		drawRect.OffsetBy(B_ORIGIN-topLeft);
		
		// display resulting bitmap.
		DrawBitmapAsync(offscreenBuffer, drawRect, updateRect);
		
		offscreenBuffer->Unlock();
	}
}

// ==== CPictureView ====

CPictureView::CPictureView(BRect frame, BPicture *picture) :
	BView(frame, "PictureView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	pict = picture;
}

CPictureView::~CPictureView()
{
	if(pict) delete pict;
}
	
void CPictureView::AttachedToWindow()
{
	BView::AttachedToWindow();

	if(Parent())
		SetViewColor(Parent()->ViewColor());
}

void CPictureView::Draw(BRect updateRect)
{
	BAutolock lock(Window());

	if(lock.IsLocked()) {
		if(!pict)
			return;
		
		DrawPicture(pict, offset);

		Sync();
	}
}

void CPictureView::SetPicture(BPicture *newPict)
{
	BAutolock lock(Window());
	
	if(lock.IsLocked()) {
		if(pict) delete pict;
	
		pict = newPict;
	}
}
