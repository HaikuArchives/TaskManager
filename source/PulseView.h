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

#ifndef PULSE_VIEW_H
#define PULSE_VIEW_H

//! file=PulseView.h

// ====== Message IDs ======

const int32 MSG_DESKBAR_PULSE  				= 'mDBP';
const int32 MSG_SLOW_UPDATE 	   			= 'mSLW';
const int32 MSG_FAST_UPDATE 		   		= 'mFST';
const int32 MSG_NORMAL_UPDATE				= 'mNRM';

// ====== Class Defs ======

//: A view providing its own Pulse() when it's replicated.
// When a view is replicated it can't rely on the parent to deliver the
// Pulse() notification in the rate needed by a particular view.
// Therefore this view creates its own Pulse using a BMessageRunner.
class _EXPORT CPulseView : public BView
{
	public:
	CPulseView(BRect frame, const char *name, uint32 resizingMode, uint32 flags);
	CPulseView(BMessage *archive);

	virtual ~CPulseView();

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;

	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage *message);

	void SetReplicantPulseRate(bigtime_t newPulseRate);
	bigtime_t ReplicantPulseRate() const;
	bool IsReplicant() const { return replicant; }
	
	protected:
	bool					 replicant;
	BMessageRunner 			*messageRunner;
	bigtime_t 				 pulseRate;
};

#endif // PULSE_VIEW_H