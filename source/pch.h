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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

#include <string>
#include <map>
#include <fstream>
#include <algorithm>

#include <BeBuild.h>

#ifndef B_BEOS_VERSION_DANO
// Defined for compatibility with BeOS 5.0.x
#define B_BEOS_VERSION_DANO	0x0510
#endif // !B_BEOS_VERSION_DANO

#ifndef _IMPEXP_BE
#define	_IMPEXP_BE		__declspec(dllimport)
#endif // !_IMPEXP_BE

#include <kernel/OS.h>
#include <kernel/image.h>
#include <kernel/scheduler.h>
#include <kernel/fs_attr.h>

#include <app/Application.h>
#include <app/MessageRunner.h>
#include <app/Messenger.h>
#include <app/MessageFilter.h>
#include <app/Roster.h>
#include <app/PropertyInfo.h>

#include <storage/File.h>
#include <storage/Path.h>
#include <storage/AppFileInfo.h>
#include <storage/FindDirectory.h>
#include <storage/FilePanel.h>
#include <storage/Volume.h>
#include <storage/VolumeRoster.h>
#include <storage/SymLink.h>

#include <interface/View.h>
#include <interface/Window.h>
#include <interface/Box.h>
#include <interface/TabView.h>
#include <interface/ListView.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>
#include <interface/Menu.h>
#include <interface/MenuBar.h>
#include <interface/PopUpMenu.h>
#include <interface/MenuItem.h>
#include <interface/ScrollView.h>
#include <interface/Alert.h>
#include <interface/Screen.h>
#include <interface/StringView.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>
#include <interface/PictureButton.h>
#include <interface/Region.h>
#include <interface/CheckBox.h>
#include <interface/Dragger.h>
#include <interface/OutlineListView.h>
#include <interface/MenuField.h>
#include <interface/ColorControl.h>

#include <be_apps/Deskbar/Deskbar.h>
#include <be_apps/NetPositive/NetPositive.h>

#include <support/Beep.h>
#include <support/Autolock.h>
#include <support/String.h>
#include <support/StopWatch.h>

#if B_BEOS_VERSION >= B_BEOS_VERSION_5
	#include <app/Cursor.h>
#endif // B_BEOS_VERSION_5

#include "Color.h"