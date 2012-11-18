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

#ifndef TSKMGR_SYSTEM_INFO_H
#define TSKMGR_SYSTEM_INFO_H

#include "Singleton.h"

class CSystemInfo : public CSingleton
{
	public:
	static CSystemInfo *CreateInstance();

	status_t GetSystemInfo(system_info *systemInfo, bigtime_t *_timeStamp);
	virtual void Reactivate() {}
	
	protected:
	CSystemInfo();
	
	status_t UpdateSystemInfo();

	system_info cachedSystemInfo;
	bigtime_t timeStamp;

	friend class CSingleton;		
};

inline status_t get_cached_system_info(system_info *systemInfo, bigtime_t *timeStamp)
{
	CSystemInfo *instance = CSystemInfo::CreateInstance();
	
	return instance->GetSystemInfo(systemInfo, timeStamp);
}

#endif // TSKMGR_SYSTEM_INFO_H
