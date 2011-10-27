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

#ifndef TOOLTIP_H
#define TOOLTIP_H

// ====== Types ======

enum enumToolTipCorner
{
	TTC_NONE,
	TTC_LEFT_BOTTOM,
	TTC_RIGHT_BOTTOM,
	TTC_LEFT_TOP,
	TTC_RIGHT_TOP,
};

enum enumMouseWatcherCodes
{
	MOUSE_WATCHER_MOUSE_MOVED,
	MOUSE_WATCHER_MOUSE_DOWN,	// currently unsupported
	MOUSE_WATCHER_MOUSE_UP,		// currently unsupported
};

// ====== Message IDs ======

const int32 MSG_MOUSE_WATCHER				= 'mMWA';

// ====== Class Defs ======

class CTimer
{
	public:
	CTimer(bigtime_t _period, bool _singlePing, BInvoker *_invoker);
	virtual ~CTimer();
	
	void RestartTimer();
	void StopTimer();
	
	protected:
	static int32 ThreadStartFunc(void *data);
	int32 TimerThread();
	
	thread_id	thread;
	bigtime_t	period;
	bool		singlePing;
	BInvoker *	invoker;
};

class CMouseWatcher
{
	public:
	CMouseWatcher(BMessenger *_target);
	virtual ~CMouseWatcher();

	void StartWatching();
	
	protected:
	void CreateWatcherThread();
	void WatcherThread();
	
	static int32 StartWatcherThread(void *obj);
	
	BMessenger *target;
	bool exitLoop;
	thread_id watcherThread;
};

class CTooltipView : public BView
{
	public:
	CTooltipView(BRect frame);
	virtual ~CTooltipView();
	
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint point);
	
	virtual void GetPreferredSize(float *width, float *height);
	
	void SetText(const char *text);
	const char *Text() { return tipText; }

	protected:
	char *tipText;
};

class CTooltip : public BWindow
{
	public:
	CTooltip(BView *_owner);
	virtual ~CTooltip();
	
	virtual bool Visible();
	virtual void ShowTooltip(const char *text, BRect screenRect, 
		BPoint screenPoint, enumToolTipCorner corner);
	virtual void HideTooltip();
	virtual void MessageReceived(BMessage *message);
	
	BView *Owner() { return owner; }
	void SetOwner(BView *newOwner) { owner = newOwner; }
	
	protected:
	BRect toolRect;
	CMouseWatcher *mouseWatcher;
	BView *owner;
};

#endif // TOOLTIP_H