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
#include "SystemInfo.h"
#include "NameInfo.h"
#include "DataProvider.h"
#include "DefaultDataProvider.h"

// ====== globals ======

// archive fields
const char * const DATA_PROVIDER_ARCHIVE_CPU_NUM	= "DATAPROVIDER:CpuNum";
const char * const DATA_PROVIDER_ARCHIVE_TEAM_ID	= "DATAPROVIDER:TeamId";
const char * const DATA_PROVIDER_ARCHIVE_THREAD_ID	= "DATAPROVIDER:ThreadId";
const char * const DATA_PROVIDER_ARCHIVE_DEV_ID		= "DATAPROVIDER:DevId";

const char * const ADD_ON_SIGNATURE = "application/x-vnd.task_manager_default";

const float MEGA_BYTE = 1024*1024;

// ==== CDefaultDataProviderBase =====

CDefaultDataProviderBase::CDefaultDataProviderBase()
{
}

CDefaultDataProviderBase::CDefaultDataProviderBase(BMessage *archive) :
	CArchivableDataProvider(archive)
{
}
	
status_t CDefaultDataProviderBase::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( CArchivableDataProvider::Archive(archive, deep) );
	RETURN_IF_FAILED( archive->AddString("add_on", ADD_ON_SIGNATURE) );
	
	return B_OK;
}

// ==== CCPUDataProvider ====

CCPUDataProvider::CCPUDataProvider(int32 _cpuNum)
{
	cpuNum = _cpuNum;
	Init();
}

CCPUDataProvider::CCPUDataProvider(BMessage *archive) :
	BArchivable(archive)
{
	archive->FindInt32(DATA_PROVIDER_ARCHIVE_CPU_NUM, &cpuNum);
	Init();
}

void CCPUDataProvider::Init()
{
	lastActiveTime = 0;

	system_info sysInfo;
	get_system_info(&sysInfo);
	cpu_info* cpuInfos = new cpu_info[sysInfo.cpu_count];
	get_cpu_info(0, sysInfo.cpu_count, cpuInfos);

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		if(cpuNum == CPU_NUM_ALL) {
			// average usage of all cpu's
			for(uint32 i=0 ; i<sysInfo.cpu_count ; i++) {
				lastActiveTime += cpuInfos[i].active_time;
			}
		} else {
			lastActiveTime = cpuInfos[cpuNum].active_time;
		}
	}
	delete[] cpuInfos;
}

status_t CCPUDataProvider::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( BArchivable::Archive(archive, deep) );

	RETURN_IF_FAILED( archive->AddString("add_on", ADD_ON_SIGNATURE) );
	RETURN_IF_FAILED( archive->AddInt32(DATA_PROVIDER_ARCHIVE_CPU_NUM, cpuNum) );
	
	return B_OK;
}

BArchivable *CCPUDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CCPUDataProvider"))
		return NULL;

	return new CCPUDataProvider(archive);
}

bool CCPUDataProvider::GetNextValue(float &value)
{
	system_info sysInfo;
	get_system_info(&sysInfo);
	cpu_info* cpuInfos = new cpu_info[sysInfo.cpu_count];
	get_cpu_info(0, sysInfo.cpu_count, cpuInfos);

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		// current accumulated CPU active time
		bigtime_t activeTime=0;

		uint32 cpuCount = 1;

		if(cpuNum == CPU_NUM_ALL) {
			// show average usage of all cpu's
		
			for(uint32 i=0 ; i<sysInfo.cpu_count ; i++) {
				activeTime += cpuInfos[i].active_time;
			}
			
			cpuCount = sysInfo.cpu_count;
		} else {
			activeTime = cpuInfos[cpuNum].active_time;
		}

		if(lastActiveTime != 0) {
			value = (activeTime - lastActiveTime) / (float)cpuCount;

			lastActiveTime = activeTime;	
			
			return true;
		}

		lastActiveTime = activeTime;	
	}
	delete[] cpuInfos;
	return false;
}

BString CCPUDataProvider::DisplayName()
{
	if(cpuNum != CPU_NUM_ALL)
		return BString("CPU ") << (cpuNum+1);
	else
		return BString("Average CPU Usage");
}

bool CCPUDataProvider::Equal(IDataProvider *other)
{
	CCPUDataProvider *obj = dynamic_cast<CCPUDataProvider *>(other);
	
	return (obj && obj->cpuNum == cpuNum);
}

// ==== CMemoryDataProvider ====

CMemoryDataProvider::CMemoryDataProvider()
{
}

CMemoryDataProvider::CMemoryDataProvider(BMessage *archive) :
	CDefaultDataProviderBase(archive)
{
}

BArchivable *CMemoryDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CMemoryDataProvider"))
		return NULL;
		
	return new CMemoryDataProvider(archive);
}

bool CMemoryDataProvider::GetNextValue(float &value)
{
	system_info sysInfo;

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		value = sysInfo.used_pages;
		return true;
	}
	
	return false;
}

bool CMemoryDataProvider::Equal(IDataProvider *other)
{
	return dynamic_cast<CMemoryDataProvider *>(other) != NULL;
}

// ===== CThreadCountDataProvider =====

CThreadCountDataProvider::CThreadCountDataProvider()
{
}

CThreadCountDataProvider::CThreadCountDataProvider(BMessage *archive) :
	CDefaultDataProviderBase(archive)
{
}

BArchivable *CThreadCountDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CThreadCountDataProvider"))
		return NULL;

	return new CThreadCountDataProvider(archive);
}

bool CThreadCountDataProvider::Equal(IDataProvider *other)
{
	return dynamic_cast<CThreadCountDataProvider *>(other) != NULL;
}
	
bool CThreadCountDataProvider::GetNextValue(float &value)
{
	system_info sysInfo;

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		value = sysInfo.used_threads;
		return true;
	}
	
	return false;
}

// ===== CTeamCountDataProvider =====

CTeamCountDataProvider::CTeamCountDataProvider()
{
}

CTeamCountDataProvider::CTeamCountDataProvider(BMessage *archive) :
	CDefaultDataProviderBase(archive)
{
}

BArchivable *CTeamCountDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamCountDataProvider"))
		return NULL;

	return new CTeamCountDataProvider(archive);
}

bool CTeamCountDataProvider::Equal(IDataProvider *other)
{
	return dynamic_cast<CTeamCountDataProvider *>(other) != NULL;
}
	
bool CTeamCountDataProvider::GetNextValue(float &value)
{
	system_info sysInfo;

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		value = sysInfo.used_teams;
		return true;
	}
	
	return false;
}

// ===== CTeamInfoDataProvider =====

CTeamInfoDataProvider::CTeamInfoDataProvider(team_id team)
{
	teamId = team;
}

CTeamInfoDataProvider::CTeamInfoDataProvider(BMessage *archive) :
	CDefaultDataProviderBase(archive)
{
	archive->FindInt32(DATA_PROVIDER_ARCHIVE_TEAM_ID, &teamId);
}

status_t CTeamInfoDataProvider::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( CDefaultDataProviderBase::Archive(archive, deep) );
	RETURN_IF_FAILED( archive->AddInt32(DATA_PROVIDER_ARCHIVE_TEAM_ID, teamId) );
	
	return B_OK;
}

bool CTeamInfoDataProvider::Equal(IDataProvider *other)
{
	CTeamInfoDataProvider *obj = dynamic_cast<CTeamInfoDataProvider *>(other);
	
	return obj && obj->teamId == teamId;
}

// ===== CTeamCPUDataProvider =====

CTeamCPUDataProvider::CTeamCPUDataProvider(team_id team) :
	CTeamInfoDataProvider(team)
{
	Init();
}

CTeamCPUDataProvider::CTeamCPUDataProvider(BMessage *archive) :
	CTeamInfoDataProvider(archive)
{
	Init();
}

void CTeamCPUDataProvider::Init()
{
	lastActiveTime = 0;
}

BArchivable *CTeamCPUDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamCPUDataProvider"))
		return NULL;

	return new CTeamCPUDataProvider(archive);
}

bool CTeamCPUDataProvider::Equal(IDataProvider *other)
{
	return CTeamInfoDataProvider::Equal(other) &&
		dynamic_cast<CTeamCPUDataProvider *>(other) != NULL;
}

bool CTeamCPUDataProvider::GetNextValue(float &value)
{
	thread_info threadInfo;

	bigtime_t userTime=0, kernelTime=0;
	bigtime_t activeTime;
	int32 cookie = 0;
	system_info sysInfo;

	if(get_cached_system_info(&sysInfo, NULL) == B_OK) {
		while(get_next_thread_info(teamId, &cookie, &threadInfo) == B_OK) {
			userTime   += threadInfo.user_time;
			kernelTime += threadInfo.kernel_time;
		}
	
		activeTime = userTime + kernelTime;

		if(lastActiveTime != 0) {
			value = (activeTime - lastActiveTime) / sysInfo.cpu_count;

			lastActiveTime = userTime + kernelTime;
		
			return true;
		}
	} 
	
	lastActiveTime = userTime + kernelTime;
	 
	return false;
}

BString CTeamCPUDataProvider::DisplayName()
{
	return (BString("CPU Usage of ") << team_name(teamId));
}

// ==== CTeamMemoryDataProvider ====

CTeamMemoryDataProvider::CTeamMemoryDataProvider(team_id team) :
	CTeamInfoDataProvider(team)
{
}

CTeamMemoryDataProvider::CTeamMemoryDataProvider(BMessage *archive) :
	CTeamInfoDataProvider(archive)
{
}

BArchivable *CTeamMemoryDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamMemoryDataProvider"))
		return NULL;

	return new CTeamMemoryDataProvider(archive);
}

bool CTeamMemoryDataProvider::Equal(IDataProvider *other)
{
	return CTeamInfoDataProvider::Equal(other) &&
		dynamic_cast<CTeamMemoryDataProvider *>(other) != NULL;
}

bool CTeamMemoryDataProvider::GetNextValue(float &value)
{
	ssize_t cookie = 0;
	size_t totalAreaSize = 0;
		
	area_info areaInfo;
		
	while(get_next_area_info(teamId, &cookie, &areaInfo) == B_OK) {
		totalAreaSize += areaInfo.ram_size;
	}

	value = totalAreaSize / 1024.0;
	
	return true;
}

BString CTeamMemoryDataProvider::DisplayName()
{
	return (BString("Mem Usage of ") << team_name(teamId));
}

// ==== CTeamThreadCountDataProvider ====

CTeamThreadCountDataProvider::CTeamThreadCountDataProvider(team_id team) :
	CTeamInfoDataProvider(team)
{
}

CTeamThreadCountDataProvider::CTeamThreadCountDataProvider(BMessage *archive) :
	CTeamInfoDataProvider(archive)
{
}

BArchivable *CTeamThreadCountDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamThreadCountDataProvider"))
		return NULL;

	return new CTeamThreadCountDataProvider(archive);
}

BString CTeamThreadCountDataProvider::DisplayName()
{
	return (BString("Thread count of ") << team_name(teamId));
}

bool CTeamThreadCountDataProvider::Equal(IDataProvider *other)
{
	return CTeamInfoDataProvider::Equal(other) &&
		dynamic_cast<CTeamThreadCountDataProvider *>(other) != NULL;
}
	
bool CTeamThreadCountDataProvider::GetNextValue(float &value)
{
	team_info teamInfo;
	
	if( GetTeamInfo(&teamInfo) == B_OK ) {
		value = teamInfo.thread_count;
		
		return true;
	}
	
	return false;
}

// ==== CTeamAreaCountDataProvider ====

CTeamAreaCountDataProvider::CTeamAreaCountDataProvider(team_id team) :
	CTeamInfoDataProvider(team)
{
}

CTeamAreaCountDataProvider::CTeamAreaCountDataProvider(BMessage *archive) :
	CTeamInfoDataProvider(archive)
{
}

BArchivable *CTeamAreaCountDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamAreaCountDataProvider"))
		return NULL;

	return new CTeamAreaCountDataProvider(archive);
}

BString CTeamAreaCountDataProvider::DisplayName()
{
	return (BString("Area count of ") << team_name(teamId));
}

bool CTeamAreaCountDataProvider::Equal(IDataProvider *other)
{
	return CTeamInfoDataProvider::Equal(other) &&
		dynamic_cast<CTeamAreaCountDataProvider *>(other) != NULL;
}
	
bool CTeamAreaCountDataProvider::GetNextValue(float &value)
{
	team_info teamInfo;
	
	if( GetTeamInfo(&teamInfo) == B_OK ) {
		value = teamInfo.area_count;
		return true;
	}
	
	return false;
}

// ==== CTeamImageCountDataProvider ====

CTeamImageCountDataProvider::CTeamImageCountDataProvider(team_id team) :
	CTeamInfoDataProvider(team)
{
}

CTeamImageCountDataProvider::CTeamImageCountDataProvider(BMessage *archive) :
	CTeamInfoDataProvider(archive)
{
}

BArchivable *CTeamImageCountDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CTeamImageCountDataProvider"))
		return NULL;

	return new CTeamImageCountDataProvider(archive);
}

BString CTeamImageCountDataProvider::DisplayName()
{
	return (BString("Image count of ") << team_name(teamId));
}

bool CTeamImageCountDataProvider::Equal(IDataProvider *other)
{
	return CTeamInfoDataProvider::Equal(other) &&
		dynamic_cast<CTeamImageCountDataProvider *>(other) != NULL;
}
	
bool CTeamImageCountDataProvider::GetNextValue(float &value)
{
	team_info teamInfo;
	
	if( GetTeamInfo(&teamInfo) == B_OK ) {
		value = teamInfo.image_count;
		return true;
	}
	
	return false;
}
	
// ==== CThreadCPUDataProvider ====

CThreadCPUDataProvider::CThreadCPUDataProvider(thread_id thread)
{
	threadId = thread;
	Init();
}

CThreadCPUDataProvider::CThreadCPUDataProvider(BMessage *archive) :
	CDefaultDataProviderBase(archive)
{
	archive->FindInt32(DATA_PROVIDER_ARCHIVE_THREAD_ID, &threadId);
	Init();
}

void CThreadCPUDataProvider::Init()
{
	lastActiveTime = 0;
}

BArchivable *CThreadCPUDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CThreadCPUDataProvider"))
		return NULL;

	return new CThreadCPUDataProvider(archive);
}

bool CThreadCPUDataProvider::Equal(IDataProvider *other)
{
	CThreadCPUDataProvider *o = dynamic_cast<CThreadCPUDataProvider *>(other);
	
	return (o && o->threadId == threadId);
}

bool CThreadCPUDataProvider::GetNextValue(float &value)
{
	thread_info threadInfo;
	system_info systemInfo;
	
	if(get_thread_info(threadId, &threadInfo) == B_OK &&
	   get_cached_system_info(&systemInfo, NULL) == B_OK) {
		bigtime_t activeTime = threadInfo.user_time + threadInfo.kernel_time;
	
		if(lastActiveTime > 0) {
			value = (activeTime - lastActiveTime) / systemInfo.cpu_count;

			lastActiveTime = activeTime;

			return true;
		}

		lastActiveTime = activeTime;
	}
	
	return false;
}

BString CThreadCPUDataProvider::DisplayName()
{
	return (BString("CPU Usage of ") << thread_name(threadId));
}

status_t CThreadCPUDataProvider::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( CDefaultDataProviderBase::Archive(archive, deep) );

	RETURN_IF_FAILED( archive->AddInt32(DATA_PROVIDER_ARCHIVE_THREAD_ID, threadId) );
	
	return B_OK;
}

// ==== CVolumeDataProviderBase ====

CVolumeDataProviderBase::CVolumeDataProviderBase(const BVolume &_volume)
{
	volume = new BVolume(_volume);
}

CVolumeDataProviderBase::CVolumeDataProviderBase(BMessage *archive)
{
	volume = NULL;

	dev_t dev;

	archive->FindInt32(DATA_PROVIDER_ARCHIVE_DEV_ID, 0, &dev);
	
	volume = new BVolume(dev);
}

CVolumeDataProviderBase::~CVolumeDataProviderBase()
{
	delete volume;
}

status_t CVolumeDataProviderBase::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( CDefaultDataProviderBase::Archive(archive, deep) );
	RETURN_IF_FAILED( archive->AddInt32(DATA_PROVIDER_ARCHIVE_DEV_ID, 
						volume->Device()) );
	
	return B_OK;
}

BString CVolumeDataProviderBase::DisplayName()
{
	char name[255];

	volume->GetName(name);
	
	return BString(name);
}

// ==== CVolumeUsageAbsoluteDataProvider ====

CVolumeUsageAbsoluteDataProvider::CVolumeUsageAbsoluteDataProvider(const BVolume &_volume) :
	CVolumeDataProviderBase(_volume)
{
}

CVolumeUsageAbsoluteDataProvider::CVolumeUsageAbsoluteDataProvider(BMessage *archive) :
	CVolumeDataProviderBase(archive)
{
}

BArchivable *CVolumeUsageAbsoluteDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CVolumeUsageAbsoluteDataProviderInstantiate"))
		return NULL;

	return new CVolumeUsageAbsoluteDataProvider(archive);
}

bool CVolumeUsageAbsoluteDataProvider::Equal(IDataProvider *other)
{
	CVolumeUsageAbsoluteDataProvider *base = dynamic_cast<CVolumeUsageAbsoluteDataProvider *>(other);
	
	return (base != NULL && base->volume->Device() == volume->Device());
}

BString CVolumeUsageAbsoluteDataProvider::DisplayName()
{
	return BString("Usage of ") << CVolumeDataProviderBase::DisplayName();
}

bool CVolumeUsageAbsoluteDataProvider::GetNextValue(float &value)
{
	if(volume->InitCheck() == B_OK) {
		value = (volume->Capacity() - volume->FreeBytes()) / MEGA_BYTE;
		return true;
	}
	
	return false;
}

// ==== CVolumeCapacityDataProvider ====

CVolumeCapacityDataProvider::CVolumeCapacityDataProvider(const BVolume &_volume) :
	CVolumeDataProviderBase(_volume)
{
}

CVolumeCapacityDataProvider::CVolumeCapacityDataProvider(BMessage *archive) :
	CVolumeDataProviderBase(archive)
{
}

BArchivable *CVolumeCapacityDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CVolumeCapacityDataProvider"))
		return NULL;

	return new CVolumeCapacityDataProvider(archive);
}

bool CVolumeCapacityDataProvider::Equal(IDataProvider *other)
{
	CVolumeCapacityDataProvider *base = dynamic_cast<CVolumeCapacityDataProvider *>(other);
	
	return (base != NULL && base->volume->Device() == volume->Device());
}

BString CVolumeCapacityDataProvider::DisplayName()
{
	return BString("Capacity of ") << CVolumeDataProviderBase::DisplayName();
}

bool CVolumeCapacityDataProvider::GetNextValue(float &value)
{
	if(volume->InitCheck() == B_OK) {
		value = volume->Capacity() / MEGA_BYTE;
		return true;
	}
	
	return false;
}

// ==== CVolumeUsageDataProvider ====

CVolumeUsageDataProvider::CVolumeUsageDataProvider(const BVolume &_volume) :
	CVolumeDataProviderBase(_volume)
{
}

CVolumeUsageDataProvider::CVolumeUsageDataProvider(BMessage *archive) :
	CVolumeDataProviderBase(archive)
{
}

BArchivable *CVolumeUsageDataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CVolumeUsageDataProvider"))
		return NULL;

	return new CVolumeUsageDataProvider(archive);
}

bool CVolumeUsageDataProvider::Equal(IDataProvider *other)
{
	CVolumeUsageDataProvider *base = dynamic_cast<CVolumeUsageDataProvider *>(other);
	
	return (base != NULL && base->volume->Device() == volume->Device());
}

BString CVolumeUsageDataProvider::DisplayName()
{
	return BString("Usage of ") << CVolumeDataProviderBase::DisplayName();
}

bool CVolumeUsageDataProvider::GetNextValue(float &value)
{
	if(volume->InitCheck() == B_OK) {
		off_t capacity = volume->Capacity();
		off_t usage = capacity - volume->FreeBytes();
	
		value = usage / (float)capacity;
		
		return true;
	}
	
	return false;
}


