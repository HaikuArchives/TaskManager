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

#include "commondefines.h"
#include "pch.h"
#include "help.h"
#include "DoubleBufferView.h"
#include "URLTextView.h"
#ifndef _TMGR_FULLBLUR
  #include "Blur.h"
#else
  #include "ABlur.h"
  #include "ADither.h"  
#endif
#include "AboutView.h"

#include "version.h"
#include "my_assert.h"

CFadeView::CFadeView(BRect frame, const char *name, uint32 resizingMode, 
	uint32 flags) :
	BView(frame, name, resizingMode, flags)
{
	current_step = 0;
}

CBeLogoView::CBeLogoView(BRect frame) :
	CFadeView(frame, "BeLogoView", B_FOLLOW_LEFT, B_WILL_DRAW)
{
}

void CBeLogoView::Draw(BRect updateRect)
{
	// erase background
	SetHighColor(255,255,255);
	FillRect(updateRect);

	BFont big_font;
	
	big_font.SetFamilyAndStyle("Swis721 BT", "Roman");
	big_font.SetSize(42);
	
	font_height big_font_height;
	
	big_font.GetHeight(&big_font_height);

	const char displayString[] = "Be";

	float stringWidth = big_font.StringWidth(displayString);
	
	// center string
	MovePenTo((Bounds().Width()-stringWidth)/2, 
			  (Bounds().Height()-big_font_height.ascent)/2+big_font_height.ascent);
	
	SetFont(&big_font);
	SetHighColor(current_step, current_step, 255, 255);
	DrawString(&displayString[0], 1); // "B"
	SetHighColor(255, current_step, current_step, 255);
	DrawString(&displayString[1], 1); // "e"
	
	Sync();
}

CTaskManagerLogoView::CTaskManagerLogoView(BRect frame) :
	CFadeView(frame, "TaskManagerLogoView", B_FOLLOW_LEFT, B_WILL_DRAW)
{
}

void CTaskManagerLogoView::Draw(BRect updateRect)
{
	// erase background
	SetHighColor(255,255,255);
	FillRect(updateRect);

	BFont big_font;
	
	big_font.SetFamilyAndStyle("Swis721 BT", "Roman");
	big_font.SetSize(27);
	
	font_height big_font_height;
	
	big_font.GetHeight(&big_font_height);

	const char displayString[] = "TaskManager";

	float stringWidth = big_font.StringWidth(displayString);
	
	// center string
	MovePenTo((Bounds().Width()-stringWidth)/2, 
			  (Bounds().Height()-big_font_height.ascent)/2+big_font_height.ascent);
	
	SetFont(&big_font);
	SetHighColor(current_step, current_step, current_step, 255);
	DrawString(displayString);
	
	Sync();
}

CTitleBlurView::CTitleBlurView(BRect frame) : 
	CDoubleBufferView(frame, "TitleBlurView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	logo = new BBitmap(Bounds(), B_RGB32, true);
  #ifdef _TMGR_FULLBLUR
    logo2= new BBitmap(Bounds(), B_RGB32, true);
  #endif
  
	beLogoView		= new CBeLogoView(Bounds());
	taskMgrLogoView	= new CTaskManagerLogoView(Bounds());

	logo->AddChild(beLogoView);
  #ifndef _TMGR_FULLBLUR
	logo->AddChild(taskMgrLogoView);
  #else
	logo2->AddChild(taskMgrLogoView);  
  #endif
  
	current_frame = 0;

  #ifdef _TMGR_FULLBLUR
    if(logo->Lock()) {
		beLogoView->Draw(logo->Bounds());
		logo->Unlock();
	}
    if(logo2->Lock()) {
		taskMgrLogoView->Draw(logo2->Bounds());
		logo2->Unlock();
	}
    effect=new CMixedEffect(logo,logo2);
  #endif    
}

CTitleBlurView::~CTitleBlurView()
{
	// tell background thread to exit
	StopThread();

	// delete views
	if(logo->Lock()) {
		beLogoView->RemoveSelf();
  #ifndef _TMGR_FULLBLUR
		taskMgrLogoView->RemoveSelf();
  #endif
		logo->Unlock();
	}
  #ifdef _TMGR_FULLBLUR
	if(logo2->Lock()) {
		taskMgrLogoView->RemoveSelf();
		logo2->Unlock();
	}
	if (effect) delete effect;
  #endif

	delete beLogoView;
	delete taskMgrLogoView;
	
	// delete bitmap
  #ifndef _TMGR_FULLBLUR
	delete logo;
  #endif
}

void CTitleBlurView::AttachedToWindow()
{
	CDoubleBufferView::AttachedToWindow();

  #ifndef _TMGR_FULLBLUR
	color_space buffer_color_space;
	color_space screen_color_space = BScreen(Window()).ColorSpace();

	if(CBlur::SupportedOutputColorSpace(screen_color_space)) {
		// color space is directly supported.
		// this avoids color space transformation
		// during the blit.
		buffer_color_space = screen_color_space;
	} else {
		buffer_color_space = B_RGB32; 
	}

	CreateBitmaps(Bounds(), buffer_color_space);
  #endif
	StartThread();
}

#ifndef _TMGR_FULLBLUR

bool CTitleBlurView::CalcNextFrame()
{
	bool result = true;
	
	if(current_frame >= 0 && current_frame <= 29)
	{
		// fade IN Be logo
		beLogoView->Fade((255/30.0) * (30-current_frame));

		if(logo->Lock()) {
			beLogoView->Draw(logo->Bounds());
			logo->Unlock();
		}
		
		int mx = (30-current_frame)/2*2+1;
		
		CBlur::Blur(logo, background_buf, mx, 1);
	} else if(current_frame >= 30 && current_frame <= 39) {
		// sleep for one frame
		result = false;
	} else if(current_frame >= 40 && current_frame <= 69) {
		// fade OUT Be logo
		beLogoView->Fade((255/30) * (current_frame-39));

		if(logo->Lock()) {
			beLogoView->Draw(logo->Bounds());
			logo->Unlock();
		}

		int mx = (current_frame-39)/2*2+1;
		
		CBlur::Blur(logo, background_buf, mx, 1);
	} else if(current_frame >= 70 && current_frame <= 99) {
		// fade IN Taskmgr logo
		taskMgrLogoView->Fade((255/30.0) * (99-current_frame));

		if(logo->Lock()) {
			taskMgrLogoView->Draw(logo->Bounds());
			logo->Unlock();
		}
		
		int my = (99-current_frame)/4*2+1;
		
		CBlur::Blur(logo, background_buf, 1, my);		
	} else if(current_frame >= 100 && current_frame <= 109) {
		// sleep for one frame
		result = false;
	} else if(current_frame >= 110 && current_frame <= 139) {
		// fade IN Taskmgr logo
		taskMgrLogoView->Fade((255/30.0) * (current_frame-109));

		if(logo->Lock()) {
			taskMgrLogoView->Draw(logo->Bounds());
			logo->Unlock();
		}
		
		int my = (current_frame-109)/4*2+1;
		
		CBlur::Blur(logo, background_buf, 1, my);		
	} else {
		// restart cycle
		current_frame = -1;
	}

	current_frame++;
	
	return result;
}

#else // #ifndef _TMGR_FULLBLUR

bool CTitleBlurView::CalcNextFrame()
{ 
	// current_frame ranges from 0.0 to 1.0+1.0+1.0+1.0=4.0
	current_frame+=0.04;
	
	// restart cycle
	if (current_frame>4.0)
		current_frame=0.0;
	
	// delay. don't swap buffers.
	if ((current_frame<=1.0) && (foreground_buf))
	    return false;
	    
	// Fade in Be and fade out TaskManager
	if (current_frame<=2.0)
	{ 
		if (background_buf)
		  delete background_buf;
    
    	background_buf=effect->GetEffect(current_frame-1.0);
    	if (BScreen().ColorSpace()!=B_RGB32)
    	{ 
    	  CDither dither;
    	  BBitmap *tmp=dither.GetAsScreenColorSpace(*background_buf);
    	  delete background_buf;
    	  background_buf=tmp;
    	}
    	return true;
	}
	
	// delay. don't swap buffers.
	if (current_frame<=3.0)
		return false;
		
	// Fade in TaskManager and fade out Be
	if (background_buf)
	  delete background_buf;
		
	background_buf=effect->GetEffect(4.0-current_frame);
  	if (BScreen().ColorSpace()!=B_RGB32)
  	{ CDither dither;
  	  BBitmap *tmp=dither.GetAsScreenColorSpace(*background_buf);
  	  delete background_buf;
   	  background_buf=tmp;
  	}
	
	return true;
}

#endif // #ifndef (else) _TMGR_FULLBLUR

CAboutView::CAboutView(BRect frame, uint32 resizingMode, uint32 flags) :
	BView(frame, "AboutView", resizingMode, flags)
{
	// Create sub-views on dummy positions, but with correct size.
	// They are moved to their correct positions in FrameResized().
	const float blurViewHeight = 70;

	BRect blurViewRect(0, 0, 200, blurViewHeight);
	BRect stringViewRect(0, blurViewHeight+1, Bounds().Width(), Bounds().Height()); 
	BRect textRect(0,0,Bounds().Width(), Bounds().Height()-blurViewHeight-1);		

	AddChild(new CTitleBlurView(blurViewRect));
	
	CURLTextView *textView = new CURLTextView(stringViewRect, 
										"Copyright_View", 
										textRect, 
										B_FOLLOW_ALL,
										B_FRAME_EVENTS | B_WILL_DRAW);

	textView->SetWordWrap(true);	
	textView->SetAlignment(B_ALIGN_CENTER);

	BString creditsURL;
	
	creditsURL << "file://" << BPath(get_app_dir().Path(), "doc/credits.html").Path();

	textView->AddText("Be TaskManager V" TASKMANAGER_VERSION_STRING);
	textView->AddText("\n\n" B_UTF8_COPYRIGHT " 1999-2002 by ");
	textView->AddAnchor("Thomas Krammer", "mailto:tkrammer@3rd-evolution.de");
	textView->AddText("\n\n");
	textView->AddAnchor("View credits", creditsURL.String());
	textView->AddText(".\n\nThis program is FREEWARE.\n\n");
	textView->AddAnchor("Visit the TaskManager Homepage", "http://www.3rd-evolution.de/BeOS/TaskMgr/");

	AddChild(textView);
}

void CAboutView::AttachedToWindow()
{
	BView::AttachedToWindow();

	SetViewColor(CColor::White);
	
	FrameResized(Bounds().Width(), Bounds().Height());
}

void CAboutView::FrameResized(float width, float height)
{
	float centerX = (width-200)/2;

	float blurViewHeight = ChildAt(0)->Bounds().Height();
	
	ChildAt(0)->MoveTo(centerX, 0);
	ChildAt(1)->MoveTo(0, blurViewHeight+1);
	ChildAt(1)->ResizeTo(width, height-blurViewHeight-1);
}

void CAboutView::GetPreferredSize(float *width, float *height)
{
	if(width)
		*width = Bounds().Width();
	
	if(height) {
		float blurViewHeight = 
			dynamic_cast<CTitleBlurView *>(ChildAt(0))->Bounds().Height();

		// This works because the range passed to TextHeight is automatically
		// restricted to a valid range. This means this call measures the 
		// height of ALL text.
		float textViewHeight = 
			dynamic_cast<CURLTextView *>(ChildAt(1))->TextHeight(0, 100000);
	
		*height = blurViewHeight + textViewHeight + 1;
	}
}