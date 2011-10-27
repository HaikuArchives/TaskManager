/*

	MakSplitterView - A class for creating your own splitter views.
	Leave a thin space between the two BViews you wish to seperate (usually
	about 5 or 6 pixels is enough), give it a colour (grey is the default)
	and off you go.
	
	
	You currently need to specify the thin Region you want it to take up,
	so make sure this is correct.
	
		-Created by Makhno
*/

#ifndef __MakSplitterView_H__
#define __MakSplitterView_H__

#define MAK_H_SPLITTER true
#define MAK_V_SPLITTER false

#include <View.h>
#include "MakCursors.h"

const rgb_color Mak_BeBackgroundGrey ={216,216,216,255};

class MakSplitterView:public BView
{
    private: 
    	bool mouseisdown;
    	BPoint oldpoint;
  		bool	h_or_v;
  		BView *lui,*rdi;	
  		void MoveViews(BPoint p);
  		
    public: 
        MakSplitterView (BRect,BView*,BView*,bool,uint32,rgb_color=Mak_BeBackgroundGrey);
		virtual void MouseMoved(BPoint,uint32,const BMessage*);
		virtual void MouseUp(BPoint);
		virtual void MouseDown(BPoint);
};

#endif
