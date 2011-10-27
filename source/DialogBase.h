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

#ifndef DIALOG_BASE_H
#define DIALOG_BASE_H

// ====== Includes =======

#include "FlickerFreeButton.h"

// ====== Message IDs =======

const int32 MSG_OK							= 'mOK ';
const int32 MSG_CANCEL						= 'mCAN';
const int32 MSG_HELP						= 'mHLP';

// ====== Class Defs =======

class CDialogBase : public BView
{
	public:
	CDialogBase(BRect frame, const char *name, 
		uint32 resizingMode=B_FOLLOW_ALL, 
		uint32 flags=B_FRAME_EVENTS,
		const char *okButtonLabel=NULL,
		const char *cancelButtonLabel=NULL,
		bool showHelpButton=false);
	
	virtual ~CDialogBase();
	
	virtual BRect ClientRect();
	
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage *message);
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void GetPreferredSize(float *width, float *height);
	virtual void FrameResized(float width, float height);

	void SetHelpID(const char *id);
	const char *HelpID() const { return helpId; }
	
	protected:
	// called if the user presses the OK button or enters return
	virtual bool Ok() { return true; }
	// called if the user presses the CANCEL button or presses ESC
	virtual void Cancel() {}
	
	void SetOkButtonLabel(const char *label)		{ okButton->SetLabel(label); }
	void SetCancelButtonLabel(const char *label)	{ cancelButton->SetLabel(label); }
	
	virtual void EnableOkButton(bool enable=true)	{ okButton->SetEnabled(enable); }
	
	static const float dist;
	
	BButton *okButton, *cancelButton, *helpButton;
	char *helpId;
};

class CHelpButton : public CFlickerFreeButton
{
	public:
	CHelpButton(BRect frame, const char *name, BMessage *message,
		uint32 resizingMode=B_FOLLOW_LEFT|B_FOLLOW_TOP,
		uint32 flags=B_WILL_DRAW | B_NAVIGABLE);
	
	virtual void GetPreferredSize(float *width, float *height);
	
	// Returns true if the button is pressed (highlighted)
	bool IsHighlighted() { return Value() == B_CONTROL_ON; }

	virtual void Draw(BRect updateRect);
	
	void SetDrawFont();
};

class CEscapeMessageFilter : public BMessageFilter
{
	public:
	CEscapeMessageFilter(BHandler *_escTarget);

	virtual filter_result Filter(BMessage *message, BHandler **target);
	
	protected:
	BHandler *escTarget;
};

#endif // DIALOG_BASE_H