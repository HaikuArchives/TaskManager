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
 
#ifndef POINTER_LIST_H
#define POINTER_LIST_H

#include <support/List.h>

//: Smart pointer.
// Automatically frees the referenced object on destruction using delete.
template<class T>
class CPointer
{
	public:
	CPointer(T *o=NULL) : p(o) {}
	virtual ~CPointer() { delete p; }
	
	operator T*() 					{ return p; }

	T *operator-> ()				{ return p; }
	T *operator = (T* o)			{ delete p; p=o; return p; }
	T &operator * ()				{ return *p; }
	const T &operator * () const	{ return *p; }
	
	protected:
	T *p;	
};

//: Smart pointer 
// Automatically frees the referenced array on destruction using delete [].
template<class T>
class CAPointer
{
	public:
	CAPointer(T *o=NULL) : p(o) {}
	virtual ~CAPointer() { delete [] p; }
	
	operator T*() 						{ return p; }

	T *operator = (T* o)				{ delete [] p; p=o; return p; }
	T &operator * ()					{ return *p; }
	T &operator [](int i)				{ return p[i]; }
	const T &operator * () const		{ return *p; }
	const T &operator [](int i)	const	{ return p[i]; }
	
	protected:
	T *p;	
};

//: BList based template class.
template<class T>
class CPointerList : public BList
{
	public:
	typedef T ** iterator;
	
	CPointerList(int32 count=20) : BList(count) {}
	virtual ~CPointerList() { MakeEmpty(); }
	
	T *ItemAt(int32 index) { return (T *)BList::ItemAt(index); }
	const T *ItemAt(int32 index) const { return (const T*)BList::ItemAt(index); }
	
	iterator Begin()   { return (iterator)Items(); }
	iterator End()	   { return Begin() + CountItems(); }
	
	bool AddItem(const T *item, int32 index) { return BList::AddItem(item, index); }
	bool AddItem(const T *item) { return BList::AddItem(item); }

	bool AddItem(T *item, int32 index) { return BList::AddItem(item, index); }
	bool AddItem(T *item) { return BList::AddItem(item); }
	
	T *RemoveItem(int32 index) { return (T *)BList::RemoveItem(index); }
	bool RemoveItem(T *item) { return BList::RemoveItem((void *)item); }
	
	void SortItems() { std::sort(Begin(), End(), CompareFkt); }

	template<class cmp>
	void SortItems(cmp comp) { std::sort(Begin(), End(), comp);  }

	void MakeEmpty()
	{
		for(int i=0 ; i<CountItems() ; i++) {
			delete ItemAt(i);
		}

		BList::MakeEmpty();	
	}

	protected:
	static bool CompareFkt(T *p1, T *p2)
	{
		return (*p1 < *p2);
	}
};

#endif // POINTER_LIST_H