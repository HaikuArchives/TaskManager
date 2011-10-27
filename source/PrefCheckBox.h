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

#ifndef PREF_CHECK_BOX_H
#define PREF_CHECK_BOX_H

#include "TaskManagerPrefs.h"

// CPrefCheckBox
// This checkbox supports the CPrefPersistent interface and can
// read and write its state from/to the preferences.
class CPrefCheckBox : public CPrefPersistent, public BCheckBox
{
	public:
	CPrefCheckBox(BRect frame, const char *_prefName, const char *label,
		BMessage *message, bool _defaultValue, bool _invertValue=false, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~CPrefCheckBox();
	
	virtual void WriteToPrefs(CPreferences *prefs);
	virtual void ReadFromPrefs(CPreferences *prefs);
	
	protected:
	bool invertValue, defaultValue;
	char *prefName;
};

// CScriptingPrefCheckBox
// Instead of direktly reading from the prefs in ReadFromPrefs
// this checkbox tries to read a boolean property. If this fails
// it gets the value from the prefs.
// In WriteToPrefs it writes its state to the prefs and also sets
// a boolean property to its state.
// This checkbox is useful to direktly represent the state of a
// pref setting in the application.
class CScriptingPrefCheckBox : public CPrefCheckBox
{
	public:
	CScriptingPrefCheckBox(BRect frame, BMessage *_scriptMessage, 
		const char *_prefName, const char *label, BMessage *message, 
		bool _defaultValue, bool _invertValue=false, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~CScriptingPrefCheckBox();
	
	virtual void WriteToPrefs(CPreferences *prefs);
	virtual void ReadFromPrefs(CPreferences *prefs);
	
	protected:
	BMessage *scriptMessage;
};

#endif // PREF_CHECK_BOX_H