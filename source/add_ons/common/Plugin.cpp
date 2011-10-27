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
#include "PerformanceCounter.h"
#include "Plugin.h"

// ===== CPerformanceCounter =====

CPerformanceCounter::CPerformanceCounter(
	IPerformanceCounterNamespace *counterNamespace, 
	const char *parentPath, 
	const char *name,
	IDataProvider *dataProvider)
{
	namesp = counterNamespace;

	path = parentPath;
	
	if(path[path.Length()-1] != '/')
		path.Append("/");
		
	path.Append(name);
	
	internalName = name;

	provider = dataProvider;
	
	initChildren = true;
}

CPerformanceCounter::~CPerformanceCounter()
{
	delete provider;
}

int32 CPerformanceCounter::CountChildren()
{
	InitChildren();
	return children.CountItems();
}

IPerformanceCounter *CPerformanceCounter::ChildAt(int32 i)
{
	InitChildren();
	return children.ItemAt(i);
}

IDataProvider *CPerformanceCounter::DataProvider()
{
	return provider;
}

const char *CPerformanceCounter::Path() const
{
	return path.String();
}

const char *CPerformanceCounter::InternalName() const
{
	return internalName.String();
}

const char *CPerformanceCounter::Name() const
{
	return InternalName();
}

void CPerformanceCounter::InitChildren()
{
	if(initChildren) {
		namesp->EnumChildren(path.String(), &children);

		initChildren = false;
	}
}

// ===== CArchivableDataProvider =====

status_t CArchivableDataProvider::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED( BArchivable::Archive(archive, deep) );
		
	return B_OK;
}
