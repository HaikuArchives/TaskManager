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
#include "Process.h"	// for enumTeamAction (TEAM_ACTION_xxx)
#include "my_assert.h"
#include "LocalizationHelper.h"
#include "ProcessView.h"
#include "ADither.h"	// for CreateCloneBitmap
#include "SelectTeamWindow.h"

// ==== CSelectTeamWindow ====

CSelectTeamWindow::CSelectTeamWindow() :
	CSingletonWindow(
		BRect(0,0,10,10), 
		CLocalizationHelper::GetDefaultInstance()->String("SelectTeamWindow.Title"), 
		B_TITLED_WINDOW, 
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, 
		B_CURRENT_WORKSPACE)
{
	static const float windowWidth = 200;
	float windowHeight;

	BView *view = new CSelectTeamWindowView(BRect(0, 0, windowWidth, 10));
	
	view->GetPreferredSize(NULL, &windowHeight);
	view->ResizeToPreferred();

	BRect screenRect = BScreen(this).Frame();
	
	// center window
	float wx = (screenRect.Width()-windowWidth)/2;
	float wy = (screenRect.Height()-windowHeight)/2;

	MoveTo(wx, wy);
	ResizeTo(windowWidth, windowHeight);

	AddChild(view);
	
	Show();
}

CSelectTeamWindow::~CSelectTeamWindow()
{
	RemoveFromList(ClassName());
}

CSelectTeamWindow *CSelectTeamWindow::CreateInstance()
{
	// Initialize to quiet compiler.
	CSelectTeamWindow *window = NULL;

	return CreateSingleton(window, "CSelectTeamWindow");
}

// Set the target for the MSG_TEAM_ACTION message which is send when
// a team is selected.
void CSelectTeamWindow::SetTarget(BView *target)
{
	BAutolock lock(this);

	CCrossHairDragView *dragView = 
		dynamic_cast<CCrossHairDragView *>(ChildAt(0)->ChildAt(1)->ChildAt(0));
	
	MY_ASSERT(dragView);
	
	dragView->SetTarget(BMessenger(target));
}

// ==== CSelectTeamWindowView ====

const float CSelectTeamWindowView::dist = 5.0;

CSelectTeamWindowView::CSelectTeamWindowView(BRect frame) :
	BView(frame, "SelectTeamWindowView", B_FOLLOW_NONE, 0)
{
	static const float crossHairSize = 30;
	static const float windowWidth = frame.Width();

	BRect textViewRect = BRect(dist, dist, windowWidth-dist, 10);
	BRect textRect = BRect(0, 0, textViewRect.Width(), 10);

	textView = new BTextView(textViewRect, "HintText", textRect, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_PULSE_NEEDED);

	textView->MakeEditable(false);
	textView->MakeSelectable(false);
	textView->Insert(CLocalizationHelper::GetDefaultInstance()->String("SelectTeamWindowView.HintText"));

	float textHeight = textView->TextHeight(0, 10000);
	
	textView->ResizeTo(textViewRect.Width(), textHeight+1);

	float crossHairX = (windowWidth-crossHairSize)/2;
	float crossHairY = textHeight+3*dist;

	crossHairBorder = new CBorderView(
							BRect(crossHairX, crossHairY, crossHairX+crossHairSize, crossHairY+crossHairSize),
							"CrossHairBorder",
							2,
							B_FOLLOW_NONE,
							B_WILL_DRAW,
							B_FANCY_BORDER);

	crossHair = new CCrossHairDragView(crossHairBorder->ClientRect());
	
	crossHairBorder->AddChild(crossHair);
	
	AddChild(textView);
	AddChild(crossHairBorder);

	textView->SetViewColor(CColor::BeBackgroundGray);
}

void CSelectTeamWindowView::AttachedToWindow()
{
	SetViewColor(CColor::BeBackgroundGray);
}

void CSelectTeamWindowView::GetPreferredSize(float *width, float *height)
{
	if(width)
		*width = Bounds().Width();

	if(height)
		*height = crossHairBorder->Frame().bottom + 2*dist;
}

// ==== CCrossHairDragView ====

CCrossHairDragView::CCrossHairDragView(BRect frame) :
	BView(frame, "CrossDragArea", B_FOLLOW_NONE, B_WILL_DRAW)
{
	isDragged = false;

	crossHair = new BBitmap(Bounds(), B_RGB32, true, false);
	
	BView *crossHairView = new CCrossHairView(crossHair->Bounds());
	
	crossHair->Lock();
	crossHair->AddChild(crossHairView);
	crossHairView->Draw(crossHair->Bounds());
	crossHair->Unlock();
}

CCrossHairDragView::~CCrossHairDragView()
{
	delete crossHair;
}

void CCrossHairDragView::AttachedToWindow()
{
	SetViewColor(CColor::White);
}

void CCrossHairDragView::Draw(BRect updateRect)
{
	SetDrawingMode(B_OP_OVER);
	DrawBitmap(crossHair, BPoint(0,0));
}

void CCrossHairDragView::SetTarget(BMessenger messenger)
{
	target = messenger;
}

void CCrossHairDragView::MouseDown(BPoint where)
{
	// Send a message as drag message, which isn't understood
	// by the target. The target will respond with a 
	// B_NOT_UNDERSTOOD message. The reply is ignored.
	BMessage dragMessage('!+#@');

	float width  = crossHair->Bounds().Width();
	float height = crossHair->Bounds().Height();

	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);

	isDragged = true;

	// The drag bitmap is deleted by the system. Create a copy
	// of the crosshair bitmap.
	BBitmap *clone = CreateCloneBitmap(crossHair);

	DragMessage(&dragMessage, clone, B_OP_OVER, BPoint(width/2,height/2), this);
}

void CCrossHairDragView::MouseUp(BPoint where)
{
	if(isDragged) {
		// The crosshair was dropped.
	
		// The point where it was dropped.
		BPoint screenPoint = ConvertToScreen(where);
		
		team_id team = FindTeam(screenPoint);
	
		if(team != -1) {
			BMessage killMessage(MSG_TEAM_ACTION), reply;
		
			killMessage.AddInt32(MESSAGE_DATA_ID_ACTION, TEAM_ACTION_KILL);
			killMessage.AddInt32(MESSAGE_DATA_ID_TEAM_ID, team);
		
			target.SendMessage(&killMessage, &reply);
		}
		
		isDragged = false;
	}
}

void CCrossHairDragView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	if(isDragged) {
		BPoint screenPoint = ConvertToScreen(point);
		
		team_id team = FindTeam(screenPoint);
		
		if(team != -1) {
			// The mouse is hoovering over a window.
			// Select the team which this window belongs to
			// in the process list.
		
			BMessage selectMessage(MSG_SELECT_TEAM);
			
			selectMessage.AddInt32(MESSAGE_DATA_ID_TEAM_ID, team);
			
			target.SendMessage(&selectMessage, this);
		}
	}
}

team_id CCrossHairDragView::FindTeam(const BPoint &screenPoint)
{
	int32 currentWorkspace = current_workspace();
	int32 tokenCount;

	// Get a list of window tokens in the order of the 
	// z-order of the windows.
	int32 *tokens = get_token_list(-1, &tokenCount);

	if(tokens) {
		for(int32 i=0 ; i<tokenCount; i++) {
			window_info *windowInfo = get_window_info(tokens[i]);
			
			if(!windowInfo) {
				// That window probably closed. Just go to the next one.
				continue;
			}
			
			if(!windowInfo->is_mini) {
				// Not a hidden/minimized window
				
				if((windowInfo->workspaces & (1 << currentWorkspace)) != 0) {
					// Window is on this workspace
					
					if(	screenPoint.x >= windowInfo->window_left &&
						screenPoint.x <= windowInfo->window_right &&
						screenPoint.y >= windowInfo->window_top &&
						screenPoint.y <= windowInfo->window_bottom ) {
						// I found a valid window.
						
						team_id id = windowInfo->team;
						
						free(windowInfo);
						
						return id;
					}
				}
			}
			
			free(windowInfo);
		}
		
		free(tokens);
	}
	
	return -1;
}

// ==== CCrossHairView ====

CCrossHairView::CCrossHairView(BRect frame) :
	BView(frame, "CrossHair", B_FOLLOW_NONE, B_WILL_DRAW)
{
}

void CCrossHairView::Draw(BRect updateRect)
{
	BRect bounds = Bounds();

	SetHighColor(CColor::Transparent);

	FillRect(bounds);

	BRect bigCircle, smallCircle;
	
	float width  = bounds.Width();
	float height = bounds.Height();
	
	bigCircle = bounds.InsetByCopy(5, 5);
	smallCircle = bounds.InsetByCopy(10, 10);

	SetHighColor(CColor::White);
	StrokeEllipse(bigCircle.OffsetByCopy(1, 1));
	StrokeEllipse(smallCircle.OffsetByCopy(1, 1));
	
	StrokeLine(BPoint(2, height/2+1), BPoint(width, height/2+1));
	StrokeLine(BPoint(width/2+1, 2), BPoint(width/2+1, height));
	
	SetHighColor(CColor::Black);
	StrokeEllipse(bigCircle);
	StrokeEllipse(smallCircle);
	
	StrokeLine(BPoint(1, height/2), BPoint(width-1, height/2));
	StrokeLine(BPoint(width/2, 1), BPoint(width/2, height-1));
	
	Sync();
}