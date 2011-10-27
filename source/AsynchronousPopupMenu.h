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
 
#ifndef ASYNCHRONOUS_POPUP_MENU_H
#define ASYNCHRONOUS_POPUP_MENU_H

// ====== Message IDs ======

const int32 MSG_MENU_BEGINNING				= 'mMEB';
const int32 MSG_MENU_ENDED					= 'mMEE';

// ====== Class Defs ======

// This PopUp menu notifies a target when it's closed.
// It can only be used in the asynchonous mode.
class CAsynchronousPopUpMenu : public BPopUpMenu
{
	public:
	CAsynchronousPopUpMenu(const char *name, bool radioMode=true,
		bool labelFromMarked=true, menu_layout layout=B_ITEMS_IN_COLUMN);
	virtual ~CAsynchronousPopUpMenu();

	void SetNotificationTarget(BMessenger *messenger);
	void SetNotificationTarget(BHandler *handler, BLooper *looper=NULL);

	void Go(BPoint screenPoint, bool openAnyway=false, bool autoDelete=true);
	void Go(BPoint screenPoint, bool openAnyway, BRect clickToOpenRect, bool autoDelete=true);

	protected:
	struct wait_thread_info
	{
		CAsynchronousPopUpMenu *popupMenu;
		BPoint screenPoint;
		bool openAnyway;
		BRect clickToOpenRect;
		bool autoDelete;
	};

	void StartWaitThread(wait_thread_info *data);

	static int32 ThreadFunc(void *data);
	
	BMenuItem *Go(BPoint screenPoint, bool deliversMessage=false, 
		bool openAnyway=false, bool asynchronous=false);
	BMenuItem *Go(BPoint screenPoint, bool deliversMessage, 
		bool openAnyway, BRect clickToOpenRect, bool asynchronous=false);
		
	BMessenger *notifyTarget;
};

#endif // ASYNCHRONOUS_POPUP_MENU_H