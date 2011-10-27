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
#include "signature.h"
#include "common.h"
#include "PulseView.h"

#include "msg_helper.h"

const char * const PULSE_VIEW_ARCHIVE_PULSE_RATE = "PULSEVIEW:PulseRate";

CPulseView::CPulseView(BRect frame, const char *name, uint32 resizingMode, uint32 flags) :
	BView(frame, name, resizingMode, flags)
{
	messageRunner	= NULL;
	pulseRate 		= 0;
	replicant		= false;
}

CPulseView::CPulseView(BMessage *archive) :
	BView(archive)
{
	pulseRate		= FindTime(archive, PULSE_VIEW_ARCHIVE_PULSE_RATE);
	messageRunner	= NULL;
	replicant		= true;
	
	// Delete B_PULSE_NEEDED flag. I use the MSG_DESKBAR_PULSE instead.							
	SetFlags(Flags() & ~B_PULSE_NEEDED);
}

CPulseView::~CPulseView()
{
	delete messageRunner;
}

BArchivable *CPulseView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CPulseView"))
		return NULL;

	return new CPulseView(archive);
}

status_t CPulseView::Archive(BMessage *data, bool deep) const
{
	status_t status = BView::Archive(data, deep);
	
	if(status == B_OK) {
		bigtime_t repPulseRate = ReplicantPulseRate();

		// data->AddString("class", "CPulseView");
		data->AddString("add_on", APP_SIGNATURE);
		data->AddData(PULSE_VIEW_ARCHIVE_PULSE_RATE, B_TIME_TYPE, &repPulseRate, sizeof(pulseRate));
	}
	
	return status;
}

void CPulseView::AttachedToWindow()
{
	BView::AttachedToWindow();
	
	if(replicant && pulseRate != 0) {
		// For a replicant it isn't save to rely on the pulse rate
		// of the parent window. I create my own message instead.	
		messageRunner = new BMessageRunner(
								BMessenger(this),
								new BMessage(MSG_DESKBAR_PULSE),
								pulseRate);
	}
}

void CPulseView::SetReplicantPulseRate(bigtime_t newPulseRate)
{
	if(replicant) {
		delete messageRunner;

		pulseRate = newPulseRate;

		messageRunner = new BMessageRunner(
								BMessenger(this),
								new BMessage(MSG_DESKBAR_PULSE),
								pulseRate);
	} else {
		if(Window())
			Window()->SetPulseRate(newPulseRate);
	}
}

bigtime_t CPulseView::ReplicantPulseRate() const
{
	return replicant ? pulseRate : (Window() ? Window()->PulseRate() : 0);
}

void CPulseView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case 'PSTE':
			if(message->WasDropped()) {
				// a color was droped from the 'ColorSelector' utility
				rgb_color bgColor;
				
				if(FindColor(message, "RGBColor", bgColor) == B_OK) {
					SetViewColor(bgColor);
					Invalidate();
				}
			}
			break;
		case MSG_SLOW_UPDATE:
			SetReplicantPulseRate(SLOW_PULSE_RATE);
			break;
		case MSG_FAST_UPDATE:
			SetReplicantPulseRate(FAST_PULSE_RATE);
			break;
		case MSG_NORMAL_UPDATE:
			SetReplicantPulseRate(NORMAL_PULSE_RATE);
			break;
		case MSG_DESKBAR_PULSE:
			Pulse();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
