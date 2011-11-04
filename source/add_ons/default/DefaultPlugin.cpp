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
#include "Process.h"
#include "Plugin.h"
#include "NameInfo.h"
#include "DefaultDataProvider.h"
#include "DefaultPlugin.h"

// ==== globals ====

// declared in NameInfo.h
BString team_name(team_id id)
{
	BString name;
	BPath teamPath;
		
	image_info imageInfo;
	int32 cookie=0;

	// look for main application image
	while(get_next_image_info(id, &cookie, &imageInfo) == B_OK) {
		if(imageInfo.type == B_APP_IMAGE) {
			teamPath.SetTo(imageInfo.name);
			name = teamPath.Leaf();
		}
	}
			
	if(name == "") {
		team_info teamInfo;

		if(get_team_info(id, &teamInfo) == B_OK)
			name = teamInfo.args;
	}

	if(name == "") {
		name << "Team #" << id;
	}
	
	return name;
}

// declared in NameInfo.h
BString thread_name(thread_id id)
{
	thread_info threadInfo;
	BString name;
	
	if(get_thread_info(id, &threadInfo) == B_OK) {
		name = thread_name(threadInfo);
	}
	
	return name;
}

// declared in NameInfo.h
BString thread_name(const thread_info &threadInfo)
{
	BString name;
	
	if(threadInfo.name[0] == '\0')
		name << "Thread #" << (int32)threadInfo.thread;
	else 
		name = threadInfo.name;
		
	return name;
}

// ==== entry point ====

status_t CreateCounterPlugin(IPerformanceCounterPlugin **plugin)
{
	*plugin = new CDefaultPlugin();

	return (*plugin) != NULL ? B_OK : B_NO_MEMORY;
}

// ==== CDefaultPlugin ====

CDefaultPlugin::CDefaultPlugin()
{
}
	
status_t CDefaultPlugin::EnumChildren(
	IPerformanceCounterNamespace *counterNamespace, 
	const char *path, 
	BList *children)
{
	if(strcmp(path, "/") == 0) {
		// Root
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Total", NULL));
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Volumes", NULL));
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Teams", NULL));
	} else if(strcmp(path, "/Total") == 0) {
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "CPU Usage", NULL));
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Memory Usage", new CMemoryDataProvider()));
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Thread Count", new CThreadCountDataProvider()));
		children->AddItem(new CPerformanceCounter(counterNamespace, path, "Team Count", new CTeamCountDataProvider()));
	} else if(strcmp(path, "/Volumes") == 0) {
		BVolumeRoster volumeRoster;	
		BVolume volume;
		
		while(volumeRoster.GetNextVolume(&volume) == B_OK) {
			if(volume.IsPersistent()) {
				char volumeName[255];
				
				volume.GetName(volumeName);
			
				children->AddItem(
					new CVolumePerformaceCounter(volume, counterNamespace, path, volumeName, NULL));
			}
		}
	} else if(strcmp(path, "/Total/CPU Usage") == 0) {
		system_info systemInfo;
		get_system_info(&systemInfo);
		
		for(int32 i=0 ; i<systemInfo.cpu_count ; i++) {
			char newItemName[255];
			
			sprintf(newItemName, "CPU %ld", i+1);
		
			children->AddItem(new CPerformanceCounter(counterNamespace, path, 
				newItemName, new CCPUDataProvider(i)));
		}
		
		children->AddItem(new CPerformanceCounter(counterNamespace, path, 
				"Average", new CCPUDataProvider(CCPUDataProvider::CPU_NUM_ALL)));
		
	} else if(strcmp(path, "/Teams") == 0) {
		int32 cookie=0;
		team_info teamInfo;

		while(get_next_team_info(&cookie, &teamInfo) == B_OK) {
			BString name;

			name << teamInfo.team;
		
			children->AddItem(new CTeamPerformanceCounter(teamInfo.team, 
				counterNamespace, path, name.String(), NULL));
		}		
	} 
	
	return B_OK;
}

// ==== CTeamPerformanceCounter ====

CTeamPerformanceCounter::CTeamPerformanceCounter(
	team_id id, 
	IPerformanceCounterNamespace *counterNamespace, 
	const char *parentPath, 
	const char *internalName,
	IDataProvider *dataProvider) :
	CPerformanceCounter(counterNamespace, parentPath, internalName, dataProvider)
{
	teamId = id;
	
	name = team_name(teamId);
}

void CTeamPerformanceCounter::InitChildren()
{
	if(initChildren) {
		children.AddItem(new CPerformanceCounter(namesp, Path(), "CPU Usage", 
			new CTeamCPUDataProvider(teamId)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Memory Usage", 
			new CTeamMemoryDataProvider(teamId)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Thread Count",
			new CTeamThreadCountDataProvider(teamId)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Area Count", 
			new CTeamAreaCountDataProvider(teamId)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Image Count", 
			new CTeamImageCountDataProvider(teamId)));
		
		children.AddItem(new CThreadRootPerformanceCounter(teamId, namesp, Path(), 
			"Threads", NULL));
	}
	
	CPerformanceCounter::InitChildren();
}

const char *CTeamPerformanceCounter::Name() const
{
	return name.String();
}

// ==== CVolumPerformaceCounter

CVolumePerformaceCounter::CVolumePerformaceCounter(
	const BVolume &_volume,
	IPerformanceCounterNamespace *counterNamespace,
	const char *parentPath,
	const char *internalName,
	IDataProvider *dataProvider) : 
	CPerformanceCounter(counterNamespace, parentPath, internalName, dataProvider)
{
	volume = _volume;
}

void CVolumePerformaceCounter::InitChildren()
{
	if(initChildren) {
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Used Bytes",
			new CVolumeUsageAbsoluteDataProvider(volume)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Capacity",
			new CVolumeCapacityDataProvider(volume)));
		children.AddItem(new CPerformanceCounter(namesp, Path(), "Usage (%)",
			new CVolumeUsageDataProvider(volume)));
	}
	
	CPerformanceCounter::InitChildren();
}

// ==== CThreadRootPerformanceCounter ====

CThreadRootPerformanceCounter::CThreadRootPerformanceCounter(
	team_id id, 
	IPerformanceCounterNamespace *counterNamespace, 
	const char *parentPath, 
	const char *name,
	IDataProvider *dataProvider) :
	CPerformanceCounter(counterNamespace, parentPath, name, dataProvider)
{
	teamId = id;
}

void CThreadRootPerformanceCounter::InitChildren()
{
	if(initChildren) {
		thread_info threadInfo;
		int32 cookie = 0;
	
		while(get_next_thread_info(teamId, &cookie, &threadInfo) == B_OK) {
			BString name;
			
			name << threadInfo.thread;
		
			children.AddItem(new CThreadPerformanceCounter(threadInfo.thread,
				namesp, Path(), name.String(), NULL));
		}
	}
	
	CPerformanceCounter::InitChildren();
}

// ==== CThreadPerformanceCounter =====

CThreadPerformanceCounter::CThreadPerformanceCounter(
	thread_id _threadId,
	IPerformanceCounterNamespace *counterNamespace, 
	const char *parentPath, 
	const char *internalName,
	IDataProvider *dataProvider) :
	CPerformanceCounter(counterNamespace, parentPath, internalName, dataProvider)
{
	threadId = _threadId;
	
	name = thread_name(threadId);
}

void CThreadPerformanceCounter::InitChildren()
{
	if(initChildren) {
		children.AddItem(new CPerformanceCounter(namesp, Path(), "CPU Usage", 
			new CThreadCPUDataProvider(threadId)));
	}
	
	CPerformanceCounter::InitChildren();
}

const char *CThreadPerformanceCounter::Name() const
{
	return name.String();
}
