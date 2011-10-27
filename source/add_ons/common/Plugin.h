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
 
#ifndef PLUGIN_H
#define PLUGIN_H

#include "PointerList.h"
#include "DataProvider.h"
#include "PerformanceCounter.h"

class CPerformanceCounter : public IPerformanceCounter
{
	public:
	CPerformanceCounter(IPerformanceCounterNamespace *counterNamespace, 
		const char *parentPath, 
		const char *name,
		IDataProvider *dataProvider);
	virtual ~CPerformanceCounter();

	virtual int32 CountChildren();
	virtual IPerformanceCounter *ChildAt(int32 i);
	virtual IDataProvider *DataProvider();
	virtual const char *Name() const;
	virtual const char *InternalName() const;
	virtual const char *Path() const;
	
	protected:
	virtual void InitChildren();
	
	bool initChildren;
	BString path;
	BString internalName;
	IDataProvider *provider;
	IPerformanceCounterNamespace *namesp;
	CPointerList<IPerformanceCounter> children;
};

class _EXPORT CArchivableDataProvider : public IDataProvider, public BArchivable
{
	public:
	CArchivableDataProvider() {}
	CArchivableDataProvider(BMessage *archive) : BArchivable(archive) {}

	virtual status_t Archive(BMessage *archive, bool deep) const;
};

#endif // PLUGIN_H