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
 
#ifndef BUG_FIXED_TAB_H
#define BUG_FIXED_TAB_H

// interface for classes which are notified when they
// become hidden or shown by a BTabView
class ITabNotification
{
	public:
	virtual void Select(BView *owner) = 0;
	virtual void Deselect() = 0;
};

class CTabNotifcationView : public BView, public ITabNotification
{
	public:
	CTabNotifcationView(BRect frame, const char *name,
		uint32 resizingMode, uint32 flags) :
		BView(frame, name, resizingMode, flags) {}
	CTabNotifcationView(BMessage *archive) :
		BView(archive) {}
	virtual ~CTabNotifcationView() {}

	virtual void Select(BView *owner) {}
	virtual void Deselect() {}
};

class CBugFixedTab : public BTab
{
	public:
	CBugFixedTab(BView *target=NULL) : BTab(target) {}
	CBugFixedTab(BMessage *archive) : BTab(archive) {}
	
	// Normally the BTab class removes the hidden tab's view from
	// the view list. Because my views depend on getting B_PULSE messages
	// even when they are hidden I don't remove the views from the list
	// and only hide them.
	
	virtual void Select(BView *owner)
	{
		if(View()->Parent() == NULL) {
			owner->AddChild(View());
		}

		View()->ResizeTo(owner->Bounds().Width(), owner->Bounds().Height());

		ITabNotification *notify = dynamic_cast<ITabNotification *>(View());

		if(notify)
			notify->Select(owner);

		if(View()->IsHidden())
			View()->Show();

	} 
	virtual void Deselect()
	{
		ITabNotification *notify = dynamic_cast<ITabNotification *>(View());

		if(notify)
			notify->Deselect();

		View()->Hide();
	}
};

#endif // BUG_FIXED_TAB_H