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

#ifndef USAGE_VIEW_H
#define USAGE_VIEW_H

// ====== Includes ======
	
#include "Tab.h"

// ====== Message IDs ======

const int32 MSG_CPU_NUM_SEL					= 'mCPU';

// ====== Message Fields =======

// MSG_CPU_NUM_SEL
extern const char * const MESSAGE_DATA_ID_CPU_NUM;					// int32

// ====== Class Defs ======

class CGraphView;
class CLedView;
class CBorderView;
class CBox;
	
class _EXPORT CCPUGraphView : public CGraphView
{
	public:
	CCPUGraphView(BRect rect, int32 _cpuNum);
	CCPUGraphView(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;

	virtual void MessageReceived(BMessage *message);

	protected:
	BPopUpMenu *ContextMenu();
};

class _EXPORT CMemGraphView : public CGraphView
{
	public:
	CMemGraphView(BRect rect);
	CMemGraphView(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;
};

class _EXPORT CCPULedView : public CLedView
{
	public:
	CCPULedView(BRect rect);
	CCPULedView(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);
	virtual status_t Archive(BMessage *data, bool deep) const;
};

class _EXPORT CMemLedView : public CLedView
{
	public:
	CMemLedView(BRect rect);
	CMemLedView(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);
	virtual status_t Archive(BMessage *data, bool deep) const;
	
	protected:
	void Init();
};

class CUsageView : public CTabNotifcationView
{
	public:
	CUsageView(BRect rect);
	
	virtual void FrameResized(float width, float height);
	virtual void AttachedToWindow();

	protected:
	int32 cpuCount;
	const float distTop;
	const float distBottom;
	const float distLeftRight;
	const int32 borderSize;

	CCPUGraphView *cpuGraphViews[_SC_NPROCESSORS_CONF];
	BDragger *cpuGraphViewDragger[_SC_NPROCESSORS_CONF];
	CBorderView *cpuGraphBorder[_SC_NPROCESSORS_CONF];
	
	CBox *cpuLedBox;
	CBox *memLedBox;
	CBox *cpuGraphBox;
	CBox *memGraphBox;
};

#endif // USAGE_VIEW_H
