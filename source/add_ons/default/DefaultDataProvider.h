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
 
#ifndef DEFAULT_DATA_PROVIDER_H
#define DEFAULT_DATA_PROVIDER_H

#include "DataProvider.h"
#include "Plugin.h"

// ==== signature ====

extern const char * const ADD_ON_SIGNATURE;

// ==== archive fields ====

// C*DataProvider
extern const char * const DATA_PROVIDER_ARCHIVE_CPU_NUM;			// int32
extern const char * const DATA_PROVIDER_ARCHIVE_TEAM_ID;			// int32
extern const char * const DATA_PROVIDER_ARCHIVE_THREAD_ID;			// int32

// ==== classes ====

class _EXPORT CDefaultDataProviderBase : public CArchivableDataProvider
{
	public:
	CDefaultDataProviderBase();
	CDefaultDataProviderBase(BMessage *archive);
	
	virtual status_t Archive(BMessage *archive, bool deep) const;
};

class _EXPORT CCPUDataProvider : public ICPUDataProvider, public BArchivable
{
	public:
	CCPUDataProvider(int32 cpuNum);
	CCPUDataProvider(BMessage *archive);

	virtual status_t Archive(BMessage *archive, bool deep) const;
	static BArchivable *Instantiate(BMessage *archive);

	virtual bool GetNextValue(float &value);
	virtual uint32 Flags() { return DP_TYPE_RELATIVE | DP_TYPE_PERCENT; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }

	virtual BString DisplayName();

	virtual IDataProvider *Clone() { return new CCPUDataProvider(cpuNum); }
	virtual bool Equal(IDataProvider *other);

	virtual int32 CPUNum() const { return cpuNum; }
	virtual void SetCPUNum(int32 newCpu) { cpuNum = newCpu; }

	enum enumCPUNum {
		CPU_NUM_0=0,
		CPU_NUM_1,
		CPU_NUM_2,
		CPU_NUM_3,
		CPU_NUM_4,
		CPU_NUM_5,
		CPU_NUM_6,
		CPU_NUM_7,
		CPU_NUM_8,
		CPU_NUM_ALL=-1,
	};
	
	protected:
	void Init();
	
	bigtime_t lastActiveTime;
	int32 cpuNum;
};

class _EXPORT CMemoryDataProvider : 
	public CDefaultDataProviderBase
{
	public:
	CMemoryDataProvider();
	CMemoryDataProvider(BMessage *archive);

	// status_t Archive(BMessage *archive, bool deep) const;
	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CMemoryDataProvider(); }
	virtual BString DisplayName() { return BString("Memory Usage"); }
	virtual bool Equal(IDataProvider *other);
	
	virtual bool GetNextValue(float &value);
	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_PAGES; }
};

class _EXPORT CThreadCountDataProvider :
	public CDefaultDataProviderBase
{
	public:
	CThreadCountDataProvider();
	CThreadCountDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CThreadCountDataProvider(); }
	virtual BString DisplayName() { return BString("Thread Count"); }
	virtual bool Equal(IDataProvider *other);
	
	virtual bool GetNextValue(float &value);
	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
};

class _EXPORT CTeamCountDataProvider : 
	public CDefaultDataProviderBase
{
	public:
	CTeamCountDataProvider();
	CTeamCountDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamCountDataProvider(); }
	virtual BString DisplayName() { return BString("Team Count"); }
	virtual bool Equal(IDataProvider *other);
	
	virtual bool GetNextValue(float &value);
	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
};

class _EXPORT CTeamInfoDataProvider : 
	public CDefaultDataProviderBase
{
	public:
	CTeamInfoDataProvider(team_id team);
	CTeamInfoDataProvider(BMessage *archive);

	virtual status_t Archive(BMessage *archive, bool deep) const;

	virtual bool Equal(IDataProvider *other);
	
	protected:
	status_t GetTeamInfo(team_info *info)
	{
		return get_team_info(teamId, info);
	}
	
	team_id teamId;
};

class _EXPORT CTeamCPUDataProvider : 
	public CTeamInfoDataProvider
{
	public:
	CTeamCPUDataProvider(team_id team);
	CTeamCPUDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamCPUDataProvider(teamId); }
	virtual BString DisplayName();
	virtual bool Equal(IDataProvider *other);

	virtual uint32 Flags() { return DP_TYPE_RELATIVE | DP_TYPE_PERCENT; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
	
	virtual bool GetNextValue(float &value);
	
	protected:
	void Init();
	
	bigtime_t lastActiveTime;
};

class _EXPORT CTeamMemoryDataProvider : 
	public CTeamInfoDataProvider
{
	public:
	CTeamMemoryDataProvider(team_id team);
	CTeamMemoryDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamMemoryDataProvider(teamId); }
	virtual bool Equal(IDataProvider *other);
	virtual BString DisplayName();

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_KILOBYTE; }

	virtual bool GetNextValue(float &value);
};

class _EXPORT CTeamThreadCountDataProvider :
	public CTeamInfoDataProvider
{
	public:
	CTeamThreadCountDataProvider(team_id team);
	CTeamThreadCountDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamThreadCountDataProvider(teamId); }
	virtual BString DisplayName();
	virtual bool Equal(IDataProvider *other);

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
	
	virtual bool GetNextValue(float &value);
};

class _EXPORT CTeamAreaCountDataProvider :
	public CTeamInfoDataProvider
{
	public:
	CTeamAreaCountDataProvider(team_id team);
	CTeamAreaCountDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamAreaCountDataProvider(teamId); }
	virtual BString DisplayName();
	virtual bool Equal(IDataProvider *other);

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
	
	virtual bool GetNextValue(float &value);
};

class _EXPORT CTeamImageCountDataProvider :
	public CTeamInfoDataProvider
{
	public:
	CTeamImageCountDataProvider(team_id team);
	CTeamImageCountDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CTeamImageCountDataProvider(teamId); }
	virtual BString DisplayName();
	virtual bool Equal(IDataProvider *other);

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }
	
	virtual bool GetNextValue(float &value);
};

class _EXPORT CThreadCPUDataProvider : 
	public CDefaultDataProviderBase
{
	public:
	CThreadCPUDataProvider(thread_id thread);
	CThreadCPUDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CThreadCPUDataProvider(threadId); }
	virtual bool Equal(IDataProvider *other);
	virtual BString DisplayName();

	virtual uint32 Flags() { return DP_TYPE_RELATIVE | DP_TYPE_PERCENT; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }

	virtual bool GetNextValue(float &value);
	
	virtual status_t Archive(BMessage *archive, bool deep) const;
	
	protected:
	void Init();
	
	thread_id threadId;
	bigtime_t lastActiveTime;
};

class _EXPORT CVolumeDataProviderBase :
	public CDefaultDataProviderBase
{
	public:
	CVolumeDataProviderBase(const BVolume &_volume);
	CVolumeDataProviderBase(BMessage *archive);
	virtual ~CVolumeDataProviderBase();

	virtual status_t Archive(BMessage *archive, bool deep) const;
	virtual BString DisplayName();
	protected:
	BVolume *volume;
};

class _EXPORT CVolumeUsageAbsoluteDataProvider :
	public CVolumeDataProviderBase
{
	public:
	CVolumeUsageAbsoluteDataProvider(const BVolume &_volume);
	CVolumeUsageAbsoluteDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CVolumeUsageAbsoluteDataProvider(*volume); }
	virtual bool Equal(IDataProvider *other);
	virtual BString DisplayName();

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_MEGABYTE; }

	virtual bool GetNextValue(float &value);
};

class _EXPORT CVolumeCapacityDataProvider :
	public CVolumeDataProviderBase
{
	public:
	CVolumeCapacityDataProvider(const BVolume &_volume);
	CVolumeCapacityDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CVolumeCapacityDataProvider(*volume); }
	virtual bool Equal(IDataProvider *other);
	virtual BString DisplayName();

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit() { return DP_UNIT_MEGABYTE; }

	virtual bool GetNextValue(float &value);
};

class _EXPORT CVolumeUsageDataProvider :
	public CVolumeDataProviderBase
{
	public:
	CVolumeUsageDataProvider(const BVolume &_volume);
	CVolumeUsageDataProvider(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);

	virtual IDataProvider *Clone() { return new CVolumeUsageDataProvider(*volume); }
	virtual bool Equal(IDataProvider *other);
	virtual BString DisplayName();

	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE | DP_TYPE_PERCENT; }
	virtual uint32 Unit() { return DP_UNIT_NONE; }

	virtual bool GetNextValue(float &value);
};

#endif // DEFAULT_DATA_PROVIDER_H