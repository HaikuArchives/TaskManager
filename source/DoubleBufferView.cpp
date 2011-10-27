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
 
#include "DoubleBufferView.h"

#include "my_assert.h"

// Constructor
// Creates the background working thread, but doesn't start it.
// Any derived class has to call 'StartThread' to actually start
// the background thread.
CDoubleBufferView::CDoubleBufferView(BRect frame, const char *name, 
	uint32 resizingMode, uint32 flags, bool disp_fps) :
	BView(frame, name, resizingMode, flags)
{
	display_fps = disp_fps;

	foreground_buf	= NULL;
	background_buf	= NULL;
	
	buffer_sem  = create_sem(1, "Buffer_BM_Sem");
	
	MY_ASSERT(buffer_sem >= B_NO_ERROR);
	
	bg_thread = spawn_thread(BackgroundThreadStartFunc, 
					"Bg_Thread", 
					suggest_thread_priority(B_LIVE_3D_RENDERING, 20, 1000, 50),
					this);

	MY_ASSERT(bg_thread >= B_NO_ERROR);

	stop_thread = false;
}

// Destructor
CDoubleBufferView::~CDoubleBufferView()
{
	// tell background thread to exit
	StopThread();
	
	// delete bitmaps
	delete foreground_buf;
	delete background_buf;
	
	// delete semaphore
	delete_sem(buffer_sem);
}

// StopThread
// this funktion must be called before any objects shared
// with the background thread are destroyed.
void CDoubleBufferView::StopThread()
{
	if(bg_thread < 0) {
		// No background thread
		return;
	}

	// tell thread to exit
	stop_thread = true;
	
	// wait for thread to exit
	status_t exit_value;
	wait_for_thread(bg_thread, &exit_value);
	
	bg_thread = -1;
}

// CreateBitmaps
// this function must be called, before the background thread starts.
void CDoubleBufferView::CreateBitmaps(BRect bufferFrame, color_space colorSpace)
{
	foreground_buf	= new BBitmap(bufferFrame, colorSpace);
	background_buf	= new BBitmap(bufferFrame, colorSpace);
}

// SwapBuffers
// Swap foreground and background buffer
void CDoubleBufferView::SwapBuffers()
{
	acquire_sem(buffer_sem);

	// swap buffers
	BBitmap *temp = foreground_buf;
	foreground_buf = background_buf;
	background_buf = temp;

	release_sem(buffer_sem);
}

int32 CDoubleBufferView::BackgroundThreadStartFunc(void *obj)
{
	((CDoubleBufferView *)obj)->BackgroundThread();
	
	return 0;
}

// BackgroundThread
// Background working thread. Calls CalcNextFrame to calculate 
// the next frame (in the background buffer) and swaps the buffers
// after that.
void CDoubleBufferView::BackgroundThread()
{
	bigtime_t calc_time, wait_time, start_time;
	
	bool swap_buffers;
	
	while(!stop_thread) {
		start_time = calc_time = system_time();
		swap_buffers = CalcNextFrame();
		calc_time = system_time()- calc_time;
		
		// If 'CalcNextFrame' returns false 
		// the background buffer isn't changed.
		// Don't change buffers.
		if(swap_buffers) {
			SwapBuffers();

			if(Window()->LockWithTimeout(100000) == B_OK) {
				Invalidate();
				Window()->Unlock();
			}
		}

		// Restrict to 20 fps
		wait_time = 50000 - calc_time;
		
		if(wait_time > 0)	snooze(wait_time);
		
		frame_time = system_time()-start_time;
	}
}

void CDoubleBufferView::AttachedToWindow()
{
	BView::AttachedToWindow();

	bg_color = Parent()->ViewColor();

	SetViewColor(CColor::Transparent);
}

void CDoubleBufferView::Draw(BRect updateRect) 
{
	SetHighColor(bg_color);

	BPoint bitmapPos(0,0);
	BRect viewRect   = Bounds();

	acquire_sem(buffer_sem);
	
	BRect bitmapRect = BRect(0,0,0,0);
	if (foreground_buf)
	{ 
	  bitmapRect = foreground_buf->Bounds();
	  DrawBitmap(foreground_buf, bitmapPos);
    }
    
	release_sem(buffer_sem);

	// rectangle right of the bitmap (down to the bottom)
	BRect r(viewRect.left + bitmapRect.Width(), 
			0, 
			viewRect.right, 
			viewRect.bottom);

	// rectangle beneath the bitmap
	BRect b(viewRect.left, 
			viewRect.top + bitmapRect.Height(),
			viewRect.left + bitmapRect.Width(),
			viewRect.bottom);

	FillRect(b);
	FillRect(r);

	// display fps
	if(display_fps) {
		char fps[255];
		
		double f = 1.0/((double)frame_time)*1E6;
		
		sprintf(fps, "%.2f fps", f);
	
		SetHighColor(0, 0, 0);	
		DrawString(fps, BPoint(4, viewRect.bottom-4));
	}
}
