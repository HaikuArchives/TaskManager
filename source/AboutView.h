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

#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include "pch.h"
#include "commondefines.h"
#include "DoubleBufferView.h"
#ifdef _TMGR_FULLBLUR
  #include "ABlur.h"
#endif
// simple interface class
class CFadeView : public BView
{
	public:
	CFadeView(BRect frame, const char *name, uint32 resizingMode, uint32 flags);
	virtual ~CFadeView() {}
	
	virtual void Fade(uchar step) { current_step = step; }
	
	protected:
	uchar current_step;
};

class CBeLogoView : public CFadeView
{
	public:
	CBeLogoView(BRect frame);
	
	virtual void Draw(BRect updateRect);
};

class CTaskManagerLogoView : public CFadeView
{
	public:
	CTaskManagerLogoView(BRect frame);
	
	virtual void Draw(BRect updateRect);
};

class CTitleBlurView : public CDoubleBufferView
{
	public:
	CTitleBlurView(BRect frame);
	virtual ~CTitleBlurView();
	
	virtual void AttachedToWindow();
	
	protected:
	virtual bool CalcNextFrame();

  #ifndef _TMGR_FULLBLUR  
	int32 current_frame;	
  #else
    float current_frame;	
  #endif

	CFadeView *beLogoView, *taskMgrLogoView;
	BBitmap *logo;	

  #ifdef _TMGR_FULLBLUR
	BBitmap *logo2;	
    CMixedEffect *effect;
  #endif    
};

class CAboutView : public BView
{
	public:
	CAboutView(BRect frame, uint32 resizingMode=B_FOLLOW_ALL_SIDES, 
		uint32 flags=0);
		
	virtual void AttachedToWindow();
	virtual void FrameResized(float width, float height);
	virtual void GetPreferredSize(float *width, float *height);
	protected:
};

#endif // ABOUT_VIEW_H