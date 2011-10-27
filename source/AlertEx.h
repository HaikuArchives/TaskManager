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
 
#ifndef ALERT_EX_H
#define ALERT_EX_H

#include "Preferences.h"

class CAlertEx : public BAlert
{
	public:
	CAlertEx(
		const char *title,
		const char *text,
		const char *button0Label,
		const char *button1Label=NULL,
		const char *button2Label=NULL,
		button_width widthStyle = B_WIDTH_AS_USUAL,
		button_spacing spacing = B_EVEN_SPACING,
		alert_type type = B_INFO_ALERT,
		const char *checkBoxTitle=NULL,
		CPreferences *_prefs=NULL,
		const char *_prefName=NULL,
		bool prefDefaultValue=false);
	
	virtual ~CAlertEx();
	
	virtual void FrameResized(float w, float h);
	virtual void Quit();
	
	BCheckBox *CheckBox() const { return checkbox; }
	
	protected:
	CPreferences *prefs;
	char		 *prefName;
	BCheckBox	 *checkbox;
};

#endif // ALERT_EX_H