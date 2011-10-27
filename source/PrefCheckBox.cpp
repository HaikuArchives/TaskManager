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
#include "PrefCheckBox.h"

// ====== CPrefCheckBox ======

CPrefCheckBox::CPrefCheckBox(
	BRect frame,			
	const char *_prefName,	// name of pref setting
	const char *label, 		// checkbox label
	BMessage *message,
	bool _defaultValue, 	// default value, if perf setting is not present
	bool _invertValue, 		// invert value (also default value!)
	uint32 resizingMode, 
	uint32 flags) :
	BCheckBox(frame, _prefName, label, message, resizingMode, flags)
{
	invertValue  = _invertValue;
	defaultValue = _defaultValue;
	
	prefName = new char [strlen(_prefName)+1];
	strcpy(prefName, _prefName);
}

CPrefCheckBox::~CPrefCheckBox()
{
	delete prefName;
}

void CPrefCheckBox::WriteToPrefs(CPreferences *prefs)
{
	bool set = (Value() == B_CONTROL_ON);
	
	if(invertValue) 
		set = !set;

	prefs->Write(prefName, set);
}

void CPrefCheckBox::ReadFromPrefs(CPreferences *prefs)
{
	bool set;
	
	prefs->Read(prefName, set, defaultValue);
	
	if(invertValue)
		set = !set;
	
	SetValue(set ? B_CONTROL_ON : B_CONTROL_OFF);
}

// ====== CScriptingPrefCheckBox ======

CScriptingPrefCheckBox::CScriptingPrefCheckBox(
	BRect frame,
	BMessage *_scriptMessage,	// message containing the script specifiers
	const char *_prefName,
	const char *label,
	BMessage *message, 
	bool _defaultValue,
	bool _invertValue, 
	uint32 resizingMode,
	uint32 flags) :
	CPrefCheckBox(frame, _prefName, label, message, _defaultValue,
		_invertValue, resizingMode, flags)
{
	scriptMessage = _scriptMessage;
}

CScriptingPrefCheckBox::~CScriptingPrefCheckBox()
{
	delete scriptMessage;
}
	
void CScriptingPrefCheckBox::WriteToPrefs(CPreferences *prefs)
{
	BMessage reply;

	bool set = Value() == B_CONTROL_ON;

	if(invertValue) set = !set;

	scriptMessage->what = B_SET_PROPERTY;
	scriptMessage->AddBool("data", set);

	be_app_messenger.SendMessage(scriptMessage, &reply);
	
	CPrefCheckBox::WriteToPrefs(prefs);
}

void CScriptingPrefCheckBox::ReadFromPrefs(CPreferences *prefs)
{
	if(!prefs->IsSpecified(prefName)) {
		// Pref is not specified in settings file.
		// Try to read value from property.
		
		scriptMessage->what = B_GET_PROPERTY;

		BMessage reply;
		
		if(be_app_messenger.SendMessage(scriptMessage, &reply) == B_OK) {
			bool propValue;
		
			if(reply.FindBool("result", &propValue) == B_OK) {
				if(invertValue)	propValue = !propValue;
	
				SetValue(propValue ? B_CONTROL_ON : B_CONTROL_OFF);
			}
		}
	} else {
		CPrefCheckBox::ReadFromPrefs(prefs);
	}
}

