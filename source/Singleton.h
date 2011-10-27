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

#ifndef SINGLETON_H
#define SINGLETON_H

#include "PointerList.h"

// This class is a base class for all singletons.
// A singleton is a class of which only one object can be created.
// If the user tries to create a second object, the first object is
// 'reactivated'.
// This class holds a static list of instances. Every entry in the
// list is identified by its class name. If the user creates a singleton
// it looks in this list, if an instance of this class already exists.
// If that is the case, it reactivates this instance. Otherwise it creates
// a new instance and enters it to the list. A singleton can't be created
// by its contructor. It's created with a static CreateInstance method.
// Therefore the constructor of a derived class should be defined protected.
// But the CSingleton class needs to access this constructor. So you should
// define CSingleton as a friend class of your singleton. The destructor
// of your derived class must remove the singleton from the list by
// calling RemoveFromList(ClassName()).
class CSingleton
{
	public:
	CSingleton();
	virtual ~CSingleton();

	// Must be redefined by derived classes.
	// 
	// Example:
	// CMySingleton *CMySingleton::CreateInstance()
	// {
	//     CMySingleton *dummy;
	//     return CreateSingleton(dummy, "CMySingleton");
	// }
	static CSingleton *CreateInstance()
	{
		return NULL;
	}

	// Returns the className passed to 'Create'.
	virtual const char * ClassName() { return className.String(); }
	
	// Called, if 'Create()' is called with the class name of a
	// singleton which is already created.
	virtual void Reactivate() = 0;
	
	protected:
	static void AddToList(CSingleton *singleton);
	static CSingleton *RemoveFromList(const char *className);
	static CSingleton *Find(const char *className);

	void SetClassName(const char *newClassName) { className = newClassName; }

	// This template function creates a new singleton. You should use
	// this function to implement 'CreateInstance'.
	// The first parameter simply specifies the class of the object
	// which should be created. The pointer remains untouched.
	// 'className' is a unique identifier of the singleton. It needs not
	// to be the class name.
	template<class T>
	static T *CreateSingleton(T *dummyRef, const char *className)
	{
		T *instance = dynamic_cast<T *>(Find(className));
		
		if(instance == NULL) {
			instance = new T;
		
			instance->SetClassName(className);
		
			AddToList(instance);
		} else {
			instance->Reactivate();
		}
		
		return instance;
	}
	
	static CPointerList<CSingleton> singletonList;
	BString className;
};

class CSingletonWindow : public BWindow, public CSingleton
{
	public:
	virtual void Reactivate()
	{
		BAutolock locker(this);
	
		if(IsHidden())
			Show();
		else
			Activate(true);
	}
	
	protected:
	CSingletonWindow(BRect frame, const char *title, window_type type,
		uint32 flags, uint32 workspace=B_CURRENT_WORKSPACE) :
		BWindow(frame, title, type, flags, workspace) {}
	CSingletonWindow(BRect frame, const char *title, window_look look,
		window_feel feel, uint32 flags, uint32 workspace=B_CURRENT_WORKSPACE) :
		BWindow(frame, title, look, feel, flags, workspace) {}
};

#endif // SINGLETON_H