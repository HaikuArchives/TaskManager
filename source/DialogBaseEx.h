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
 
#ifndef DIALOG_BASE_EX_H
#define DIALOG_BASE_EX_H

#include "DialogBase.h"
#include "alert.h"

class CLocalizedDialogBase : public CDialogBase
{
	public:
	CLocalizedDialogBase(BRect frame, const char *name,
		uint32 resizingMode=B_FOLLOW_ALL,
		uint32 flags=B_FRAME_EVENTS,
		const char *okButtonLabel=NULL,
		const char *cancelButtonLabel=NULL,
		bool showHelpButton=false) :
	CDialogBase(
		frame,
		name,
		resizingMode,
		flags,
		okButtonLabel==NULL ? ok_button_label() : okButtonLabel,
		cancelButtonLabel==NULL ? cancel_button_label() : cancelButtonLabel,
		showHelpButton)
	{}

	virtual ~CLocalizedDialogBase() {}
};

#endif // DIALOG_BASE_EX_H