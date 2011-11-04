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

#ifndef SELECT_TEAM_WINDOW_H
#define SELECT_TEAM_WINDOW_H

#include "Singleton.h"
#include "BorderView.h"

class CSelectTeamWindow : public CSingletonWindow
{
	public:
	static CSelectTeamWindow *CreateInstance();
	
	virtual ~CSelectTeamWindow();
	
	void SetTarget(BView *target);

	protected:
	CSelectTeamWindow();
	
	friend class CSingleton;
};

class CSelectTeamWindowView : public BView
{
	public:
	CSelectTeamWindowView(BRect frame);
	
	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	
	protected:
	BTextView *textView;
	BView *crossHair;
	CBorderView *crossHairBorder;
	
	static const float dist;
};

class CCrossHairDragView : public BView
{
	public:
	CCrossHairDragView(BRect frame);
	virtual ~CCrossHairDragView();
	
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);
	virtual void MouseUp(BPoint where);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	
	void SetTarget(BMessenger messenger);
	
	protected:
	
	team_id FindTeam(const BPoint &screenPoint);
	
	BBitmap *crossHair;
	
	// Target for MSG_TEAM_ACTION messages
	// (normally a CProcessView object)
	BMessenger target;
	
	bool isDragged;
};

class CCrossHairView : public BView
{
	public:
	CCrossHairView(BRect frame);
	
	virtual void Draw(BRect updateRect);
};

/****************************************************************************
** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **
**                                                                         **
**                          DANGER, WILL ROBINSON!                         **
**                                                                         **
** The rest of the interfaces contained here are part of BeOS's            **
**                                                                         **
**                     >> PRIVATE NOT FOR PUBLIC USE <<                    **
**                                                                         **
**                                                       implementation.   **
**                                                                         **
** These interfaces              WILL CHANGE        in future releases.    **
** If you use them, your app     WILL BREAK         at some future time.   **
**                                                                         **
**                                                                         **
** WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING **
****************************************************************************/

// from interface_defs.h
struct window_info {
	team_id		team;
    int32   	id;	  		  /* window's token */

	int32		thread;
	int32		client_token;
	int32		client_port;
	uint32		workspaces;

	int32		layer;
    uint32	  	w_type;    	  /* B_TITLED_WINDOW, etc. */
    uint32      flags;	  	  /* B_WILL_FLOAT, etc. */
	int32		window_left;
	int32		window_top;
	int32		window_right;
	int32		window_bottom;
	int32		show_hide_level;
	bool		is_mini;
	char		name[1];
};

// from interface_misc.h
enum window_action {
	B_MINIMIZE_WINDOW,
	B_BRING_TO_FRONT
};

// from interface_misc.h
void		do_window_action(int32 window_id, int32 action, 
							 BRect zoomRect, bool zoom);
window_info	*get_window_info(int32 a_token);
int32		*get_token_list(team_id app, int32 *count);


#endif // SELECT_TEAM_WINDOW_H
