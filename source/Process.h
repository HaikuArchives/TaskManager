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

#ifndef TM_PROCESS_H
#define TM_PROCESS_H

enum enumTeamAction
{
	TEAM_ACTION_NONE,
	TEAM_ACTION_ACTIVATE,
	TEAM_ACTION_KILL,
	TEAM_ACTION_QUIT,
	TEAM_ACTION_SET_PRIORITY,
};

class CTeam
{
	public:
	CTeam(team_id id);

	team_id TeamId() const { return teamId; }
	
	status_t GetBasePriority(int32 &basePrio);
	status_t SetBasePriority(int32 newBase);
	
	status_t Kill();
	status_t Activate();

	status_t AsyncKill();
	status_t AsyncQuit();
	
	status_t GetDebuggerTeam(team_id &debuggerTeam);
	status_t GetPath(BPath *path);
	status_t GetName(BString *name);
	
	bool IsSystemTeam();
	bool IsIdleTeam();
	
	bool IsValid();
	
	protected:
	team_id teamId;
};

class CAsyncTeamAction
{
	public:
	CAsyncTeamAction(CTeam *team, enumTeamAction teamAction);
	virtual ~CAsyncTeamAction();

	virtual status_t Run();

	virtual void Error(status_t errorCode) = 0;
	virtual void Success() = 0;
	
	protected:
	static int32 ThreadFunc(void *data);
	
	virtual status_t AsyncAction();
	
	CTeam *team;
	enumTeamAction teamAction;
	thread_id threadId;
};

class CAsyncMessageTeamAction : public CAsyncTeamAction
{
	public:
	CAsyncMessageTeamAction(CTeam *team, enumTeamAction teamAction, BInvoker *_invoker);
	virtual ~CAsyncMessageTeamAction();

	virtual void Success() {}
	virtual void Error(status_t errorCode);
	
	protected:
	BInvoker *invoker;
};

#endif // TM_PROCESS_H