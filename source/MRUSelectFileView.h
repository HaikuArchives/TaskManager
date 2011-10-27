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

#ifndef MRU_SELECT_FILE_VIEW_H
#define MRU_SELECT_FILE_VIEW_H

#include "DialogBaseEx.h"
#include "PointerList.h"

// ====== Message IDs ======

const int32 MSG_MRU_ENTRY_SEL				= 'mMRS';
const int32 MSG_SHOW_MRU_LIST				= 'mMRU';
const int32 MSG_PATH_CHANGED				= 'mPCH';
const int32 MSG_BROWSE						= 'mBWS';
const int32 MSG_BROWSE_FINISHED				= 'mBWF';

// ====== Message Fields ======

// MSG_MRU_ENTRY_SEL
extern const char * const MESSAGE_DATA_ID_MRU_PATH;					// string
extern const char * const MESSAGE_DATA_ID_MRU_INDEX;				// int32

// ====== Class Defs ======

class CPreferences;

class CMRUSelectFileView : public CLocalizedDialogBase
{
	public:
	CMRUSelectFileView(BRect frame, CPreferences *mru_prefs,
		const char *name="MRUSelectFile", 
		uint32 resizingMode=B_FOLLOW_ALL, uint32 flags=0);
		
	virtual ~CMRUSelectFileView();
	
	virtual bool Ok();
	virtual void Cancel();
	
	virtual void MessageReceived(BMessage *message);
	virtual void GetPreferredSize(float *width, float *height);
	virtual void AttachedToWindow();
	
	protected:
	typedef CPointerList<BString> CMRUList;
	
	virtual const char *MRUPrefBaseName() { return "MRU_"; }
	virtual void 		LoadMRUList(CMRUList &list);
	virtual void 		SaveMRUList(const CMRUList &list);
	virtual void 		DisplayMRUList();
	virtual void 		MRUEntrySelected(const entry_ref &file);
	virtual void 		MRUEntrySelected(int32 index, const char *path);
	virtual void		AddToMRUList(CMRUList &mruList, const char *path);
	virtual void		BrowseForFile();
	
	BButton			*arrowButton;
	BTextControl	*pathControl;
	BFilePanel		*browsePanel;
	
	CPreferences	*prefs;
};

#endif // MRU_SELECT_FILE_VIEW_H