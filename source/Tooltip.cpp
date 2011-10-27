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
#include "Tooltip.h"

#include "my_assert.h"

// ====== CTimer ======

CTimer::CTimer(bigtime_t _period, bool _singlePing, BInvoker *_invoker)
{
	thread = spawn_thread(ThreadStartFunc, "TimerThread", B_NORMAL_PRIORITY, (void *)this);
	
	MY_ASSERT(thread >= B_OK);
	
	period		= _period;
	singlePing	= _singlePing;
	invoker		= _invoker;
	
	resume_thread(thread);
}

CTimer::~CTimer()
{
	if(thread >= 0)
		kill_thread(thread);
		
	delete invoker;
}

int32 CTimer::ThreadStartFunc(void *data)
{
	return static_cast<CTimer *>(data)->TimerThread();
}

int32 CTimer::TimerThread()
{
	while(!singlePing) {
		snooze(period);
		invoker->Invoke();
	}
	
	return 0;
}

void CTimer::StopTimer()
{
	kill_thread(thread);
	thread = -1;
}

void CTimer::RestartTimer()
{
	if(thread >= 0)
		kill_thread(thread);

	thread = spawn_thread(ThreadStartFunc, "TimerThread", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(thread);
}

// ====== CMouseWatcher ======

CMouseWatcher::CMouseWatcher(BMessenger *_target)
{
	target = _target;
	exitLoop = false;
	watcherThread = -1;
}

CMouseWatcher::~CMouseWatcher()
{
	if(watcherThread >= 0) {
		exitLoop = true;		// tell watcher thread to exit
		
		status_t exitValue;		// this value is ignored

		wait_for_thread(watcherThread, &exitValue);
	}
		
	delete target;
	target = NULL;
}

void CMouseWatcher::StartWatching()
{
	CreateWatcherThread();
}

void CMouseWatcher::CreateWatcherThread()
{
	watcherThread = spawn_thread(StartWatcherThread, "MWatcherThread",
									B_NORMAL_PRIORITY, this);
									
	MY_ASSERT(watcherThread >= B_OK);
	
	resume_thread(watcherThread);
}

int32 CMouseWatcher::StartWatcherThread(void *obj)
{
	reinterpret_cast<CMouseWatcher *>(obj)->WatcherThread();
	
	return 0;
}

void CMouseWatcher::WatcherThread()
{
	BPoint previousPos(0,0);
	bool firstCheck=true;

	while(true) {
		if(!target->LockTarget()) {
			// window is dead
			return;
		}
		
		BLooper *looper;
		
		BView *view = dynamic_cast<BView *>(target->Target(&looper));

		BPoint mousePos;
		uint32 buttons;
		
		if(view != NULL) {
			view->GetMouse(&mousePos, &buttons, false);
		} else {
			// The target is a window
			
			BWindow *window = dynamic_cast<BWindow *>(looper);
			
			MY_ASSERT(window != NULL);
			
			if(window->CountChildren() <= 0) {
				MY_ASSERT(!"Window has no children!");
			}
			
			window->ChildAt(0)->GetMouse(&mousePos, &buttons, false);
			window->ChildAt(0)->ConvertToScreen(&mousePos);
			
			window->ConvertFromScreen(&mousePos);
		}
		
		if(firstCheck) {
			previousPos = mousePos;
			firstCheck = false;
		}
		
		BMessage message;
		bool sendMessage = false;
		
		if(mousePos != previousPos) {
			message.what = MSG_MOUSE_WATCHER;
			message.AddInt32("code", MOUSE_WATCHER_MOUSE_MOVED);
			message.AddPoint("where", mousePos);
			sendMessage = true;
		}
		
		looper->Unlock();
		
		if(sendMessage) target->SendMessage(&message);
		
		snooze(50000);
		
		if(exitLoop)
			break;
	}
}

// ====== CTooltipView ======

CTooltipView::CTooltipView(BRect frame) :
	BView(frame, "TooltipView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	tipText = NULL;
}

CTooltipView::~CTooltipView()
{
	delete [] tipText;
}

void CTooltipView::AttachedToWindow()
{
	BView::AttachedToWindow();

	SetViewColor(255, 255, 150);
}

void CTooltipView::Draw(BRect updateRect)
{
	SetHighColor(CColor::Black);
	
	if(tipText) {
		font_height fh;
	
		be_plain_font->GetHeight(&fh);
	
		SetLowColor(ViewColor());
		DrawString(tipText, BPoint(3.0, fh.ascent+2.0));
	}
	
	StrokeRect(Bounds());
}

void CTooltipView::MouseDown(BPoint point)
{
	BMessage *currentMessage = Window()->CurrentMessage();

	// Forward message to window
	Window()->PostMessage(currentMessage, Window());
}

void CTooltipView::GetPreferredSize(float *w, float *h)
{
	if(w) {
		*w = tipText ? ceil(StringWidth(tipText) + 4.0) : 0.0;
	}
	
	if(h) {
		font_height fh;
		
		GetFontHeight(&fh);
	
		*h = ceil(fh.ascent + fh.descent + 4.0);
	}
}

void CTooltipView::SetText(const char *text)
{
	delete [] tipText;
	
	tipText = new char [strlen(text)+1];
	
	strcpy(tipText, text); 
}

// ====== CTooltip ======

CTooltip::CTooltip(BView *_owner) :
	BWindow(
		BRect(-10, -10, -5, -5), 
		"Tooltip", 
		B_NO_BORDER_WINDOW_LOOK,
		B_FLOATING_ALL_WINDOW_FEEL, 
		B_AVOID_FOCUS | B_WILL_ACCEPT_FIRST_CLICK,
		B_CURRENT_WORKSPACE),
	owner(_owner)
{
	AddChild(new CTooltipView(Bounds()));
	
	mouseWatcher = NULL;
}

CTooltip::~CTooltip()
{
	delete mouseWatcher;
}

// ShowTooltip
// Displays a tooltip with 'text' on screenPoint. If the mouse cursor leafs 'screenRect'
// the tooltip is hidden. The corner specified in 'corner' is displayed on position 'screenPoint'.
void CTooltip::ShowTooltip(const char *text, BRect screenRect, BPoint screenPoint, 
	enumToolTipCorner corner)
{
	BAutolock autoLocker(this);

	if(autoLocker.IsLocked()) {
		toolRect = screenRect;
		
		if(mouseWatcher == NULL) {
			mouseWatcher = new CMouseWatcher(new BMessenger(NULL, this));
			mouseWatcher->StartWatching();
		}

		CTooltipView *view = dynamic_cast<CTooltipView *>(ChildAt(0));

		MY_ASSERT(view);
	
		view->SetText(text);
	
		float width, height;
		
		view->GetPreferredSize(&width, &height);
	
		BScreen screen(this);
	
		BRect screenRect = screen.Frame();

		switch(corner) {
			case TTC_NONE:
				// FALL THROUGH
			case TTC_LEFT_TOP:
				// don't change screen point.
				break;
			case TTC_LEFT_BOTTOM:
				screenPoint.y -= height;
				break;
			case TTC_RIGHT_BOTTOM:
				screenPoint.x -= width;
				screenPoint.y -= height;
				break;
			case TTC_RIGHT_TOP:
				screenPoint.x -= width;
				break;
		}
		
		if(screenPoint.x + width > screenRect.Width())
			screenPoint.x = screenRect.Width() - width;
			
		if(screenPoint.y + height > screenRect.Height())
			screenPoint.y = screenRect.Height() - height;

		if(screenPoint.x < 0)
			screenPoint.x = 0;

		if(screenPoint.y < 0)
			screenPoint.y = 0;

		ResizeTo(width, height);
		MoveTo(screenPoint);
		
		view->MoveTo(0, 0);
		view->ResizeTo(width, height);
		view->Invalidate();

		if(IsHidden()) {
			SetWorkspaces(B_CURRENT_WORKSPACE);
			Show();
		}
	}
}

void CTooltip::HideTooltip()
{
	BAutolock autoLocker(this);

	if(autoLocker.IsLocked()) {
		if(!IsHidden()) 
			Hide();
	}
}

bool CTooltip::Visible()
{
	BAutolock autoLocker(this);

	if(autoLocker.IsLocked()) {
		if(IsHidden()) 
			return false;
			
		return true;
	}
	
	return false;
}

void CTooltip::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_MOUSE_WATCHER:
			{
				int32 code 		= message->FindInt32("code");
				BPoint where 	= message->FindPoint("where");
				
				switch(code) {
					case MOUSE_WATCHER_MOUSE_MOVED:
						ConvertToScreen(&where);					
					
						if(!toolRect.Contains(where)) {
							HideTooltip();
						}
						break;
					default:
						// quiet compiler
						break;
				} 
			}
			break;
		case B_MOUSE_DOWN:
			// this message was forwarded by the TooltipView
			{
				// hide this window
				HideTooltip();
				
				BMessage forwardMessage = *message;
				
				// Convert point to coordinate system of owner.
				BPoint p = message->FindPoint("where");
				
				ConvertToScreen(&p);
				
				BAutolock lock(owner->Window());
				owner->ConvertFromScreen(&p);
				
				forwardMessage.ReplacePoint("where", p);
				forwardMessage.ReplacePoint("be:view_where", p);
				
				// forward mouseclick to owner
				BMessenger ownerMessenger(owner);
				ownerMessenger.SendMessage(&forwardMessage, (BHandler *)this);
			}
			break;
		default:
			BWindow::MessageReceived(message);
	}	
}