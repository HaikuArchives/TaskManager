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
#include "help.h"
#include "Color.h"
#include "FlickerFreeButton.h"
#include "DialogBase.h"

#include "my_assert.h"

// ==== CHelpButton ====

CHelpButton::CHelpButton(
	BRect frame, 
	const char *name,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags) :
	CFlickerFreeButton(frame, name, "", message, resizingMode, flags)
{
}

void CHelpButton::SetDrawFont()
{
	BFont boldPlain(be_plain_font);
		
	boldPlain.SetFace(B_BOLD_FACE);

	SetFont(&boldPlain);
}
	
void CHelpButton::Draw(BRect updateRect)
{
	if(IsFocusChanging()) {
		Invalidate();
	} else {
		SetDrawFont();
	
		CFlickerFreeButton::Draw(updateRect);
	
		float width  = Bounds().Width();
		float height = Bounds().Height();
		
		BRect circleRect(6, 6, width-6, height-6);
	
		CColor highColor, antiAliasColor;
	
		if(IsEnabled()) {
		
			if(IsHighlighted()) {
				highColor = CColor::White;
				antiAliasColor = highColor - CColor(200, 200, 200);
			} else {
				if(IsFocus())
					highColor = CColor::BeFocusBlue;
				else
					highColor = CColor::Black;
				
				antiAliasColor = highColor + CColor(200, 200, 200);
			}
		} else {
			highColor = CColor(128, 128, 128);
			antiAliasColor = CColor(200, 200, 200);
		}
		
		SetHighColor(antiAliasColor);
		SetPenSize(3);
		StrokeEllipse(circleRect);
		
		SetHighColor(highColor);
		SetPenSize(0);
		StrokeEllipse(circleRect);

		const char *label = "?";
		float labelWidth = StringWidth(label);
	
		font_height fh;
		GetFontHeight(&fh);
	
		BPoint labelPoint((width-labelWidth)/2+0.5, (height-fh.ascent)/2+fh.ascent-0.5);
	
		DrawString(label, labelPoint);
	}
}

void CHelpButton::GetPreferredSize(float *width, float *height)
{
	font_height fh;
	
	GetFontHeight(&fh);

	const float offset = 12;
	
	float size = fh.ascent + fh.descent + offset;

	if(width)
		*width = size;

	if(height)
		*height = size;
}

// ==== CDialogBase =====

// distance of the buttons from the border.
const float CDialogBase::dist = 10.0;

CDialogBase::CDialogBase(
	BRect frame,
	const char *name, 
	uint32 resizingMode,
	uint32 flags,
	const char *okButtonLabel,
	const char *cancelButtonLabel,
	bool showHelpButton) :
	BView(frame, name, resizingMode, flags | B_FRAME_EVENTS)
{
	// intermediate position for the buttons during creation. They are
	// resized/moved afterwards.
	BRect dummyPos(0,0,5,5);

	// --- process 'OK' and 'Cancel' button
	
	// create buttons on dummy position

	const char *defaultOkLabel		= "OK";
	const char *defaultCancelLabel	= "Cancel";

	okButton = new CFlickerFreeButton(dummyPos, "OkButton", 
							okButtonLabel ? okButtonLabel : defaultOkLabel, 
							new BMessage(MSG_OK), 
							B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);
											
	cancelButton = new CFlickerFreeButton(dummyPos, "CancelButton", 
							cancelButtonLabel ? cancelButtonLabel : defaultCancelLabel, 
							new BMessage(MSG_CANCEL), 
							B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);

	// calculate real button size

	float okButtonHeight, okButtonWidth;
	float cancelButtonHeight, cancelButtonWidth;
	
	okButton->GetPreferredSize(&okButtonWidth, &okButtonHeight);
	cancelButton->GetPreferredSize(&cancelButtonWidth, &cancelButtonHeight);

	const float minButtonWidth	= 80.0;
	const float minButtonHeight	= 20.0;

	float buttonWidth  = MAX(okButtonWidth, MAX(cancelButtonWidth, minButtonWidth));
	float buttonHeight = MAX(okButtonHeight, MAX(cancelButtonHeight, minButtonHeight));

	okButton->ResizeTo(buttonWidth, buttonHeight);
	cancelButton->ResizeTo(buttonWidth, buttonHeight);

	// add buttons

	AddChild(okButton);
	AddChild(cancelButton);

	// --- process help button
	
	if(showHelpButton) {
		float buttonWidth, buttonHeight;
	
		helpButton = new CHelpButton(dummyPos, "HelpButton",
							new BMessage(MSG_HELP),
							B_FOLLOW_BOTTOM | B_FOLLOW_LEFT);

		helpButton->GetPreferredSize(&buttonWidth, &buttonHeight);

		helpButton->ResizeTo(buttonWidth, buttonHeight);
		helpButton->SetEnabled(false);
		AddChild(helpButton);
	} else {
		helpButton = NULL;
	}
	
	helpId = NULL;
}

CDialogBase::~CDialogBase()
{
	// the buttons are automatically deleted.
	
	delete [] helpId;
}

void CDialogBase::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);

	// move buttons to correct position

	// A default button is resized to all sides by 3 pixel
	const float defaultButtonInset=-3;

	float buttonWidth  = cancelButton->Bounds().Width();
	float buttonHeight = cancelButton->Bounds().Height();
	
	BRect okButtonRect(width  - dist - buttonWidth,
					   height - dist - buttonHeight,
					   width  - dist,
					   height - dist);

	BRect cancelButtonRect = okButtonRect.OffsetByCopy(-(dist+buttonWidth), 0);

	if(okButton->IsDefault()) {
		okButtonRect.InsetBy(defaultButtonInset, defaultButtonInset);
	}

	okButton->MoveTo(okButtonRect.LeftTop());
	cancelButton->MoveTo(cancelButtonRect.LeftTop());
	
	if(helpButton) {
		helpButton->MoveTo(dist, cancelButtonRect.top);
	}
}

void CDialogBase::AttachedToWindow()
{
	BView::AttachedToWindow();

	SetViewColor(CColor::BeBackgroundGray);

	okButton->SetTarget(this);											
	cancelButton->SetTarget(this);
	
	if(helpButton)
		helpButton->SetTarget(this);

	// Simulate frame resized event to move buttons to 
	// correct position.
	FrameResized(Bounds().Width(), Bounds().Height());

	// install message hook for ESC keypress.
	// this class needs to be notified when the user presses ESC,
	// even if it's not the keyboard focus view.
	Window()->AddCommonFilter(new CEscapeMessageFilter(this));

	Window()->SetDefaultButton(okButton);
}

void CDialogBase::KeyDown(const char *bytes, int32 numBytes)
{
	switch(bytes[0]) {
		case B_ESCAPE:
			// I always get the ESC keypress (even when not keybord focus),
			// because I installed a message filter which redirects 
			// these messages.
			Cancel();
			Window()->PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			BView::KeyDown(bytes, numBytes);
	}
}

void CDialogBase::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_OK:
			if(Ok()) Window()->PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_CANCEL:
			Cancel();
			Window()->PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_HELP:
			{
				const char *helpId = HelpID();
				
				if(helpId)
					show_help(helpId);
			}
			break;
	}
}

void CDialogBase::SetHelpID(const char *id)
{
	delete [] helpId;
	
	helpId = new char [strlen(id)+1];
	
	strcpy(helpId, id);
	
	helpButton->SetEnabled(true);
}

BRect CDialogBase::ClientRect()
{
	return BRect(
				0, 
				0, 
				Bounds().Width(), 
				Bounds().Height()-2*dist-cancelButton->Bounds().Height()
			);
}

void CDialogBase::GetPreferredSize(float *width, float *height)
{
	if(width) {
		if(helpButton) {
			*width = 4*dist + 2*(cancelButton->Bounds().Width()) + helpButton->Bounds().Width();
		} else {
			*width = 3*dist + 2*(cancelButton->Bounds().Width());
		}
	}

	if(height) {
		*height = 2*dist + cancelButton->Bounds().Height();
	}
}

// ==== CEscapeMessageFilter ====

CEscapeMessageFilter::CEscapeMessageFilter(BHandler *_escTarget) :
	BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN)
{
	escTarget = _escTarget;
}

// replaces the target for ESC keypress messages 'escTarget'.
filter_result CEscapeMessageFilter::Filter(
	BMessage *message, 
	BHandler **target
)
{
	if(message->what == B_KEY_DOWN) {
		int8 pressedChar;
	
		if(message->FindInt8("byte", 0, &pressedChar) == B_OK) {
			if(pressedChar == B_ESCAPE) {
				// replace target
				*target = escTarget;
			}
		}
	}
	
	// contiune processing
	return B_DISPATCH_MESSAGE;
}
