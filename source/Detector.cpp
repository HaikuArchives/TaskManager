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
#include "Detector.h"

// ==== CContextMenuDetector ====

CContextMenuDetector::CContextMenuDetector(BView *view)
{
	target     = view;
	waitThread = -1;
}

CContextMenuDetector::~CContextMenuDetector()
{
	// WARNING: This isn't totally safe. It can 
	// happen that the thread still holds a lock to the
	// window when it's killed.
	KillWaitThread();
}

void CContextMenuDetector::MouseDown(BPoint point)
{
	BMessage *message = target->Window()->CurrentMessage();

	int32 modifiers = message->FindInt32("modifiers");
	int32 buttons   = message->FindInt32("buttons");
	int32 clicks	= message->FindInt32("clicks");
	
	if(clicks == 1) {
		// single click
		if(buttons == B_PRIMARY_MOUSE_BUTTON) {
			if(modifiers & B_CONTROL_KEY) {
				// left click with option key down.
				SendMessage(point);
			} else {
				// left click -- can be a long click.
				startPoint = point;
				StartWaitThread();
			}
		} else if(buttons == B_SECONDARY_MOUSE_BUTTON) {
			// right click
			SendMessage(point);
		}
	}
}

void CContextMenuDetector::MouseUp(BPoint point)
{
	// This is a safe thing to do, because in order to
	// receive a message the window must be locked.
	// Before the WaitThread() does anything critical
	// it locks the window. So the thread won't leak or
	// hold a lock when it's killed.
	KillWaitThread();
}

void CContextMenuDetector::SendMessage(BPoint point)
{
	BMessenger messenger(target);
	
	if(messenger.IsValid()) {
		BMessage message(MSG_CONTEXT_MENU);
		message.AddPoint("where", point);
	
		messenger.SendMessage(&message, (BHandler *)NULL);
	}
}

int32 CContextMenuDetector::WaitThread()
{
	bigtime_t longClickTime;
	
	if(get_click_speed(&longClickTime) != B_OK)
		longClickTime = DEFAULT_LONG_CLICK_TIME;

	snooze(longClickTime);
	
	BPoint point;
	uint32 buttons;
	
	target->Window()->Lock();
	target->GetMouse(&point, &buttons, false);
	
	// Mouse button still down??
	if(buttons == B_PRIMARY_MOUSE_BUTTON) {
		// Mouse moved?

		// TODO: test if mouse was moved!
	
		SendMessage(point);
	}
	
	waitThread = -1;
	
	target->Window()->Unlock();
	
	return 0;
}

void CContextMenuDetector::StartWaitThread()
{
	if(waitThread >= 0)
		KillWaitThread();
		
	waitThread = spawn_thread(wait_thread_entry, "ContextMenuDetector",
						B_NORMAL_PRIORITY, this);
						
	if(waitThread >= 0)
		resume_thread(waitThread);
}

void CContextMenuDetector::KillWaitThread()
{
	if(waitThread >= 0)
		kill_thread(waitThread);
}

int32 CContextMenuDetector::wait_thread_entry(void *p_this)
{
	return static_cast<CContextMenuDetector *>(p_this)->WaitThread();
}

