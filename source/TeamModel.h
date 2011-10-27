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

#ifndef TEAM_MODEL_H
#define TEAM_MODEL_H

// ====== Message IDs ======

const int32 MSG_NOTIFY_ITEM_ADDED		= 'nTMA';
const int32 MSG_NOTIFY_ITEM_REMOVED		= 'nTMR';

// ====== Message Fields ======

extern const char * const MESSAGE_DATA_ID_ITEM;					// CTeamModelEntry *

// ====== Scripting Properties ======

// TeamModelEntry
extern const char * const TEAM_MODEL_ENTRY_PROP_FILENAME;		// string
extern const char * const TEAM_MODEL_ENTRY_PROP_ID;				// int32
extern const char * const TEAM_MODEL_ENTRY_PROP_IS_SYSTEM_TEAM;	// bool
extern const char * const TEAM_MODEL_ENTRY_PROP_IS_IDLE_TEAM;	// bool
extern const char * const TEAM_MODEL_ENTRY_PROP_THEAD_COUNT;	// int32
extern const char * const TEAM_MODEL_ENTRY_PROP_AREA_COUNT;		// int32
extern const char * const TEAM_MODEL_ENTRY_PROP_IMAGE_COUNT;	// int32

// TeamModel
extern const char * const TEAM_MODEL_PROP_TEAM;					// TeamModelEntry

class CTeamModelEntry : public BHandler
{
	public:
	CTeamModelEntry(const team_info &teamInfo);
	virtual ~CTeamModelEntry() { id = -1; }

	virtual void MessageReceived(BMessage *message);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index,
      BMessage *specifier, int32 what, const char *property);
    virtual status_t GetSupportedSuites(BMessage *message);

	status_t	InitCheck() const		{ return initResult;  }
	team_id 	TeamId() const			{ return id;          }
	bool		IsSystemTeam() const	{ return systemTeam;  }
	bool		IsIdleTeam() const		{ return idleTeam;    }
	BPath		FileName() const		{ return fileName;    }
	int32   	ThreadCount() const		{ return threadCount; }
	int32   	ImageCount() const		{ return imageCount;  }
	int32   	AreaCount()	const		{ return areaCount;	  }
	bigtime_t 	KernelTime() const		{ return kernelTime;  }
	bigtime_t 	UserTime() const		{ return userTime;	  }
	size_t		AreaSize() const		{ return areaSize;    }

	void Update();
	void Update(const team_info &teamInfo);
	
	protected:
	
	team_id 	id;					// team_id
	BPath	    fileName;
	int32		threadCount;
	int32		imageCount;
	int32		areaCount;
	bigtime_t 	userTime;
	bigtime_t   kernelTime;
	size_t		areaSize;
	status_t	initResult;
	bool		systemTeam;			// is team part of operating system??
	bool		idleTeam;			// is this team the idle task??
};

class ITeamModelListener
{
	public:
	virtual void ItemAdded(CTeamModelEntry *entry, int32 index) = 0;
	virtual void ItemRemoved(CTeamModelEntry *entry, int32 index) = 0;
};

//: Model for team views.
// The model contains a list of the currently running teams in the system.
// Every team is represented as CTeamModelEntry which contains additional
// information about the team.
// A view can choose two ways of notifications about model changes:
// - synchronous: by attaching a CTeamModelListener to the model.
// - asynchronous: by listening for MSG_NOTIFY_ITEM_ADDED and
//   MSG_NOTIFY_ITEM_REMOVED notifications.
class CTeamModel : public BHandler
{
	public:
	CTeamModel(BLooper *looper);
	virtual ~CTeamModel();

	int32 CountEntries() const;
	CTeamModelEntry *EntryAt(int32 index);

	virtual void MessageReceived(BMessage *message);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index,
      BMessage *specifier, int32 what, const char *property);
    virtual status_t GetSupportedSuites(BMessage *message);

	void SetPulseRate(bigtime_t rate);
	
	void StartUpdate(bigtime_t pulseRate);

	void Update();

	void AddTeamModelListener(ITeamModelListener *l);
	void RemoveTeamModelListener(ITeamModelListener *l);
	
	protected:
	void AddTeam(team_id id);
	void AddTeam(team_info *teamInfo);
	void RemoveTeam(team_id id);
	void RemoveEntryAt(int index);

	BMessageRunner *messageRunner;
	CPointerList<CTeamModelEntry> entryList;
	BList listeners;
};

#endif // TEAM_MODEL_H