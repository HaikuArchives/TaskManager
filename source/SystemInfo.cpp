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
#include "SystemInfo.h"

CSystemInfo::CSystemInfo()
{
	UpdateSystemInfo();
}

status_t CSystemInfo::GetSystemInfo(system_info *systemInfo, bigtime_t *_timeStamp)
{
	bigtime_t dist = system_time() - timeStamp;
	
	if(dist >= FAST_PULSE_RATE/2) {
		// cached system info is out of date
		RETURN_IF_FAILED( UpdateSystemInfo() );
	}
	
	*systemInfo = cachedSystemInfo;
	
	if(_timeStamp)
		*_timeStamp = timeStamp;
	
	return B_OK;
}

status_t CSystemInfo::UpdateSystemInfo()
{
	RETURN_IF_FAILED( get_system_info(&cachedSystemInfo) );
	timeStamp = system_time();
	
	return B_OK;
}

CSystemInfo *CSystemInfo::CreateInstance()
{
	// Initialize to quiet compiler.
	CSystemInfo *sysInfo = NULL;

	return CreateSingleton(sysInfo, "CSystemInfo");
}
