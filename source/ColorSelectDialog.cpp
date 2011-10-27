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
 
#include "pch.h"
#include "common.h"
#include "msg_helper.h"
#include "BorderView.h"
#include "ColorSelectDialog.h"

// ==== globals ====

const char * const MESSAGE_DATA_ID_DIALOG_CANCELED	= "COLORSELECT:DialogCanceled";

// ==== CChangeNotifyColorControl ====

CChangeNotifyColorControl::CChangeNotifyColorControl(
	BPoint leftTop, color_control_layout matrix, float cellSize, 
	const char *name, BMessage *message, bool bufferedDrawing) :
	BColorControl(leftTop, matrix, cellSize, name, message, bufferedDrawing)
{
	changeNotifyMessage = NULL;
}

CChangeNotifyColorControl::~CChangeNotifyColorControl()
{
	delete changeNotifyMessage;
}

void CChangeNotifyColorControl::SetChangeNotifyMessage(BMessage *message)
{
	changeNotifyMessage = message;
}

BMessage *CChangeNotifyColorControl::ChangeNotifyMessage() const
{
	return changeNotifyMessage;
}

void CChangeNotifyColorControl::SetValue(rgb_color color)
{
	int32 c = (color.red << 24) + (color.green << 16) + (color.blue << 8);
	SetValue(c);
}

void CChangeNotifyColorControl::SetValue(int32 value)
{
	if(changeNotifyMessage && Value() != value) {
		if(Target()) {
			Messenger().SendMessage(changeNotifyMessage, (BHandler *)NULL);
		}
	}

	BColorControl::SetValue(value);
}

// ==== CColorSelectDialog ====

CColorSelectDialog::CColorSelectDialog(BWindow *blocked, const char *title) :
	BWindow(BRect(0,0,50,50), 
		title, 
		B_TITLED_WINDOW_LOOK,
		blocked ? B_MODAL_SUBSET_WINDOW_FEEL : B_NORMAL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS)
{
	if(blocked)
		AddToSubset(blocked);
		
	CColorSelectDialogView *view = 
		new CColorSelectDialogView(Bounds(), "ColorSelectDialogView", NULL);
	
	float width, height;
	
	view->GetPreferredSize(&width, &height);
	
	BRect screenRect = BScreen(this).Frame();
	
	float x = (screenRect.Width() - width) /2;
	float y = (screenRect.Height() - height) /2;
	
	MoveTo(x, y);
	ResizeTo(width, height);
	
	view->ResizeToPreferred();
	
	AddChild(view);
}

// ==== CColorSelectDialogView ====

const float CColorSelectDialogView::dist = 10;

CColorSelectDialogView::CColorSelectDialogView(BRect frame, 
	const char *name, BMessage *message, uint32 resizingMode, uint32 flags) :
	CLocalizedDialogBase(frame, name, resizingMode, flags),
	BInvoker(message, NULL)
{
	float colorCtrlWidth, colorCtrlHeight;

	colorControl = new CChangeNotifyColorControl(BPoint(0, dist), B_CELLS_32x8,
							4.0, "ColorControl", new BMessage(MSG_COLOR_SELECTED));

	colorControl->SetChangeNotifyMessage(new BMessage(MSG_COLOR_SELECTED));

	colorControl->GetPreferredSize(&colorCtrlWidth, &colorCtrlHeight);

	colorViewer = new CBorderView(BRect(dist, dist, dist+colorCtrlHeight, dist+colorCtrlHeight),
							"ColorViewer", 2, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW,
							B_FANCY_BORDER);

	colorViewer->SetBgColor(colorControl->ValueAsColor());

	colorControl->MoveTo(2*dist + colorCtrlHeight, dist);

	AddChild(colorControl);
	AddChild(colorViewer);
}

void CColorSelectDialogView::AttachedToWindow()
{
	CLocalizedDialogBase::AttachedToWindow();

	colorControl->ResizeToPreferred();
	colorControl->SetTarget(this);
	
	if(!(Window()->Flags() & B_ASYNCHRONOUS_CONTROLS)) {
		// Ensure that the ASYNCHRONOUS_CONTROLS flag is set.
		Window()->SetFlags(Window()->Flags() | B_ASYNCHRONOUS_CONTROLS);
	}
}

void CColorSelectDialogView::SetColor(rgb_color color)
{
	colorControl->SetValue(color);
}

rgb_color CColorSelectDialogView::Color()
{
	rgb_color color = colorControl->ValueAsColor();
		
	// The returned color has the alpha channel set to zero.
	// This is corrected here.
	color.alpha = 255;

	return color;
}

void CColorSelectDialogView::GetPreferredSize(float *width, float *height)
{
	float baseWidth, baseHeight;
	
	CLocalizedDialogBase::GetPreferredSize(&baseWidth, &baseHeight);

	colorControl->GetPreferredSize(width, height);
	
	*width  = MAX(*width+3*dist+colorViewer->Bounds().Width(), baseWidth);
	*height += 2*dist + baseHeight;
}

void CColorSelectDialogView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case 'PSTE':
			if(message->WasDropped()) {
				// a color was droped from the 'ColorSelector' utility
				rgb_color color;
				
				if(FindColor(message, "RGBColor", color) == B_OK) {
					colorControl->SetValue(color);
					colorViewer->SetBgColor(color);
					colorViewer->Invalidate();
				}
			}
			break;
		case MSG_COLOR_SELECTED:
			// new color was selected
			colorViewer->SetBgColor(colorControl->ValueAsColor());
			colorViewer->Invalidate();
			break;
		default:
			CLocalizedDialogBase::MessageReceived(message);
	}
}

bool CColorSelectDialogView::Ok()
{
	if(Messenger().IsValid() && Message()) {
		AddColor(Message(), MESSAGE_DATA_ID_COLOR, Color());
			
		Invoke();
	}
	
	return true;
}

void CColorSelectDialogView::Cancel()
{
	if(Messenger().IsValid() && Message()) {
		Message()->AddBool(MESSAGE_DATA_ID_DIALOG_CANCELED, true);
		
		Invoke();
	}
}
