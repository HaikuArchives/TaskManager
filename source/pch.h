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

#include <OS.h>
#include <image.h>
#include <scheduler.h>
#include <fs_attr.h>

#include <Application.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <MessageFilter.h>
#include <Roster.h>
#include <PropertyInfo.h>

#include <File.h>
#include <Path.h>
#include <AppFileInfo.h>
#include <FindDirectory.h>
#include <FilePanel.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <SymLink.h>

#include <View.h>
#include <Window.h>
#include <Box.h>
#include <TabView.h>
#include <ListView.h>
#include <Bitmap.h>
#include <Button.h>
#include <Menu.h>
#include <MenuBar.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <Alert.h>
#include <Screen.h>
#include <StringView.h>
#include <TextView.h>
#include <TextControl.h>
#include <PictureButton.h>
#include <Region.h>
#include <CheckBox.h>
#include <Dragger.h>
#include <OutlineListView.h>
#include <MenuField.h>
#include <ColorControl.h>

#include <Deskbar.h>
#include <NetPositive.h>

#include <Beep.h>
#include <Autolock.h>
#include <String.h>
#include <StopWatch.h>

//#if B_BEOS_VERSION >= B_BEOS_VERSION_5
	#include <Cursor.h>
//#endif // B_BEOS_VERSION_5

#include "Color.h"
