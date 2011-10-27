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


#include <Looper.h>

#include "MouseDragOrPopupWatcher.h"


int32 MouseDragOrPopupWatcher(void* data);


thread_id StartMouseDragOrPopupWatcher(BView* TargetView)
{
	thread_id MouseWatcherThread = spawn_thread(MouseDragOrPopupWatcher,"MouseDragOrPopupWatcher",
		B_NORMAL_PRIORITY,new BMessenger(TargetView));
	if(MouseWatcherThread != B_NO_MORE_THREADS && MouseWatcherThread != B_NO_MEMORY)
		resume_thread(MouseWatcherThread);
	return MouseWatcherThread;
}


int32 MouseDragOrPopupWatcher(void* data)
{
	BMessenger* TheMessenger = (BMessenger*)data;
	BPoint StartPos;
	uint32 StartButtons = 0;
	bigtime_t StartTime = 0;
	bool FirstCheck = true;
	BMessage MessageToSend;
	MessageToSend.AddPoint("where",BPoint(0,0));
	MessageToSend.AddInt32("buttons",0);
	MessageToSend.AddInt32("modifiers",0);
	bigtime_t PopupTime;
	if(get_click_speed(&PopupTime) != B_OK)
		return 0;
	PopupTime *= 2;
	while(true)
	{
		if (!TheMessenger->LockTarget())
		{
			delete TheMessenger;
			return 0;			// window is dead so exit
		}
		BLooper *TheLooper;
		BView* TheView = (BView*)TheMessenger->Target(&TheLooper);
		BPoint Where;
		uint32 Buttons;
		bigtime_t Time = system_time();
		TheView->GetMouse(&Where,&Buttons,false);
		if(FirstCheck)
		{
			StartPos = Where;
			StartButtons = Buttons;
			StartTime = Time;
			FirstCheck = false;
		}
		bool Send = false;
		if(Buttons == 0 || Buttons != StartButtons)
		{
			//Changed or released
			MessageToSend.what = MW_MOUSE_CLICK;
			Send = true;
		}
		else if(Where.x < StartPos.x-1.0 || Where.x > StartPos.x+1.0 ||
			Where.y < StartPos.y-1.0 || Where.y > StartPos.y+1.0)
		{
			MessageToSend.what = MW_MOUSE_DRAG;
			Send = true;
		}
		else if(Time >= StartTime + PopupTime)
		{
			MessageToSend.what = MW_MOUSE_POPUP;
			Send = true;
		}
		TheLooper->Unlock();
		if(Send)
		{
			MessageToSend.ReplacePoint("where",StartPos);
			MessageToSend.ReplaceInt32("buttons",Buttons);
			MessageToSend.ReplaceInt32("modifiers",modifiers());
			TheMessenger->SendMessage(&MessageToSend);
			delete TheMessenger;
			return 0;
		}
		snooze(50000);
	}
}
