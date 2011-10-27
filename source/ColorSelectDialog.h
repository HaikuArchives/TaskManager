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
 
#ifndef COLOR_SELECT_DIALOG_H
#define COLOR_SELECT_DIALOG_H

#include "DialogBaseEx.h"

// ====== Message IDs =======

const int32 MSG_COLOR_SELECTED				= 'mCSE';
const int32 MSG_COLOR_SELECTOR_CLOSED		= 'mCSC';

// ====== Message Fields ======

// MSG_COLOR_SELECTOR_CLOSED
extern const char * const MESSAGE_DATA_ID_DIALOG_CANCELED;			// bool (optional)

// ====== Class Defs ======

class CBorderView;

class CChangeNotifyColorControl : public BColorControl
{
	public:
	CChangeNotifyColorControl(BPoint leftTop, color_control_layout matrix, 
		float cellSize, const char *name, BMessage *message, 
		bool bufferedDrawing=false);
	virtual ~CChangeNotifyColorControl();

	virtual void SetValue(int32 value);
	void SetValue(rgb_color color);

	void SetChangeNotifyMessage(BMessage *message);
	BMessage *ChangeNotifyMessage() const;
	
	protected:
	BMessage *changeNotifyMessage;
};

class CColorSelectDialogView : public CLocalizedDialogBase, public BInvoker
{
	public:
	CColorSelectDialogView(BRect frame, const char *name, BMessage *message,
		uint32 resizingMode=B_FOLLOW_ALL, uint32 flags=B_FRAME_EVENTS);
	virtual ~CColorSelectDialogView() {}
	
	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	virtual void MessageReceived(BMessage *message);

	void SetColor(rgb_color color);
	rgb_color Color();
	
	protected:
	virtual bool Ok();
	virtual void Cancel();
	
	static const float dist;
	
	CChangeNotifyColorControl *colorControl;
	CBorderView *colorViewer;
};

class CColorSelectDialog : public BWindow
{
	public:
	CColorSelectDialog(BWindow *blocked, const char *title);
	virtual ~CColorSelectDialog() {}

	void SetColor(rgb_color color)
	{
		return dynamic_cast<CColorSelectDialogView *>(ChildAt(0))->SetColor(color); 
	}

	void SetTarget(BHandler *handler) { Invoker()->SetTarget(handler); }
	void SetMessage(BMessage *message) { Invoker()->SetMessage(message); }

	protected:	
	BInvoker *Invoker() { return dynamic_cast<BInvoker *>(ChildAt(0)); }
};

#endif // COLOR_SELECT_DIALOG_H