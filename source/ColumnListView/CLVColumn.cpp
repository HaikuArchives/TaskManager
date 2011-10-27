//Column list header source file

//*** LICENSE ***
//ColumnListView, its associated classes and source code, and the other components of Santa's Gift Bag are
//being made publicly available and free to use in freeware and shareware products with a price under $25
//(I believe that shareware should be cheap). For overpriced shareware (hehehe) or commercial products,
//please contact me to negotiate a fee for use. After all, I did work hard on this class and invested a lot
//of time into it. That being said, DON'T WORRY I don't want much. It totally depends on the sort of project
//you're working on and how much you expect to make off it. If someone makes money off my work, I'd like to
//get at least a little something.  If any of the components of Santa's Gift Bag are is used in a shareware
//or commercial product, I get a free copy.  The source is made available so that you can improve and extend
//it as you need. In general it is best to customize your ColumnListView through inheritance, so that you
//can take advantage of enhancements and bug fixes as they become available. Feel free to distribute the 
//ColumnListView source, including modified versions, but keep this documentation and license with it.


//******************************************************************************************************
//**** SYSTEM HEADER FILES
//******************************************************************************************************
#include <string.h>
#include <Window.h>

//******************************************************************************************************
//**** PROJECT HEADER FILES
//******************************************************************************************************
#include "CLVColumn.h"
#include "ColumnListView.h"
#include "CLVColumnLabelView.h"


//******************************************************************************************************
//**** CLVColumn CLASS DEFINITION
//******************************************************************************************************
CLVColumn::CLVColumn(const char* label,float width,uint32 flags,float min_width)
{
	if(flags & CLV_EXPANDER)
	{
		label = NULL;
		width = 20.0;
		min_width = 20.0;
		flags &= CLV_NOT_MOVABLE | CLV_LOCK_AT_BEGINNING | CLV_HIDDEN | CLV_LOCK_WITH_RIGHT;
		flags |= CLV_EXPANDER | CLV_NOT_RESIZABLE | CLV_MERGE_WITH_RIGHT;
	}
	if(min_width < 4.0)
		min_width = 4.0;
	if(width < min_width)
		width = min_width;
	if(label)
	{
		fLabel = new char[strlen(label)+1];
		strcpy((char*)fLabel,label);
		if(CLV_HEADER_TRUNCATE)
		{
			fTruncatedText = new char[strlen(label)+3];
			GetTruncatedString(label,fTruncatedText,width-15);
			fCachedRect.Set(-1,-1,-1,-1);		
		}
		else
			fTruncatedText = NULL;
	}
	else
	{
		fLabel = NULL;
		fTruncatedText = NULL;
	}
	fWidth = width;
	fMinWidth = min_width;
	fFlags = flags;
	fPushedByExpander = false;
	fParent = NULL;
	fSortMode = NoSort;
	
	// ADDED TK
	fNeedsTruncate = false;
}


CLVColumn::~CLVColumn()
{
	if(fParent)
	{
		fParent->AssertWindowLocked();
		fParent->RemoveColumn(this);
	}
	if(fLabel)
		delete[] fLabel;
	if(fTruncatedText)
		delete[] fTruncatedText;

}


float CLVColumn::Width() const
{
	return fWidth;
}


void CLVColumn::SetWidth(float width)
{
	if(fParent)
		fParent->AssertWindowLocked();
	if(width < fMinWidth)
		width = fMinWidth;
	if(width != fWidth)
	{
		float OldWidth = fWidth;
		fWidth = width;
		if(IsShown() && fParent)
		{
			if(fFlags & CLV_HEADER_TRUNCATE)
			{
				//Do truncation of label
				
				// CHANGED TK 04Feb2000
				float truncWidth = width;
			
				if(!(fFlags & CLV_NOT_SHOW_ARROW) && 
					fParent->fSortKeyList.HasItem(this)) {
					// Is a sort key. Reserve space for arrow.
					truncWidth -= 3.0+fParent->fColumnLabelView->fFontAscent;
				}
				
				BRect invalid_region = TruncateText(truncWidth/*was: width*/);
				if(fCachedRect != BRect(-1,-1,-1,-1))
				{
					float delta_width = width-Width();
					fCachedRect.right += delta_width;
				}
				if(invalid_region != BRect(-1,-1,-1,-1))
					GetHeaderView()->Invalidate(invalid_region);
			}

			//Figure out the area after this column to scroll
			BRect ColumnViewBounds = fParent->fColumnLabelView->Bounds();
			BRect MainViewBounds = fParent->Bounds();
			BRect SourceArea = ColumnViewBounds;
			SourceArea.left = fColumnEnd+1.0;
			BRect DestArea = SourceArea;
			float Delta = width-OldWidth;
			DestArea.left += Delta;
			DestArea.right += Delta;
			float LimitShift;
			if(DestArea.right > ColumnViewBounds.right)
			{
				LimitShift = DestArea.right-ColumnViewBounds.right;
				DestArea.right -= LimitShift;
				SourceArea.right -= LimitShift;
			}
			if(DestArea.left < ColumnViewBounds.left)
			{
				LimitShift = ColumnViewBounds.left - DestArea.left;
				DestArea.left += LimitShift;
				SourceArea.left += LimitShift;
			}

			//Scroll the area that is being shifted
			BWindow* ParentWindow = fParent->Window();
			if(ParentWindow)
				ParentWindow->UpdateIfNeeded();
			fParent->fColumnLabelView->CopyBits(SourceArea,DestArea);
			SourceArea.top = MainViewBounds.top;
			SourceArea.bottom = MainViewBounds.bottom;
			DestArea.top = MainViewBounds.top;
			DestArea.bottom = MainViewBounds.bottom;
			fParent->CopyBits(SourceArea,DestArea);

			//Invalidate the region that got revealed
			DestArea = ColumnViewBounds;
			if(width > OldWidth)
			{
				DestArea.left = fColumnEnd+1.0;
				DestArea.right = fColumnEnd+Delta;
			}
			else
			{
				DestArea.left = ColumnViewBounds.right+Delta+1.0;
				DestArea.right = ColumnViewBounds.right;
			}
			
			fParent->fColumnLabelView->Invalidate(DestArea);

			// ADDED TK 04Feb2000
			if( !(fFlags & CLV_NOT_SHOW_ARROW) ) 
			{
				// Invalidate full column-header (because of arrow).
				BRect InvalidRect = ColumnViewBounds;
				InvalidRect.left  = fColumnBegin;
				InvalidRect.right = fColumnEnd;
				fParent->fColumnLabelView->Invalidate(InvalidRect);
			} 
			
			DestArea.top = MainViewBounds.top;
			DestArea.bottom = MainViewBounds.bottom;
			fParent->Invalidate(DestArea);

			//Invalidate the old or new resize handle as necessary
			DestArea = ColumnViewBounds;
			if(width > OldWidth)
				DestArea.left = fColumnEnd;
			else
				DestArea.left = fColumnEnd + Delta;
			DestArea.right = DestArea.left;
			fParent->fColumnLabelView->Invalidate(DestArea);
			
			//Update the column sizes, positions and group positions
			fParent->UpdateColumnSizesDataRectSizeScrollBars(false);
			fParent->fColumnLabelView->UpdateDragGroups();
		}
		if(fParent)
			fParent->ColumnWidthChanged(fParent->fColumnList.IndexOf(this),fWidth);
	}
}


BRect CLVColumn::TruncateText(float column_width)
//Returns whether the truncated text has changed
{
	// ADDED TK
	fNeedsTruncate = false;

	column_width -= 1+8+5+1;
		//Because when I draw the text I start drawing 8 pixels to the right from the text area's left edge,
		//which is in turn 1 pixel smaller than the column at each edge, and I want 5 trailing pixels.
	BRect invalid(-1,-1,-1,-1);
	const char* text = GetLabel();
	char* new_text = new char[strlen(text)+3];
	GetTruncatedString(text,new_text,column_width);
	if(strcmp(fTruncatedText,new_text)!=0)
	{
		//The truncated text has changed
		invalid = fCachedRect;
		if(invalid != BRect(-1,-1,-1,-1))
		{
			//Figure out which region just got changed
			int32 cmppos;
			int32 cmplen = strlen(new_text);
			char remember = 0;
			for(cmppos = 0; cmppos <= cmplen; cmppos++)
				if(new_text[cmppos] != fTruncatedText[cmppos])
				{
					remember = new_text[cmppos];
					new_text[cmppos] = 0;
					break;
				}
			invalid.left += 8 + be_plain_font->StringWidth(new_text);
			new_text[cmppos] = remember;
		}
		//Remember the new truncated text
		strcpy(fTruncatedText,new_text);
	}
	delete[] new_text;
	return invalid;
}


void GetTruncatedString(const char* full_string, char* truncated, float width)
{
	strcpy(truncated,full_string);
	int32 choppos = strlen(full_string)-1;
	while(choppos >= -2 && be_plain_font->StringWidth(truncated) > width)
	{
		while(choppos > 0 && truncated[choppos-1] == ' ')
			choppos--;
		if(choppos > 0 || (choppos == 0 && truncated[0] == ' '))
			truncated[choppos] = '.';
		if(choppos > -1)
			truncated[choppos+1] = '.';
		if(choppos > -2)
			truncated[choppos+2] = '.';
		truncated[choppos+3] = 0;
		choppos--;
	}
}


uint32 CLVColumn::Flags() const
{
	return fFlags;
}


bool CLVColumn::IsShown() const
{
	if(fFlags & CLV_HIDDEN)
		return false;
	else
		return true;
}


void CLVColumn::SetShown(bool Shown)
{
	if(fParent)
		fParent->AssertWindowLocked();
	bool shown = IsShown();
	if(shown != Shown)
	{
		if(Shown)
			fFlags &= 0xFFFFFFFF^CLV_HIDDEN;
		else
			fFlags |= CLV_HIDDEN;
		if(fParent)
		{
			float UpdateLeft = fColumnBegin;
			fParent->UpdateColumnSizesDataRectSizeScrollBars();
			fParent->fColumnLabelView->UpdateDragGroups();
			if(Shown)
				UpdateLeft = fColumnBegin;
			BRect Area = fParent->fColumnLabelView->Bounds();
			Area.left = UpdateLeft;
			fParent->fColumnLabelView->Invalidate(Area);
			Area = fParent->Bounds();
			Area.left = UpdateLeft;
			fParent->Invalidate(Area);
			if(fFlags & CLV_EXPANDER)
			{
				if(!Shown)
					fParent->fExpanderColumn = -1;
				else
					fParent->fExpanderColumn = fParent->IndexOfColumn(this);
			}
		}
	}
}


CLVSortMode CLVColumn::SortMode() const
{
	return fSortMode;
}

void CLVColumn::SetSortMode(CLVSortMode mode)
{
	if(fParent)
	{
		fParent->AssertWindowLocked();
		fParent->SetSortMode(fParent->IndexOfColumn(this),mode);
	}
	else
		fSortMode = mode;
}

void CLVColumn::SortingChanged(CLVSortMode mode) 
{
	// ADDED TK 04Feb2000
	if( !(fFlags & CLV_NOT_SHOW_ARROW) ) 
	{
		if( (fSortMode == NoSort && mode != NoSort) || 
			(fSortMode != NoSort && mode == NoSort) ) {
			// The width of the text area changed because the arrow is
			// displayed or hidden.
			fNeedsTruncate = true;
		}
	}
	
	fSortMode = mode;
}

const char* CLVColumn::GetLabel() const
{
	return fLabel;
}


ColumnListView* CLVColumn::GetParent() const
{
	return fParent;
}


BView* CLVColumn::GetHeaderView() const
{
	if(fParent)
		return fParent->fColumnLabelView;
	else
		return NULL;
}


// TK 17Feb2000: added 'primary_sort_key'. If set to true the sort label is
// solid underlined, else 'underdotted'.
void CLVColumn::DrawColumnHeader(BView* view, BRect header_rect, bool sort_key, 
	bool primary_sort_key, bool focus, float font_ascent)
{
	char* label=NULL;
	if(fFlags & CLV_HEADER_TRUNCATE)
		label = fTruncatedText;
	else
		label = fLabel;

	// ADDED TK 04Feb2000
	if(fNeedsTruncate) {
		TruncateText(header_rect.Width());
	} 

	if(label)
	{
		if(focus)
			view->SetHighColor(CColor::BeFocusBlue);
		else
			view->SetHighColor(CColor::Black);
	
		//Draw the label
		view->SetDrawingMode(B_OP_OVER);
		BPoint text_point(header_rect.left+8.0,header_rect.top+1.0+font_ascent);
		view->DrawString(label,text_point);
		view->SetDrawingMode(B_OP_COPY);
	
		//Underline if this is a selected sort column
		if(sort_key)
		{
			float Width = view->StringWidth(label);
			view->StrokeLine(BPoint(text_point.x-1,text_point.y+2.0),
				BPoint(text_point.x-1+Width,text_point.y+2.0),
				primary_sort_key ? B_SOLID_HIGH : B_MIXED_COLORS);
		}
		fCachedRect = header_rect;
	}
}
