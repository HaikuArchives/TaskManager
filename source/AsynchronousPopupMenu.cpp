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
#include "msg_helper.h"
#include "AsynchronousPopupMenu.h"

CAsynchronousPopUpMenu::CAsynchronousPopUpMenu(
	const char *name, bool radioMode, bool labelFromMarked, 
	menu_layout layout) :
	BPopUpMenu(name, radioMode, labelFromMarked, layout)
{
	notifyTarget = NULL;
}

CAsynchronousPopUpMenu::~CAsynchronousPopUpMenu()
{
	delete notifyTarget;
}

void CAsynchronousPopUpMenu::SetNotificationTarget(BMessenger *messenger)
{
	delete notifyTarget;
	notifyTarget = messenger;
}

void CAsynchronousPopUpMenu::SetNotificationTarget(BHandler *handler, BLooper *looper)
{
	SetNotificationTarget(new BMessenger(handler, looper));
}

// Go starts a thread, which opens the popupmenu and waits for it's close.
// When the menu is closed that thread sends a message to the notification
// target. Because that thread modifies and even deletes the popup menu 
// (when autoDelete is set) the caller should not modify the
// CAsynchronousPopUpMenu object after Go was called.
void CAsynchronousPopUpMenu::Go(BPoint screenPoint, bool openAnyway, bool autoDelete)
{
	wait_thread_info *waitThreadInfo = new wait_thread_info;
	
	waitThreadInfo->popupMenu		= this;
	waitThreadInfo->screenPoint		= screenPoint;
	waitThreadInfo->openAnyway		= openAnyway;
	waitThreadInfo->clickToOpenRect	= BRect(-1, -1, -1, -1);
	waitThreadInfo->autoDelete		= autoDelete;
	
	StartWaitThread(waitThreadInfo);
}

void CAsynchronousPopUpMenu::Go(BPoint screenPoint, bool openAnyway, BRect clickToOpenRect, bool autoDelete)
{
	wait_thread_info *waitThreadInfo = new wait_thread_info;
	
	waitThreadInfo->popupMenu		= this;
	waitThreadInfo->screenPoint		= screenPoint;
	waitThreadInfo->openAnyway		= openAnyway;
	waitThreadInfo->clickToOpenRect	= clickToOpenRect;
	waitThreadInfo->autoDelete		= autoDelete;
	
	StartWaitThread(waitThreadInfo);
}

void CAsynchronousPopUpMenu::StartWaitThread(wait_thread_info *data)
{
	thread_id waitThreadId = spawn_thread(ThreadFunc, "AsyncPopupMenu", 
		B_NORMAL_PRIORITY, data);
	
	if(waitThreadId >= B_OK) {
		resume_thread(waitThreadId);
	}
}

// Hidden from the user.
BMenuItem *CAsynchronousPopUpMenu::Go(BPoint screenPoint, bool deliversMessage, 
	bool openAnyway, bool asynchronous)
{
	return BPopUpMenu::Go(screenPoint, deliversMessage, openAnyway, asynchronous);
}

// Hidden from the user.
BMenuItem *CAsynchronousPopUpMenu::Go(BPoint screenPoint, bool deliversMessage, 
	bool openAnyway, BRect clickToOpenRect, bool asynchronous)
{
	return BPopUpMenu::Go(screenPoint, deliversMessage, openAnyway, 
		clickToOpenRect, asynchronous);
}

int32 CAsynchronousPopUpMenu::ThreadFunc(void *data)
{
	wait_thread_info *info = reinterpret_cast<wait_thread_info *>(data);

	BMessenger *messenger = info->popupMenu->notifyTarget;

	// send notification message, before menu is displayed.
	if(messenger) {
		BMessage openNotify(MSG_MENU_BEGINNING);

		AddTime(&openNotify, "when", system_time());
		openNotify.AddPointer("source", info->popupMenu);
	
		messenger->SendMessage(&openNotify);
	}

	BMenuItem *item = NULL;

	// call SYNCHRONOUS version of Go() and wait until the 
	// the function returns.
	if(info->clickToOpenRect == BRect(-1,-1,-1,-1)) {
		// No clickToOpenRect supplied.
		item = info->popupMenu->Go(info->screenPoint, true, 
				info->openAnyway, false);
	} else {
		item = info->popupMenu->Go(info->screenPoint, true, 
				info->openAnyway, info->clickToOpenRect, false);
	}
	
	// Menu was closed. Send notification message.
	if(messenger) {
		BMessage closeNotify(MSG_MENU_ENDED);

		AddTime(&closeNotify, "when", system_time());
		closeNotify.AddPointer("source", info->popupMenu);
		closeNotify.AddPointer("sel_item", item);
	
		messenger->SendMessage(&closeNotify);
	}
	
	// If autoDelete flag is set delete popup menu.
	if(info->autoDelete) {
		delete info->popupMenu;
	}
	
	delete info;
	
	return 0;
}