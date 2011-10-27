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
 
#ifndef DEFAULT_PLUGIN_H
#define DEFAULT_PLUGIN_H

#include "PerformanceCounter.h"

class CDefaultPlugin : public IPerformanceCounterPlugin
{
	public:
	CDefaultPlugin();
	
	virtual status_t EnumChildren(IPerformanceCounterNamespace *counterNamespace, 
							const char *path, 
							BList *children);
};

class CTeamPerformanceCounter : public CPerformanceCounter
{
	public:
	CTeamPerformanceCounter(
		team_id id, 
		IPerformanceCounterNamespace *counterNamespace, 
		const char *parentPath, 
		const char *internalName,
		IDataProvider *dataProvider);

	virtual void InitChildren();
	virtual const char *Name() const;
	
	protected:
	team_id teamId;	
	BString name;
};

class CVolumePerformaceCounter : public CPerformanceCounter
{
	public:
	CVolumePerformaceCounter(
		const BVolume &_volume,
		IPerformanceCounterNamespace *counterNamespace,
		const char *parentPath,
		const char *internalName,
		IDataProvider *dataProvider);
		
	virtual void InitChildren();
	
	protected:
	BVolume volume;
};

class CThreadRootPerformanceCounter : public CPerformanceCounter
{
	public:
	CThreadRootPerformanceCounter(
		team_id id, 
		IPerformanceCounterNamespace *counterNamespace, 
		const char *parentPath, 
		const char *name,
		IDataProvider *dataProvider);

	virtual void InitChildren();
	
	protected:
	team_id teamId;	
};

class CThreadPerformanceCounter : public CPerformanceCounter
{
	public:
	CThreadPerformanceCounter(
		thread_id _threadId,
		IPerformanceCounterNamespace *counterNamespace, 
		const char *parentPath, 
		const char *internalName,
		IDataProvider *dataProvider);

	virtual void InitChildren();
	virtual const char *Name() const;
	
	protected:
	thread_id threadId;
	BString name;
};

extern "C" status_t __declspec(dllexport) CreateCounterPlugin(IPerformanceCounterPlugin **plugin);

#endif // DEFAULT_PLUGIN_H
