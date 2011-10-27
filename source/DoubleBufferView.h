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
 
#ifndef DOUBLE_BUFFER_VIEW_H
#define DOUBLE_BUFFER_VIEW_H
#include "pch.h"

class CDoubleBufferView : public BView
{
	public:
	CDoubleBufferView(BRect frame, const char *name, 
			uint32 resizingMode, uint32 flags,  
			bool disp_fps=false);

	virtual ~CDoubleBufferView();

	virtual void Draw(BRect updateRect);
	virtual void AttachedToWindow();
			
	protected:
	virtual void BackgroundThread();
	virtual bool CalcNextFrame() = 0;

	void StartThread() { resume_thread(bg_thread); }
	void StopThread();
	void CreateBitmaps(BRect bufferFrame, color_space colorSpace);
	void SwapBuffers();

	static int32 BackgroundThreadStartFunc(void *obj);

	BBitmap *	foreground_buf;		// current frame
	BBitmap *	background_buf;		// background buffer
	sem_id	 	buffer_sem;			// semaphore protecting buffers
	thread_id	bg_thread;			// background thread calculating the next frame
	bool		stop_thread;		// boolean flag telling the background thread to exit
	bigtime_t	frame_time;			// time for the last frame (calculation only, without blit)
	bool		display_fps;		// display frames per sec. counter
	rgb_color	bg_color;			// fill color for areas not covered by bitmap
};

#endif // DOUBLE_BUFFER_VIEW_H