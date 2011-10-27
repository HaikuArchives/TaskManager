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
#include "AlertEx.h"

// Constructor
// Creates an alert which contains an additional checkbox.
// The checkbox is only displayed, if the name of the checkbox
// is not NULL. The state of the checkbox is loaded on open and
// stored on close if a perferences object and a settings name is
// passed.
// Additonal params:
// * checkBoxTitle:		title of checkbox
// * prefs:				perferences object (belongs to the alert)
// * prefName:			name of a bool perference setting which defines the 
//						state of the checkbox.
// * prefDefaultValue:	default value if 'prefName' setting is not present
//						in settings file
CAlertEx::CAlertEx(
	const char *title,
	const char *text,
	const char *button0Label,
	const char *button1Label,
	const char *button2Label,
	button_width widthStyle,
	button_spacing spacing,
	alert_type type,
	const char *checkBoxTitle,			
	CPreferences *_prefs,				 
	const char *_prefName, 
	bool prefDefaultValue) :
	BAlert(title, text, button0Label, button1Label,
		button2Label, widthStyle, spacing, type)
{
	prefs = _prefs;
	
	if(_prefName) {
		prefName = new char[strlen(_prefName)+1];
		strcpy(prefName, _prefName);
	} else {
		prefName = NULL;
	}

	if(checkBoxTitle != NULL) {
		// --- create checkbox
		
		BRect textRect  = TextView()->Frame();

		TextView()->ConvertToScreen(&textRect);
		ConvertFromScreen(&textRect);
		
		// This is a dummy rect. The correct size is determined
		// by ResizeToPreferred(). The correct position is set
		// in FrameResized(). (This position is resonable. Just
		// for the case that FrameResized isn't called for some
		// strange reason).
		BRect checkRect(textRect.left+10, textRect.bottom+10, 
						textRect.left+20, textRect.bottom+20);
						
		checkbox = new BCheckBox(checkRect, "prefCheck", checkBoxTitle, 
							NULL);
						
		checkbox->SetViewColor(CColor::BeBackgroundGray);
		checkbox->ResizeToPreferred();
		
		if(prefs && prefName) {
			// Load preferences
			bool controlOn=true;
			
			prefs->Read(prefName, controlOn, prefDefaultValue);
			
			checkbox->SetValue(controlOn ? B_CONTROL_ON : B_CONTROL_OFF);
		}

		// Enlarge window to contain checkbox.
		ResizeBy(0, checkbox->Bounds().Height()+20);
		
		// Add checkbox to main view of alert.
		TextView()->Parent()->AddChild(checkbox);
	} else {
		checkbox = NULL;
	}
}

CAlertEx::~CAlertEx()
{
	delete prefs;
	delete [] prefName;
}

void CAlertEx::FrameResized(float w, float h)
{
	BAlert::FrameResized(w, h);

	if(checkbox) {
		// Move text view out of the way.
		TextView()->ResizeBy(0, -20-checkbox->Bounds().Height());
		
		BRect textRect  = TextView()->Bounds();

		// Convert text rectangle to parent coordiantes.
		TextView()->ConvertToScreen(&textRect);
		checkbox->Parent()->ConvertFromScreen(&textRect);

		checkbox->MoveTo(textRect.left, textRect.bottom + 10);
	}
}

void CAlertEx::Quit()
{
	if(checkbox && prefs && prefName) {
		// write back prefs
		prefs->Write(prefName, checkbox->Value()==B_CONTROL_ON);
	}
	
	BAlert::Quit();
}