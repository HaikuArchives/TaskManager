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

/****DOCUMENTATION
You probably don't need to use this class.  BView now allows you to continue receiving mouse move and 
button events while the mouse button is held down after the pointer has left the BView following a 
click-drag in the BView, if you ask for them.  Before this functionality was added, MouseWatcher was 
needed, but no longer.  Just start a timer until the menu click-hold-popup time elapses, and if the 
button is still down and the mouse hasn't moved, pop up the menu.

Once started, MouseDragOrPopupWatcher will watch the mouse for twice the double-click time.  If the
primary mouse button is held down and the mouse is not moved more than one pixel in that time, it will
send a MW_MOUSE_POPUP message to the target view.  If the mouse moves before then, it will send a
MW_MOUSE_DRAG message, and if the mouse button is released before either of those, then a
MW_MOUSE_CLICK message is sent. (TargetView is specified as the target handler in the
BMessenger used to send the messages.  The BLooper == window of the target view is determined
automatically by the BMessenger)

These messages will have two data entries:

"where" (B_POINT_TYPE)		- The position of the mouse in TargetView's coordinate system.
"buttons" (B_INT32_TYPE)	- The mouse buttons.  See BView::GetMouse().
"modifiers" (B_INT32_TYPE)	- The modifier keys held down at the time.  See modifiers().

Once it is started, you can't stop it, but that shouldn't matter - the user will most likely release
the buttons soon, and you can interpret the events however you want.

StartMouseDragOrPopupWatcher returns a valid thread ID, or it returns an error code:
B_NO_MORE_THREADS. all thread_id numbers are currently in use.
B_NO_MEMORY. Not enough memory to allocate the resources for another thread.
****/

#ifndef _SGB_MOUSE_DRAG_POPUP_WATCHER_H_
#define _SGB_MOUSE_DRAG_POPUP_WATCHER_H_


//******************************************************************************************************
//**** SYSTEM HEADER FILES
//******************************************************************************************************
#include <View.h>


//******************************************************************************************************
//**** TYPE DEFINITIONS AND CONSTANTS
//******************************************************************************************************
const uint32 MW_MOUSE_DRAG = 'Mw-d';
const uint32 MW_MOUSE_POPUP = 'Mw-P';
const uint32 MW_MOUSE_CLICK = 'Mw-C';


//******************************************************************************************************
//**** FUNCTION DECLARATIONS
//******************************************************************************************************
thread_id StartMouseDragOrPopupWatcher(BView* TargetView);

#endif
