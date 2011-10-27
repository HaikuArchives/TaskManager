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
#include "Box.h"
#include "DialogBase.h"
#include "SettingsView.h"

const float dist=10;

// ====== CSettingsGroup ======

CSettingsGroup::CSettingsGroup(BRect frame, const char *name, 
	int32 _numColumns, uint32 resizingMode, uint32 flags, 
	border_style border) :
	CBox(frame, name, resizingMode, flags, border)
{
	numColumns = _numColumns;
}

void CSettingsGroup::GetPreferredSize(float *width, float *height)
{
	float h=0, rowh=0, w=0;

	for(int32 i=0 ; i<CountChildren() ; i++) {
		float childWidth, childHeight;
		
		ChildAt(i)->GetPreferredSize(&childWidth, &childHeight);
		
		w = MAX(w, childWidth);

		rowh = MAX(rowh, childHeight);

		if((i%numColumns) == 0)
			h += rowh+dist;
	}
	
	if(width)	*width = numColumns*(w+dist)+dist;
	if(height)	*height = h+dist+LabelHeight()+BorderSize();
}

void CSettingsGroup::FrameResized(float width, float height)
{
	CBox::FrameResized(width, height);

	BRect clientRect = ClientRect();

	float ypos = clientRect.top+dist;
	float columnWidth = (clientRect.Width() - numColumns*dist - dist) / numColumns;
	
	for(int32 i=0 ; i<CountChildren() ; i++) {
		BView *child = ChildAt(i);
		
		float childWidth, childHeight;
		
		child->GetPreferredSize(&childWidth, &childHeight);
		
		int32 column = i%numColumns;
		
		child->MoveTo(clientRect.left+column*(columnWidth+dist)+dist, ypos);
		child->ResizeTo(columnWidth, childHeight);
		
		if(((i+1)%numColumns) == 0)
			ypos += childHeight+dist;
	}
}

void CSettingsGroup::WriteToPrefs(CPreferences *prefs)
{
	for(int32 i=0 ; i<CountChildren() ; i++) {
		CPrefPersistent *child = dynamic_cast<CPrefPersistent *>(ChildAt(i));
		
		if(child) child->WriteToPrefs(prefs);
	}
}

void CSettingsGroup::ReadFromPrefs(CPreferences *prefs)
{
	for(int32 i=0 ; i<CountChildren() ; i++) {
		CPrefPersistent *child = dynamic_cast<CPrefPersistent *>(ChildAt(i));
		
		if(child) child->ReadFromPrefs(prefs);
	}
}

// ====== CInternalSettingsView ======

CInternalSettingsView::CInternalSettingsView(BRect frame, const char *name, 
	uint32 resizingMode, uint32 flags) :
	BView(frame, name, resizingMode, flags)
{
}

void CInternalSettingsView::AttachedToWindow()
{
	BView::AttachedToWindow();

	SetViewColor(CColor::BeBackgroundGray);
	FrameResized(Bounds().Width(), Bounds().Height());
}

void CInternalSettingsView::GetPreferredSize(float *width, float *height)
{	
	float w=0, h=0;

	for(int32 i=0 ; i<CountChildren() ; i++) {
		float childWidth, childHeight;
		
		ChildAt(i)->GetPreferredSize(&childWidth, &childHeight);
		
		w = MAX(w, childWidth+2*dist);

		h += childHeight+dist;
	}
	
	if(width)	*width  = w;
	if(height)	*height = (h!=0) ? h+dist : 0;
}

void CInternalSettingsView::FrameResized(float width, float height)
{
	float ypos = dist;

	for(int32 i=0 ; i<CountChildren() ; i++) {
		BView *child = ChildAt(i);
		
		float childWidth, childHeight;
		
		child->GetPreferredSize(&childWidth, &childHeight);
		
		child->MoveTo(dist, ypos);
		child->ResizeTo(width-2*dist, childHeight);
		
		ypos += childHeight+dist;
	}
}

void CInternalSettingsView::WriteToPrefs(CPreferences *prefs)
{
	for(int32 i=0 ; i<CountChildren() ; i++) {
		CPrefPersistent *child = dynamic_cast<CPrefPersistent *>(ChildAt(i));
		
		if(child) child->WriteToPrefs(prefs);
	}
}

void CInternalSettingsView::ReadFromPrefs(CPreferences *prefs)
{
	for(int32 i=0 ; i<CountChildren() ; i++) {
		CPrefPersistent *child = dynamic_cast<CPrefPersistent *>(ChildAt(i));
		
		if(child) child->ReadFromPrefs(prefs);
	}
}

// ====== CSettingsView ======

CSettingsView::CSettingsView(BRect frame, const char *name, 
	uint32 resizingMode, uint32 flags) :
	CLocalizedDialogBase(frame, name, resizingMode, flags, NULL, NULL, true)
{
	internalView = new CInternalSettingsView(ClientRect(), "Internal");

	prefsRead = false;

	AddChild(internalView);
	
	SetHelpID("settings.html#settings_dialog");
}

void CSettingsView::GetPreferredSize(float *width, float *height)
{
	float baseWidth, baseHeight;
	float internalWidth, internalHeight;

	CLocalizedDialogBase::GetPreferredSize(&baseWidth, &baseHeight);
	
	InternalView()->GetPreferredSize(&internalWidth, &internalHeight);
	
	if(width)  { *width  = MAX(baseWidth, internalWidth); }
	if(height) { *height = baseHeight + internalHeight;   }
}

void CSettingsView::FrameResized(float width, float height)
{
	CLocalizedDialogBase::FrameResized(width, height);

	BRect clientRect = ClientRect();
	
	InternalView()->MoveTo(clientRect.LeftTop());
	InternalView()->ResizeTo(clientRect.Width(), clientRect.Height());
}

void CSettingsView::AttachedToWindow()
{
	CLocalizedDialogBase::AttachedToWindow();
	
}

void CSettingsView::WindowActivated(bool active)
{
	if(active && !prefsRead) {
		// Read prefs, when window becomes visible
	
		CTaskManagerPrefs prefs;
		ReadFromPrefs(&prefs);
		
		prefsRead = true;
	}
}

bool CSettingsView::Ok()
{
	CTaskManagerPrefs prefs;
	
	WriteToPrefs(&prefs);

	return true;	
}

void CSettingsView::WriteToPrefs(CPreferences *prefs)
{
	CPrefPersistent *internal = dynamic_cast<CPrefPersistent *>(InternalView());
		
	if(internal) internal->WriteToPrefs(prefs);
}

void CSettingsView::ReadFromPrefs(CPreferences *prefs)
{
	CPrefPersistent *internal = dynamic_cast<CPrefPersistent *>(InternalView());
		
	if(internal) internal->ReadFromPrefs(prefs);
}
