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

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#ifdef MEM_DEBUG
extern"C" void SetNewLeakChecking(bool);
extern"C" void SetMallocLeakChecking(bool);
#endif // MEM_DEBUG

// ====== Application Signature ======

#include "signature.h"

// ====== Application Name ======

#define APP_NAME	  "TaskManager"
#define FULL_APP_NAME "Be " APP_NAME

// ====== Message IDs ======

const int32 MSG_TWEAK_DESKBAR				= 'mDTW';
const int32 MSG_SHOW_MAIN_WINDOW			= 'mSMW';
const int32 MSG_SHOW_REPLICANT 				= 'mSRP';

// ====== Message Fields ======

// MSG_SHOW_REPLICANT
extern const char * const MESSAGE_DATA_ID_SHOW;						// bool

// ====== Class Defs ======

class CTeamModel;

class CTaskManagerApplication : public BApplication
{
	public:
	CTaskManagerApplication();
	virtual ~CTaskManagerApplication();
		
	virtual void ReadyToRun();
	virtual void AboutRequested();
	virtual void ArgvReceived(int32 argc, char **argv);
	virtual void MessageReceived(BMessage *message);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, BMessage *specifier, int32 what, const char *property);

	protected:
	bool IncludesOption(int32 argc, char **argv, const char *option);
	
	BWindow *mainWindow;
	bool showMainWindow;
};

#endif // TASK_MANAGER_H