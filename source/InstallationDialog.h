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

#ifndef INSTALLATION_DIALOG_H
#define INSTALLATION_DIALOG_H

#include "Singleton.h"
#include "DialogBase.h"

class CInstallationDialog : public CSingletonWindow
{
	public:
	static CInstallationDialog *CreateInstance();
	
	virtual ~CInstallationDialog();

	virtual bool QuitRequested();

	protected:
	CInstallationDialog();
	
	friend class CSingleton;
};

class CInstallationDialogView : public CDialogBase
{
	public:
	CInstallationDialogView(BRect frame);
	
	virtual void AttachedToWindow();
	virtual void GetPreferredSize(float *width, float *height);
	virtual void FrameResized(float widht, float height);
	virtual void MessageReceived(BMessage *msg);
	
	protected:
	virtual bool Ok();

	static const float dist;
	
	static const int32 NUM_CHECK_BOXES = 3;
	BCheckBox *checkBoxes[NUM_CHECK_BOXES];
	BMenuField *groupMenuField;
	BMenuField *languageMenuField;
	BBox *generalOptionsBox;
	BBox *languageOptionsBox;
};


#endif // INSTALLATION_DIALOG_H
