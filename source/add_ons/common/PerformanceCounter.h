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
 
#ifndef PERFORMANCE_COUNTER_H
#define PERFORMANCE_COUNTER_H

#include "DataProvider.h"

class IPerformanceCounter
{
	public:
	IPerformanceCounter() {}
	virtual ~IPerformanceCounter() {}
	
	virtual int32 CountChildren() = 0;
	virtual IPerformanceCounter *ChildAt(int32 i) = 0;
	virtual IDataProvider *DataProvider() = 0;
	virtual const char *Name() const = 0;
	virtual const char *Path() const = 0;
	virtual const char *InternalName() const = 0;
};

class IPerformanceCounterNamespace
{
	public:
	IPerformanceCounterNamespace() {}
	virtual ~IPerformanceCounterNamespace() {}

	virtual status_t EnumChildren(const char *path, BList *children) = 0;
	virtual IDataProvider *DataProvider(const char *path) = 0;
	virtual IPerformanceCounter *Root() = 0;
};

class IPerformanceCounterPlugin
{
	public:
	IPerformanceCounterPlugin() {}
	virtual ~IPerformanceCounterPlugin() {}

	virtual status_t EnumChildren(IPerformanceCounterNamespace *counterNamespace, const char *path, 
							BList *children) = 0;
};

typedef status_t (*counter_plugin_entry_point)(IPerformanceCounterPlugin **plugin);

#endif // PERFORMANCE_COUNTER_H