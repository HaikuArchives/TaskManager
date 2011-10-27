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

#ifndef DESKBAR_LED_VIEW_H
#define DESKBAR_LED_VIEW_H

#include "PulseView.h"

class CTooltip;
class CContextMenuDetector;
class IDataProvider;
class CAsynchronousPopUpMenu;

#if B_BEOS_VERSION >= B_BEOS_VERSION_5
extern "C" BView _EXPORT  *instantiate_deskbar_item();
#endif

BView *create_deskbar_replicant();

class _EXPORT CDataProviderInfo
{
	public:
	CDataProviderInfo(CPulseView *_view, IDataProvider *_dataProvider, 
		float _maxValue,rgb_color _color=CColor::Transparent);
	virtual ~CDataProviderInfo();
	
	float 		MaxValue()		{ return maxValue; }
	void		SetMaxValue(float m) { maxValue = m; }
	float 		CurrentValue()	{ return value; }
	void 		UpdateValue();
	void		UpdateValue(bigtime_t relativeTime);
	rgb_color	Color()			{ return color; }
	void		SetColor(rgb_color c) { color = c; }
	
	protected:
	CPulseView *view;
	rgb_color color;
	float maxValue, value;
	IDataProvider *dataProvider;
};

class _EXPORT CDeskbarLedView : public CPulseView
{
	public:
	CDeskbarLedView(const char *name);
	CDeskbarLedView(BMessage *archive);
	virtual ~CDeskbarLedView();
	
	static BArchivable *Instantiate(BMessage *archive);
		
	virtual	status_t Archive(BMessage *data, bool deep = true) const;	
	virtual void MessageReceived(BMessage *message);	
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);	
	virtual void MouseDown(BPoint point);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	virtual void MouseUp(BPoint point);
	
	void Init();
	
	void UpdateTooltip(bool forceShow, BPoint point=BPoint(-1,-1));
	
	virtual void Pulse();
	
	virtual CAsynchronousPopUpMenu *ContextMenu();
	
	protected:
	CTooltip *tooltip;
	
	// Set to true when the tooltip shouln't 
	// be displayed.
	bool tooltipDisabled;
	
	// Set to true while the context menu is
	// on scren.
	bool menuVisible;
	
	rgb_color ledOnColor, ledOffColor;
	CContextMenuDetector *detector;
	
	CPointerList<CDataProviderInfo> dataProviderList;
	CDataProviderInfo *tooltipDataProvider;
};

#endif // DESKBAR_LED_VIEW_H