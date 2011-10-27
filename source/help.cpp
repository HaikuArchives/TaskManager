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
#include "alert.h"
#include "TaskManager.h"
#ifndef DISABLE_LOCALIZATION
#include "LocalizationHelper.h"
#endif
#include "help.h"

int32 taskmgr_image_tag_symbol = 0;

//: Get the full application path.
BPath get_app_path()
{
	int32 cookie=0;
	image_info imageInfo;

	team_id team = be_app->Team();

	// Try to find image using the tag symbol.

	while(get_next_image_info(team, &cookie, &imageInfo) == B_OK) {
		void *location;

		if(get_image_symbol(imageInfo.id, "taskmgr_image_tag_symbol", B_SYMBOL_TYPE_DATA, &location) == B_OK) {
			// Is the TaskManager image.
			BPath imagePath(imageInfo.name);
			
			return imagePath;
		}
	}

	// If the tag symbol can't be found use the BApplication methods.

	app_info appInfo;
	status_t result = be_app->GetAppInfo(&appInfo);

	MY_ASSERT(result == B_OK);

	BEntry appEntry(&appInfo.ref);
	BPath appPath(&appEntry);

	return appPath;
}

//: Get the directory containing the application image.
BPath get_app_dir()
{
	BPath appDir;

	BPath appPath = get_app_path();

	appPath.GetParent(&appDir);

	return appDir;
}

//: Show the help file with the specified help_id.
void show_help(const char *help_id)
{
	BPath helpFile = get_app_dir();
	
	helpFile.Append("doc");
	helpFile.Append(help_id);

	BString url;
	
	url << "file://" << helpFile.Path();
	
	open_url(url.String());
}

//: Open a URL using the default browser.
void open_url(const char *url)
{
	BString appMime = get_preferred_browser();

	BMessage message(B_NETPOSITIVE_OPEN_URL);

	message.AddString("be:url", url);
	
	status_t result;
	team_id browserId/*(ignored)*/;
	
	if((result = be_roster->Launch(appMime.String(), &message, &browserId)) != B_OK &&
		result != B_ALREADY_RUNNING ) {
		#ifndef DISABLE_LOCALIZATION
		CLocalizedString err("OpenURL.ErrorMessage.OpenBrowserFailed");
		#else
		BString err("Can't open default browser\nReason: ");
		#endif
		
		err << strerror(result);
	
		#ifndef DISABLE_LOCALIZATION
		show_alert(err);
		#else
		BAlert *alert = new BAlert("OpenBrowser", err.String(), "OK");
		
		alert->Go();
		#endif
	} 
}

//: Returns the mime type of the preferred browser application.
BString get_preferred_browser()
{
	BMimeType htmlMime("text/html");
	
	char preferredApp[B_MIME_TYPE_LENGTH];
	
	htmlMime.GetPreferredApp(preferredApp);
	
	return BString(preferredApp);
}
