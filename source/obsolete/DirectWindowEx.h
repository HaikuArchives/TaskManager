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
 
#ifndef DIRECT_WINDOW_EX_H
#define DIRECT_WINDOW_EX_H

// !!!ADD GAMESKIT DLL TO PROJECT!!!

#include <game/DirectWindow.h>

class CDirectWindowEx : public BDirectWindow
{
	CDirectWindowEx(BRect frame, const char *title, window_type type,
		uint32 flags, uint32 workspaces=B_CURRENT_WORKSPACE);
	virtual ~CDirectWindowEx();

	virtual void DrawFrame() = 0;
	virtual void Connected() = 0;
	
	protected:
	virtual void DirectConnected(direct_buffer_info *info);

	static int32 StartDrawLoop(void *data);
	int32 DrawLoop();

	bool			connected;
	bool			disabled;

	sem_id			draw_data_sem;
	thread_id		draw_thread;
	int32			num_rects;
	clipping_rect  *rects;
	clipping_rect	window_bounds;
	color_space		pixel_format;
	uchar		   *framebuffer;
	int32			bytes_per_row;
};

CDirectWindowEx::CDirectWindowEx(
		BRect frame,
		const char *title,
		window_type type,
		uint32 flags,
		uint32 workspaces) :
	BDirectWindow(frame, title, type, flags, workspaces)
{
	disabled		= false;
	connected		= false;
	num_rects		= 0;
	rects			= NULL;
	pixel_format	= B_NO_COLOR_SPACE;
	framebuffer		= NULL;
	bytes_per_row	= 0;

	draw_data_sem = create_sem(1, "draw_data_sem");
	
	draw_thread = spawn_thread(StartDrawLoop, "drawing thread",
								B_NORMAL_PRIORITY, this);
	resume_thread(draw_thread);
}

CDirectWindowEx::~CDirectWindowEx()
{
	Hide();
	Sync();
	
	disabled = true;
	int32 thread_result;

	wait_for_thread(draw_thread, &thread_result);
	
	delete_sem(draw_data_sem);
	
	delete rects;
}

void CAboutWindow::DirectConnected(direct_buffer_info *info)
{
	if (!connected && disabled)
		return;

	acquire_sem(draw_data_sem);

	switch(info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
			connected = true;
			/* fall through */
		case B_DIRECT_MODIFY:
			{
				framebuffer = (uchar *)info->bits;
				bytes_per_row = info->bytes_per_row;
				pixel_format = info->pixel_format;
	
				window_bounds = info->window_bounds;
	
				num_rects = info->clip_list_count;
	
				delete rects;
				
				rects = new rects[num_rects];
				
				memcpy(&info->clip_list[0], rects, sizeof(clipping_rect)*num_rects);

				Connected();
				
				/*
				BRect dest_rect(window_bounds.left,
								window_bounds.top,
								window_bounds.left+200,
								window_bounds.top+100);
								
				rects = CBlur::ClipRects(&info->clip_list[0], num_rects, dest_rect, 30, 30);
				*/
			}		
			break;
		case B_DIRECT_STOP:
			connected = false;
			break;
	}
	
	release_sem(draw_data_sem);
}

int32 CDirectWindowEx::StartDrawLoop(void *data)
{
	return static_cast<CDirectWindowEx *>(data)->DrawLoop();
}

int32 CDirectWindowEx::DrawLoop()
{
	while (!disabled) {
		acquire_sem(draw_data_sem);

		if (connected) {
			/*
			clipping_rect dest_rect;
			
			dest_rect.left  = window_bounds.left;
			dest_rect.top   = window_bounds.top;
			dest_rect.right = window_bounds.left+200;
			dest_rect.bottom = window_bounds.left+100;
		
			CBlur::Blur(tst, framebuffer, bytes_per_row,
				pixel_format, &dest_rect, rects, num_rects, 10, 10);
			*/
		
			DrawFrame();
		}

		release_sem(draw_data_sem);

		snooze(20000);
	}
	
	return 0;
}

#endif // DIRECT_WINDOW_EX_H