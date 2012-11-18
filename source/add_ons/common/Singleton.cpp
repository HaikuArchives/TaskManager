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
#include "Singleton.h"

CPointerList<CSingleton> CSingleton::singletonList;

CSingleton::CSingleton()
{
}

CSingleton::~CSingleton()
{
}

void CSingleton::AddToList(CSingleton *singleton)
{
	singletonList.AddItem(singleton);
}

CSingleton *CSingleton::RemoveFromList(const char *className)
{
	for(int i=0 ; i<singletonList.CountItems() ; i++) {
		if(strcmp(singletonList.ItemAt(i)->ClassName(), className) == 0) {
			return singletonList.RemoveItem(i);
		}
	}
	
	return NULL;
}

CSingleton *CSingleton::Find(const char *className)
{
	for(int i=0 ; i<singletonList.CountItems() ; i++) {
		if(strcmp(singletonList.ItemAt(i)->ClassName(), className) == 0) {
			return singletonList.ItemAt(i);
		}
	}
	
	return NULL;
}
