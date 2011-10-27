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
 
#ifndef COUNTER_NAMESPACE_IMPL_H
#define COUNTER_NAMESPACE_IMPL_H

#include "PointerList.h"
#include "PerformanceCounter.h"

extern CPointer<IPerformanceCounterNamespace> global_Namespace;

void InitGlobalNamespace();

class CRootNode : public IPerformanceCounter 
{
	public:
	CRootNode(IPerformanceCounterNamespace *ns);

	virtual int32 CountChildren();
	virtual IPerformanceCounter *ChildAt(int32 i);
	virtual IDataProvider *DataProvider() { return NULL; }
	virtual const char *Name() const { return "Root"; }
	virtual const char *Path() const { return "/"; }
	virtual const char *InternalName() const { return "Root"; }

	protected:
	virtual void InitChildren();
	
	CPointerList<IPerformanceCounter> children;
	bool initChildren;
	IPerformanceCounterNamespace *namesp;
};

class CPerformanceCounterNamespace : public IPerformanceCounterNamespace
{
	public:
	CPerformanceCounterNamespace();
	virtual ~CPerformanceCounterNamespace() {}
	
	virtual status_t EnumChildren(const char *path, BList *children);
	virtual IDataProvider *DataProvider(const char *path);
	virtual IPerformanceCounter *Root() { return new CRootNode(this); }
	
	protected:
	IPerformanceCounter *FindObject(IPerformanceCounter *parent, const char *name);
	
	class CPlugin
	{
		public:
		CPlugin(const char *path);
		virtual ~CPlugin();

		status_t InitCheck() { return initStatus; }
		status_t EnumChildren(IPerformanceCounterNamespace *counterNamespace, const char *path, BList *children);

		protected:
		status_t initStatus;
		image_id addonImage;
		IPerformanceCounterPlugin *plugin;
	};
	
	CPointerList<CPlugin> pluginList;
};

#endif // COUNTER_NAMESPACE_IMPL_H