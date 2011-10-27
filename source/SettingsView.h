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

#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "Box.h"
#include "DialogBaseEx.h"
#include "TaskManagerPrefs.h"

class CSettingsGroup : public CBox, public CPrefPersistent
{
	public:
	CSettingsGroup(BRect frame, 
		const char *name,
		int32 numColumns = 1,
		uint32 resizingMode = B_FOLLOW_NONE, 
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
		border_style border = B_FANCY_BORDER);

	virtual void GetPreferredSize(float *width, float *height);
	virtual void FrameResized(float width, float height);
	virtual void WriteToPrefs(CPreferences *prefs);
	virtual void ReadFromPrefs(CPreferences *prefs);
	
	protected:
	int32 numColumns;
};

class CInternalSettingsView : public BView, public CPrefPersistent
{
	public:
	CInternalSettingsView(BRect frame, const char *name, 
		uint32 resizingMode = B_FOLLOW_ALL, uint32 flags=B_FRAME_EVENTS);

	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	virtual void FrameResized(float width, float height);
	virtual void WriteToPrefs(CPreferences *prefs);
	virtual void ReadFromPrefs(CPreferences *prefs);
};

class CSettingsView : public CLocalizedDialogBase, public CPrefPersistent
{
	public:
	CSettingsView(BRect frame, const char *name, 
		uint32 resizingMode=B_FOLLOW_ALL, uint32 flags=B_FRAME_EVENTS);

	virtual bool Ok();
	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	virtual void FrameResized(float width, float height);
	virtual void WriteToPrefs(CPreferences *prefs);
	virtual void ReadFromPrefs(CPreferences *prefs);
	virtual void WindowActivated(bool active);

	BView *InternalView() { return internalView; }
	
	protected:
	bool	prefsRead;
	BView  *internalView;
};

#endif // SETTINGS_VIEW_H