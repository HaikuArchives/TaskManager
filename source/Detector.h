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
 
#ifndef CONTEXT_MENU_DETECTOR
#define CONTEXT_MENU_DETECTOR

// ====== Message IDs ======

const int32 MSG_CONTEXT_MENU				= 'mCXM';

// ====== Class Defs ======

//! Detects a context menu gesture.
// The user can open a context menu through tree actions:
// <OL>
// <LI>A long click with the primary mouse button (longer 1.5 secs).
// <LI>A right click.
// <LI>A left click with the control key pressed down.
// </OL>
// CContextMenuDetector tests for any of these actions and sends
// a MSG_CONTEXT_MENU to it's target when it detects one of them.
class CContextMenuDetector
{
	public:
	CContextMenuDetector(BView *view);
	virtual ~CContextMenuDetector();
	
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	
	protected:
	void StartWaitThread();
	void KillWaitThread();
	void SendMessage(BPoint point);
	
	int32 WaitThread();
	static int32 wait_thread_entry(void *p_this);
	
	static const bigtime_t DEFAULT_LONG_CLICK_TIME = 1500000;
	thread_id waitThread;
	BView *target;
	BPoint startPoint;
};

#endif // CONTEXT_MENU_DETECTOR