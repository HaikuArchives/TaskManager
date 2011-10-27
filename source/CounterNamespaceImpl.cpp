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
#include "alert.h"
#include "help.h"
#include "my_assert.h"
#include "LocalizationHelper.h"
#include "CounterNamespaceImpl.h"

// ==== globals ====

void InitGlobalNamespace()
{
	global_Namespace = new CPerformanceCounterNamespace();
}

CPointer<IPerformanceCounterNamespace> global_Namespace;

// ==== CRootNode ====

CRootNode::CRootNode(IPerformanceCounterNamespace *ns)
{
	namesp = ns;
	initChildren = true;
}

int32 CRootNode::CountChildren()
{
	InitChildren();
	return children.CountItems();
}

IPerformanceCounter *CRootNode::ChildAt(int32 i)
{
	InitChildren();
	return children.ItemAt(i);
}

void CRootNode::InitChildren()
{
	if(initChildren) {
		namesp->EnumChildren(Path(), &children);		
		initChildren = false;
	}
}

// ==== CPerformanceCounterNamespace ====

CPerformanceCounterNamespace::CPerformanceCounterNamespace()
{
	// --- load plugins

	// One plugin is the default plugin. If that plugin can't
	// be loaded the taskamanager isn't displaying anything
	// inside the "Usage" tab.
	const char *defaultPluginName = "taskmanager_default.so";
	bool defaultPluginLoaded = false;
	
	BPath  addonDirPath = get_app_dir();

	addonDirPath.Append("add_ons");
	
	BEntry addonDirEntry(addonDirPath.Path());
	BDirectory addonDir(&addonDirEntry);
	
	BEntry addonEntry;
	BPath  addonPath;

	while(addonDir.GetNextEntry(&addonEntry) == B_OK) {
		// load addon
		addonEntry.GetPath(&addonPath);
		
		CPlugin *plugin = new CPlugin(addonPath.Path());

		if(plugin->InitCheck() == B_OK) {
			pluginList.AddItem(plugin);
			
			if(strcmp(addonPath.Leaf(), defaultPluginName) == 0)
				defaultPluginLoaded = true;
		} else
			delete plugin;
	}
	
	if(!defaultPluginLoaded) {
		// default plugin couldn't be loaded. Display warning.
		CLocalizedString message("CounterNamespace.ErrorMessage.LoadDefaultFailed");
	
		message << defaultPluginName
				<< addonDirPath.Path();
			
		show_alert_with_help(message, "error_default_not_found.html");
	}
}

status_t CPerformanceCounterNamespace::EnumChildren(const char *path, BList *children)
{
	status_t status;

	for(int i=0 ; i<pluginList.CountItems() ; i++) {
		if((status = pluginList.ItemAt(i)->EnumChildren(this, path, children)) != B_OK) {
			return status;
		}
	}
	
	return B_OK;
}

IDataProvider *CPerformanceCounterNamespace::DataProvider(const char *path)
{
	// seperate components.
	CPointerList<BString> components;
	BString sPath = path;
	
	// Don't begin at index 0. Char at index 0 is always a '/'.
	for(int i=1 ; i<sPath.Length() ; i++) {
		int32 slash = sPath.FindFirst("/", i);
		
		if(slash == B_ERROR)
			slash = sPath.Length();
		
		BString *component = new BString;
		
		sPath.CopyInto(*component, i, slash-i);
		
		components.AddItem(component);
		
		i=slash;
	}
	
	CPointer<IPerformanceCounter> root = Root();
	IPerformanceCounter *current = root;
	
	for(int i=0 ; i<components.CountItems() ; i++) {
		current = FindObject(current, components.ItemAt(i)->String());
		
		if(!current) break;
	}
	
	return current ? current->DataProvider()->Clone() : NULL;
}

IPerformanceCounter *CPerformanceCounterNamespace::FindObject(IPerformanceCounter *parent, const char *name)
{
	for(int i=0 ; i<parent->CountChildren() ; i++) {
		if(strcmp(parent->ChildAt(i)->InternalName(), name) == 0) {
			return parent->ChildAt(i);
		}
	}
	
	return NULL;
}

// ==== CPerformanceCounterNamespace::CPlugin ====

CPerformanceCounterNamespace::CPlugin::CPlugin(const char *path)
{
	CLocalizedString message;
	initStatus = B_ERROR;

	addonImage = load_add_on(path);
	
	if(addonImage >= B_OK) {
		counter_plugin_entry_point entryPoint;
	
		if((initStatus = get_image_symbol(addonImage, "CreateCounterPlugin", 
			B_SYMBOL_TYPE_TEXT, (void **)&entryPoint)) == B_OK) {
			// entry point located.
			if((initStatus = entryPoint(&plugin)) != B_OK) {
				message.Load("CounterNamespace.ErrorMessage.EntryPointFailed");
			
				message << path 
						<< strerror(initStatus);
		
				show_alert(message);
			} else {
				// successfully loaded plugin.
			}
		} else {
			message.Load("CounterNamespace.ErrorMessage.EntryPointNotFound");
		
			message << path
					<< strerror(initStatus);
		
			show_alert(message);
		}
	} else {
		// Negative values are a BeOS error codes.
		initStatus = addonImage;

		message.Load("CounterNamespace.ErrorMessage.LoadFailed");

		message << path
				<< strerror(initStatus);
		
		show_alert_with_help(message, "error_load_failed.html");
	}
}

CPerformanceCounterNamespace::CPlugin::~CPlugin()
{
	if(addonImage >= 0) unload_add_on(addonImage);
}

status_t CPerformanceCounterNamespace::CPlugin::EnumChildren(IPerformanceCounterNamespace *counterNamespace, const char *path, BList *children)
{
	return plugin->EnumChildren(counterNamespace, path, children);
}
