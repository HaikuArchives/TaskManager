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

#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include "Singleton.h"

// this class is a 'singleton' window. It can
// only by created once. If you try to create
// a second instance the orignal window is
// brought to front.
class CAboutWindow : public CSingletonWindow
{
	public:
	static CAboutWindow *CreateInstance();

	virtual ~CAboutWindow();

	protected:
	CAboutWindow();

	friend class CSingleton;		
};

#endif // ABOUT_WINDOW_H
