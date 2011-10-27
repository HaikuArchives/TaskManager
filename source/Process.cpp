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
#include "my_assert.h"
#include "common.h"
#include "ProcessView.h"
#include "Process.h"

// ====== globals =====

const char * const MESSAGE_DATA_ID_ERROR_CODE		= "TEAMFAIL:ErrorCode";

// ====== CTeam ======

// Constructor
CTeam::CTeam(team_id id) : 
	teamId(id)
{
}

// GetBasePriority
// see SetBasePriority
status_t CTeam::GetBasePriority(int32 &basePrio)
{
	thread_info threadInfo;
	int32 cookie;

	int32 prio=0, threadCount;

	for(threadCount=0, cookie=0 ; get_next_thread_info(TeamId(), &cookie, &threadInfo) == B_OK ; threadCount++) {
		prio += threadInfo.priority;
	}

	if(threadCount <= 0) {
		// No threads?? The team_id seems to be invalid.
		basePrio = 0;
		return B_BAD_VALUE;
	}

	basePrio = prio / threadCount;
	
	return B_OK;
}

// SetBasePriority
// Under BeOS you can't change the priority of team. Only of a thread.
// Therefor I'm using a construct called 'base priority'. The base prio
// is the average prio of all the threads of a team.
// When you change the base prio the relative priorities of the threads
// remain untouched.
// If you set the base priority of a team the algorithm doesn't touch
// the priority values of realtime threads. It also keeps the priority
// values of the other threads in the range 0-99. This behaviour results
// sometimes in an unchanged average priority, if the team contains
// many realtime threads. This ISN'T a bug.
status_t CTeam::SetBasePriority(int32 newBase)
{
	thread_info threadInfo;
	int32 cookie;
	int32 oldBase;
	
	RETURN_IF_FAILED( GetBasePriority(oldBase) );

	for(cookie=0 ; get_next_thread_info(TeamId(), &cookie, &threadInfo) == B_OK ; ) {
		if(threadInfo.priority < 100) {
			int32 delta = threadInfo.priority-oldBase;
		
			int32 newPrio = newBase + delta;
		
			// timesharing threads are in the prio range between 0 and 99.
			if(newPrio < 0)		newPrio = 0;
			if(newPrio > 99)	newPrio = 99;
	
			RETURN_IF_FAILED( set_thread_priority(threadInfo.thread, newPrio) );
		}
	}
	
	return B_OK;
}

// AsyncQuit
// Send a B_QUIT_REQUESTED message to the team. If the team
// isn't responding on this request, this operation times out.
status_t CTeam::AsyncQuit()
{
	status_t error;
	
	BMessenger appMessenger(NULL, teamId, &error);
	
	if(error == B_OK) {
		BMessage quitMessage(B_QUIT_REQUESTED);
		BMessage reply/*ignored*/;
		
		const bigtime_t timeout = 10000000; // 10 secs timeout
		
		error = appMessenger.SendMessage(&quitMessage, &reply, timeout, timeout);
	}
	
	return error;
}

// Kill
// Kills the team. Returns E_CONTROLLED_BY_DEBUGGER, if the team is
// controlled by a debugger.
status_t CTeam::Kill()
{
	team_id dbTeamId;
	
	if(GetDebuggerTeam(dbTeamId) == B_OK) {
		// team is controlled by a debugger. The kill
		// will fail, when the team is breaked (by a
		// break point). BUT kill_team will return B_OK.
		
		return E_CONTROLLED_BY_DEBUGGER;
	}

	return kill_team(teamId);
}

// AsyncKill
// The difference between Kill and AsyncKill is, that AsyncKill retries to kill
// the team until it's eigther dead or the operation times out (after 10 secs).
// This method is called by CAsyncTeamAction. You shouldn't call it without
// a CAsyncTeamAction, because it could block your code for 10 secs.
status_t CTeam::AsyncKill()
{
	const bigtime_t timeout		= 10000000; // 10 secs timeout
	const bigtime_t sleepTime	= 100000;	// 1/10 sec.

	bigtime_t startTime = system_time();
	status_t result;

	// try to kill the team.
	RETURN_IF_FAILED( Kill() );

	for(bigtime_t time=0 ; time < timeout ; time=system_time()-startTime) {
		result  = kill_team(teamId);
		
		if(result == B_BAD_TEAM_ID) {
			// the team seems to be dead.
			return B_OK;
		}
		
		// team still around. Sleep and try again later.
		snooze(sleepTime);
	}
	
	return B_TIMED_OUT;
}

status_t CTeam::GetPath(BPath *path)
{
	if(!path) return B_BAD_VALUE;

	image_info imageInfo;
	int32 cookie=0;
		
	// look for main application image
	while(get_next_image_info(teamId, &cookie, &imageInfo) == B_OK) {
		if(imageInfo.type == B_APP_IMAGE) {
			path->SetTo(imageInfo.name);
			return B_OK;
		}
	}	

	return B_ERROR;	
}

status_t CTeam::GetName(BString *name)
{
	if(!name) return B_BAD_VALUE;

	BPath path;
	
	RETURN_IF_FAILED( GetPath(&path) );
	
	*name = path.Leaf();
	
	return B_OK;
}

// GetDebuggerInfo
// Returns the debugger. If this team has no debugger it returns B_BAD_VALUE.
status_t CTeam::GetDebuggerTeam(team_id &debuggerTeam)
{
	team_info teamInfo;

	debuggerTeam  = -1;
	
	RETURN_IF_FAILED( get_team_info(teamId, &teamInfo) );
	
	// The team_info struct contains 0 as debugger_nub_thread, if there is
	// no debugger controlling this thread. I think this is a bug, because
	// 0 is a valid team_id. Only negative values are invalid...
	
	if(teamInfo.debugger_nub_thread > 0 &&
	   teamInfo.debugger_nub_port > 0) {

		port_info dbPortInfo;
		
		RETURN_IF_FAILED( get_port_info(teamInfo.debugger_nub_port, &dbPortInfo) );
		
		debuggerTeam = dbPortInfo.team;
		
		return B_OK;
	}
	
	return B_BAD_VALUE;
}

// Activate
// Brings a window of this team to the front.
// Of course this only works, if the team has a window.
status_t CTeam::Activate()
{
	return be_roster->ActivateApp(teamId);
}

bool CTeam::IsValid()
{
	if(teamId == B_SYSTEM_TEAM)
		return true;

	team_info teamInfo;

	status_t getTeamInfoResult = get_team_info(teamId, &teamInfo);
	
	if(getTeamInfoResult != B_OK) {
		return false;
	}
	
	if(teamInfo.image_count == 0) {
		// When a team is shutdown get_team_info() might return B_OK
		// although the team is almost unloaded from memory.
		// Then the image_count is 0, but the tread and area count are
		// still valid.
		return false;
	}
	
	return true;
}

bool CTeam::IsIdleTeam()
{
	return teamId == B_SYSTEM_TEAM;
}

bool CTeam::IsSystemTeam()
{
	BPath fileName;
	GetPath(&fileName);

	BEntry appEntry(fileName.Path());

	BPath systemDirPath;
	find_directory(B_BEOS_SYSTEM_DIRECTORY, &systemDirPath);
	BDirectory systemDir(systemDirPath.Path());

	if(systemDir.Contains(&appEntry)) {
		// all teams which have their image in the system directory 
		// (or one of its subdirs) are system teams.
		return true;
	}

	return false;
}

// ====== CAsyncTeamAction ======

// Constructor
// Executes a async operation on a team. Always create an instance
// on this class through the new operator. Never create an instance
// on the stack.
// This object is automatically deleted, when the operation is
// finished, as well as the CTeam object passed to the constructor.
// Normally only the CTeam::Quit() operation is executed asynchronously,
// because only this operation can take long time.
CAsyncTeamAction::CAsyncTeamAction(CTeam *_team, enumTeamAction _teamAction)
{
	team = _team;
	teamAction = _teamAction;
	
	char name[255];
	
	sprintf(name, "AsynTeamAction:%ld", team->TeamId());
	
	threadId = spawn_thread(ThreadFunc, name, B_NORMAL_PRIORITY, this);
}

CAsyncTeamAction::~CAsyncTeamAction()
{
	delete team;
}

status_t CAsyncTeamAction::Run()
{
	return resume_thread(threadId);
}

int32 CAsyncTeamAction::ThreadFunc(void *data)
{
	CAsyncTeamAction *thisObj = reinterpret_cast<CAsyncTeamAction *>(data);
	
	return thisObj->AsyncAction();
}

status_t CAsyncTeamAction::AsyncAction()
{
	status_t result=B_ERROR;
	
	switch(teamAction) {
		case TEAM_ACTION_ACTIVATE:
			result = team->Activate();
			break;
		case TEAM_ACTION_KILL:
			result = team->AsyncKill();
			break;
		case TEAM_ACTION_QUIT:
			result = team->AsyncQuit();
			break;
		case TEAM_ACTION_SET_PRIORITY:
			// not implemented, because I don't know the new priority...
			break;
		default:
			MY_ASSERT(!"Invalid teamAction value.");
	}
	
	if(result != B_OK) {
		Error(result);
	} else {
		Success();
	}

	// This isn't a really save way to do this.
	// It should work if you don't try to access
	// this object from an other thread. NORMALLY.
	delete this;
	
	return result;
}

// ====== CAsyncMessageTeamAction ======

// Constructor
// This class executes an async team action and sends a message
// if this operation fails.
// The invoker passed to this class must contain an valid message
// and destination. When the message is sent, it contains following
// data fields:
// * MESSAGE_DATA_ID_ERROR_CODE:	Code of the error
// * MESSAGE_DATA_ID_TEAM_ID:		ID of the team
// * MESSAGE_DATA_ID_ACTION:		Which operation failed?
//
// All objects passed to this constructor are automatically deleted.
// Don't use them after you passed them.
CAsyncMessageTeamAction::CAsyncMessageTeamAction(CTeam *team,
	enumTeamAction teamAction, BInvoker *_invoker) :
	CAsyncTeamAction(team, teamAction)
{
	invoker = _invoker;
}

CAsyncMessageTeamAction::~CAsyncMessageTeamAction()
{
	delete invoker;
}

void CAsyncMessageTeamAction::Error(status_t errorCode)
{
	if(invoker) {
		BMessage *message = invoker->Message();
		
		message->AddInt32(MESSAGE_DATA_ID_ERROR_CODE, errorCode);
		message->AddInt32(MESSAGE_DATA_ID_TEAM_ID, team->TeamId());
		message->AddInt32(MESSAGE_DATA_ID_ACTION, teamAction);
		
		invoker->Invoke();
	}
}