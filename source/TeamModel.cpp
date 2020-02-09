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
#include "common.h"
#include "msg_helper.h"
#include "my_assert.h"
#include "PointerList.h"
#include "PulseView.h"
#include "Process.h"
#include "ProcessView.h"
#include "TeamModel.h"

// ====== globals ======

// Properties
const char * const TEAM_MODEL_ENTRY_PROP_FILENAME		= "Filename";
const char * const TEAM_MODEL_ENTRY_PROP_ID             = "ID";
const char * const TEAM_MODEL_ENTRY_PROP_IS_SYSTEM_TEAM = "IsSystemTeam";
const char * const TEAM_MODEL_ENTRY_PROP_IS_IDLE_TEAM	= "IsIdleTeam";
const char * const TEAM_MODEL_ENTRY_PROP_THEAD_COUNT	= "ThreadCount";
const char * const TEAM_MODEL_ENTRY_PROP_AREA_COUNT 	= "AreaCount";
const char * const TEAM_MODEL_ENTRY_PROP_IMAGE_COUNT 	= "ImageCount";
const char * const TEAM_MODEL_ENTRY_PROP_KILL			= "Kill";
const char * const TEAM_MODEL_ENTRY_PROP_QUIT			= "Quit";
const char * const TEAM_MODEL_ENTRY_PROP_ACTIVATE		= "Activate";
const char * const TEAM_MODEL_ENTRY_PROP_PRIORITY		= "Priority";

const char * const TEAM_MODEL_PROP_TEAM					= "Team";

// Message Fields
const char * const MESSAGE_DATA_ID_ITEM					= "NOTIFY:Item";

// ====== CTeamModelEntry ======

CTeamModelEntry::CTeamModelEntry(const team_info &teamInfo) :
	BHandler("")
{
	id = teamInfo.team;
	
	idleTeam = systemTeam = false;
	
	CTeam team(id);
	
	if(team.GetPath(&fileName) == B_OK) {
		systemTeam = team.IsSystemTeam();
		
		SetName(fileName.Leaf());	
	} else {
		if(!team.IsValid()) {
			initResult = B_BAD_TEAM_ID;
			return;
		}

		// for Teams without an image (e.g. the "Kernel-Team").
		
		if(id == B_SYSTEM_TEAM) {
			// the Kernel-Team has always team-id B_SYSTEM_TEAM (2).
			systemTeam = true;
			idleTeam = true;
		}
		
		SetName(teamInfo.args);
	}

	Update(teamInfo);	
	
	initResult = B_OK;
}

BHandler *CTeamModelEntry::ResolveSpecifier(BMessage *message, int32 index,
      BMessage *specifier, int32 what, const char *property)
{
	if(message->what == B_GET_PROPERTY) {
		if(what == B_DIRECT_SPECIFIER) {
			if( strcmp(property, TEAM_MODEL_ENTRY_PROP_FILENAME) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_ID) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_IS_SYSTEM_TEAM) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_IS_IDLE_TEAM) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_THEAD_COUNT) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_AREA_COUNT) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_IMAGE_COUNT) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_PRIORITY) == 0 )
				return this;
		}
	} else if(message->what == B_SET_PROPERTY) {
		if(what == B_DIRECT_SPECIFIER) {
			if( strcmp(property, TEAM_MODEL_ENTRY_PROP_PRIORITY) == 0 )
				return this;
		}
	} else if(message->what == B_EXECUTE_PROPERTY) {
		if(what == B_DIRECT_SPECIFIER) {
			if( strcmp(property, TEAM_MODEL_ENTRY_PROP_KILL) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_ACTIVATE) == 0 ||
				strcmp(property, TEAM_MODEL_ENTRY_PROP_QUIT) == 0 )
				return this;
		}
	}
	
	return BHandler::ResolveSpecifier(message, index, specifier, what, property);
}

status_t CTeamModelEntry::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)TEAM_MODEL_ENTRY_PROP_FILENAME,			// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 2nd property
			(char *)TEAM_MODEL_ENTRY_PROP_ID,				// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 3rd property
			(char *)TEAM_MODEL_ENTRY_PROP_IS_SYSTEM_TEAM,	// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 4th property
			(char *)TEAM_MODEL_ENTRY_PROP_IS_IDLE_TEAM,		// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 5th property
			(char *)TEAM_MODEL_ENTRY_PROP_THEAD_COUNT,		// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 6th property
			(char *)TEAM_MODEL_ENTRY_PROP_AREA_COUNT,		// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 7th property
			(char *)TEAM_MODEL_ENTRY_PROP_IMAGE_COUNT,		// name
			{												// commands
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 7th property
			(char *)TEAM_MODEL_ENTRY_PROP_KILL,		// name
			{												// commands
				B_EXECUTE_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 8th property
			(char *)TEAM_MODEL_ENTRY_PROP_QUIT,				// name
			{												// commands
				B_EXECUTE_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 9th property
			(char *)TEAM_MODEL_ENTRY_PROP_ACTIVATE,			// name
			{												// commands
				B_EXECUTE_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{ 										// 10th property
			(char *)TEAM_MODEL_ENTRY_PROP_PRIORITY,			// name
			{												// commands
				B_GET_PROPERTY,
				B_SET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },						// specifiers
			"",												// usage
			0												// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-TeamModelEntry");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return BHandler::GetSupportedSuites(message);
}

void CTeamModelEntry::Update(const team_info &teamInfo)
{
	thread_info threadInfo;
		
	userTime=0;
	kernelTime=0;
	
	int32 cookie = 0;
	ssize_t scookie = 0;
	
	while(get_next_thread_info(id, &cookie, &threadInfo) == B_OK) {
		userTime   += threadInfo.user_time;
		kernelTime += threadInfo.kernel_time;
	}
	
	threadCount = teamInfo.thread_count;
	areaCount   = teamInfo.area_count;
	imageCount  = teamInfo.image_count;
	
	scookie = 0;	
	areaSize = 0;
		
	area_info areaInfo;
		
	while(get_next_area_info(id, &scookie, &areaInfo) == B_OK) {
		areaSize += areaInfo.ram_size;
	}
}

void CTeamModelEntry::Update()
{
	team_info teamInfo;

	if(get_team_info(id, &teamInfo) == B_OK) {
		Update(teamInfo);
	} else {
		userTime = kernelTime = 0;
		
		threadCount = areaCount = imageCount = 0;
	}
}

void CTeamModelEntry::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case B_GET_PROPERTY:
		{
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
				if(what == B_DIRECT_SPECIFIER) {
					BMessage reply(B_REPLY);
					bool handled = true;
					status_t result = B_OK;

					if(strcmp(property, TEAM_MODEL_ENTRY_PROP_FILENAME) == 0) {
						reply.AddString("result", fileName.Path() != NULL ? fileName.Path() : "");
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_ID) == 0) {
						reply.AddInt32("result", id);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_IS_SYSTEM_TEAM) == 0) {
						reply.AddBool("result", systemTeam);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_IS_IDLE_TEAM) == 0) {
						reply.AddBool("result", idleTeam);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_THEAD_COUNT) == 0) {
						reply.AddInt32("result", threadCount);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_AREA_COUNT) == 0) {
						reply.AddInt32("result", areaCount);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_IMAGE_COUNT) == 0) {
						reply.AddInt32("result", imageCount);
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_PRIORITY) == 0) {
						CTeam team(id);
						
						int32 prio;
						
						result = team.GetBasePriority(prio);
					
						if(result == B_OK) {
							reply.AddInt32("result", prio);
						}
					} else {
						handled = false;
					}

					if(handled) {
						send_script_reply(reply, result, message);
						message->PopSpecifier();
						
						return;
					}
				}
			}
			BHandler::MessageReceived(message);
		}
		
		break;
		case B_SET_PROPERTY:
		{
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
				if(what == B_DIRECT_SPECIFIER) {
					BMessage reply(B_REPLY);
					bool handled = true;
					status_t result = B_OK;
					
					if(strcmp(property, TEAM_MODEL_ENTRY_PROP_PRIORITY) == 0) {
						CTeam team(id);
						
						int32 prio;
						
						result = message->FindInt32("data", &prio);
						
						if(result == B_OK) {
							if(prio >= 0 && prio <= 120)
								result = team.SetBasePriority(prio);
							else
								result = B_BAD_VALUE;
						}
					} else {
						handled = false;
					}
					
					if(handled) {
						send_script_reply(reply, result, message);
						message->PopSpecifier();
						
						return;
					}
				}
			}
			
			BHandler::MessageReceived(message);
		}
		
		break;
		case B_EXECUTE_PROPERTY:
		{
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
				if(what == B_DIRECT_SPECIFIER) {
					BMessage reply(B_REPLY);
					bool handled = true;
					enumTeamAction action = TEAM_ACTION_NONE;
					status_t result = B_OK;
					
					if(strcmp(property, TEAM_MODEL_ENTRY_PROP_KILL) == 0) {
						action = TEAM_ACTION_KILL;
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_ACTIVATE) == 0) {
						CTeam team(id);
						result = team.Activate();
					} else if(strcmp(property, TEAM_MODEL_ENTRY_PROP_QUIT) == 0) {
						action = TEAM_ACTION_QUIT;
					} else {
						handled = false;
					}
					
					if(handled) {
						if(action != TEAM_ACTION_NONE) {
							BMessenger errorMessenger;
							BInvoker *invoker = NULL;
						
							if(message->FindMessenger("errorMessenger", &errorMessenger) == B_OK) { 
								invoker = new BInvoker(
									new BMessage(MSG_ASYNC_TEAM_ACTION_FAILED), 
									errorMessenger);
							}
						
							CAsyncMessageTeamAction *teamAction = 
								new CAsyncMessageTeamAction(new CTeam(id), action, invoker);
							
							result = teamAction->Run();
						}
					
						send_script_reply(reply, result, message);
						message->PopSpecifier();
						
						return;
					}
				}
			}
			
			BHandler::MessageReceived(message);
		}
		
		break;
		
		default:
			BHandler::MessageReceived(message);
	}	
}

// ====== CTeamModel ======

CTeamModel::CTeamModel(BLooper *looper) :
	BHandler("TeamModel")
{
	MY_ASSERT(looper != NULL);

	looper->AddHandler(this);

	messageRunner = NULL;

	// Tell roster to send messages, when an (desktop)
	// application is launched or closed.
	be_roster->StartWatching(this);	
}

CTeamModel::~CTeamModel()
{
	delete messageRunner;

	be_roster->StopWatching(this);
}

void CTeamModel::AddTeamModelListener(ITeamModelListener *l)
{
	listeners.AddItem(l);
}

void CTeamModel::RemoveTeamModelListener(ITeamModelListener *l)
{
	listeners.RemoveItem(l);
}

void CTeamModel::StartUpdate(bigtime_t pulseRate)
{
	if(pulseRate == 0)
		pulseRate = NORMAL_PULSE_RATE;

	SetPulseRate(pulseRate);
	Update();
}

void CTeamModel::AddTeam(team_id id)
{
	team_info teamInfo;
	get_team_info(id, &teamInfo);

	AddTeam(&teamInfo);	
}

void CTeamModel::AddTeam(team_info *teamInfo)
{
	CTeamModelEntry *entry = new CTeamModelEntry(*teamInfo);

	if(entry->InitCheck() == B_OK) {
		entryList.AddItem(entry);
		
		Looper()->AddHandler(entry);
		
		BMessage msg;
		
		int32 index = entryList.CountItems()-1;
		
		msg.AddPointer(MESSAGE_DATA_ID_ITEM, entry);
		msg.AddInt32(MESSAGE_DATA_ID_INDEX, index);
		msg.AddInt32(MESSAGE_DATA_ID_TEAM_ID, entry->TeamId());
		
		SendNotices(MSG_NOTIFY_ITEM_ADDED, &msg);
		
		for(int i=0 ; i<listeners.CountItems() ; i++) {
			ITeamModelListener *l = static_cast<ITeamModelListener *>(listeners.ItemAt(i));
			
			l->ItemAdded(entry, index);
		}
	}
}

void CTeamModel::RemoveEntryAt(int index)
{
	CTeamModelEntry *entry = entryList.RemoveItem(index);
	Looper()->RemoveHandler(entry);

	BMessage msg;

	msg.AddPointer(MESSAGE_DATA_ID_ITEM, entry);
	msg.AddInt32(MESSAGE_DATA_ID_INDEX, index);
	msg.AddInt32(MESSAGE_DATA_ID_TEAM_ID, entry->TeamId());
	
	SendNotices(MSG_NOTIFY_ITEM_REMOVED, &msg);

	for(int i=0 ; i<listeners.CountItems() ; i++) {
		ITeamModelListener *l = static_cast<ITeamModelListener *>(listeners.ItemAt(i));
		
		l->ItemRemoved(entry, index);
	}
	
	delete entry;
}

void CTeamModel::RemoveTeam(team_id id)
{
	for(int i=0 ; i<entryList.CountItems() ; i++) {
		if(entryList.ItemAt(i)->TeamId() == id) {
			RemoveEntryAt(i);
			break;
		}
	}
}

int32 CTeamModel::CountEntries() const
{
	return entryList.CountItems();
}

CTeamModelEntry *CTeamModel::EntryAt(int32 index)
{
	return entryList.ItemAt(index);
}

void CTeamModel::Update()
{
	// create a list of all teams.
	CPointerList<team_info> teamList;

	team_info *teamInfo=new team_info;
	int32 cookie=0;
	
	while(get_next_team_info(&cookie, teamInfo) == B_OK) {
		teamList.AddItem(teamInfo);
		teamInfo = new team_info;
	}		

	// delete last team_info, which wasn't added to list
	delete teamInfo;

	// remove all all entries from the list, which aren't part of
	// the teamList. Update all other entries.
	for(int i=0 ; i<entryList.CountItems() ; i++) {
		bool found = false;
	
		for(int k=0 ; k<teamList.CountItems() ; k++) {
			CTeamModelEntry *listEntry = entryList.ItemAt(i);
			team_info *teamListItem = teamList.ItemAt(k);
			
			if(listEntry->TeamId() == teamListItem->team) {
				// found in team list. 
				
				// Update the entry
				listEntry->Update(*teamListItem);
				
				teamListItem->team = INT_MAX;
				
				found = true;
				break;
			}
		}
		
		if(!found) {
			// not in team list. Remove from list view.
			RemoveEntryAt(i--);
		}
	}
	
	// add all new entries.
	for(int i=0 ; i<teamList.CountItems() ; i++) {
		team_info *item = (team_info *)teamList.ItemAt(i);

		if(item->team != INT_MAX) {
			AddTeam(item);
		}
	}
}

void CTeamModel::SetPulseRate(bigtime_t rate)
{
	delete messageRunner;
	
	messageRunner = new BMessageRunner(BMessenger(this), 
		new BMessage(MSG_DESKBAR_PULSE), rate, -1);
}

void CTeamModel::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_DESKBAR_PULSE:
			{
				Update();
			}
			break;
		case MSG_FAST_UPDATE:
			 {
			 	SetPulseRate(FAST_PULSE_RATE);
			 }
			 break;
		case MSG_SLOW_UPDATE:
			 {
			 	SetPulseRate(SLOW_PULSE_RATE);
			 }
			 break;
		case MSG_NORMAL_UPDATE:
			 {
			 	SetPulseRate(NORMAL_PULSE_RATE);
			 }
			 break;
		case B_SOME_APP_LAUNCHED:
			{
				team_id id;
				
				if(message->FindInt32("be:team", &id) == B_OK) {
					AddTeam(id);
				}
			}
			break;
		case B_SOME_APP_QUIT:
			{
				team_id id;		
			
				if(message->FindInt32("be:team", &id) == B_OK) {
					RemoveTeam(id);
				}
			}
			break;
		case B_COUNT_PROPERTIES:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(message->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					if(strcmp(property, TEAM_MODEL_PROP_TEAM) == 0 && 
					   what == B_DIRECT_SPECIFIER) {
						message->PopSpecifier();

						BMessage reply(B_REPLY);
						reply.AddInt32("result", entryList.CountItems());

						send_script_reply(reply, B_OK, message);
						
						return;
					}
				}
				
				BHandler::MessageReceived(message);
			}
			break;
		default:
			BHandler::MessageReceived(message);
	}
}

BHandler *CTeamModel::ResolveSpecifier(BMessage *message, int32 index, BMessage *specifier, int32 what, const char *property)
{
	if(strcmp(property, TEAM_MODEL_PROP_TEAM) == 0) {
		BMessage reply;
	
		if(message->what == B_COUNT_PROPERTIES && what == B_DIRECT_SPECIFIER) {
			return this;
		}
		
		if(what == B_NAME_SPECIFIER) {
			const char *name;
			
			if(specifier->FindString("name", &name) == B_OK) {
				for(int i=0 ; i<CountEntries() ; i++) {
					if(strcmp(EntryAt(i)->Name(), name) == 0) {
						message->PopSpecifier();
						return EntryAt(i);
					}
				}
			}
			send_script_reply(reply, B_BAD_VALUE, message);
			return NULL;
		}
		
		if(what == B_ID_SPECIFIER) {
			int32 id;
		
			if(specifier->FindInt32("id", &id) == B_OK) {
				for(int i=0 ; i<CountEntries() ; i++) {
					if(EntryAt(i)->TeamId() == id) {
						message->PopSpecifier();
						return EntryAt(i);
					}
				}
			}
			
			send_script_reply(reply, B_BAD_VALUE, message);
			return NULL;
		}
		
		if(what == B_INDEX_SPECIFIER) {
			int32 index;
		
			if(specifier->FindInt32("index", &index) == B_OK) {
				if(index >= 0 && index < CountEntries()) {
					message->PopSpecifier();
					return EntryAt(index);
				} else {
					send_script_reply(reply, B_BAD_INDEX, message);
					return NULL;
				}
			}
			send_script_reply(reply, B_BAD_VALUE, message);
			return NULL;
		}
		
		if(what == B_REVERSE_INDEX_SPECIFIER) {
			int32 index;

			if(specifier->FindInt32("index", &index) == B_OK) {
				if(index >= 0 && index<CountEntries()) {
					message->PopSpecifier();
					return EntryAt(CountEntries()-index-1);
				} else {
					send_script_reply(reply, B_BAD_INDEX, message);
					return NULL;
				}
			}

			send_script_reply(reply, B_BAD_VALUE, message);
			return NULL;
		}
	}

	return BHandler::ResolveSpecifier(message, index, specifier, what, property);
}

status_t CTeamModel::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)TEAM_MODEL_PROP_TEAM,			// name
			{ B_COUNT_PROPERTIES, 0 },				// commands
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)TEAM_MODEL_PROP_TEAM,			// name
			{ 0 },									// commands
			{ 										// specifiers
				B_INDEX_SPECIFIER, 
				B_REVERSE_INDEX_SPECIFIER,
				B_ID_SPECIFIER,
				B_NAME_SPECIFIER,
				0 
			},				
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-TeamModel");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return BHandler::GetSupportedSuites(message);
}
