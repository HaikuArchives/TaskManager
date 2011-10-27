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

#ifndef TM_HTML_HELP_H
#define TM_HTML_HELP_H

// This symbol is used to identify the TaskManager image.
// Using a tag symbol is necessary to find the TaskManager
// image when it is loaded as addon by a different application.
// GetAppInfo() would return the image of the host application
// in that case.
extern "C" __declspec(dllexport) int32 taskmgr_image_tag_symbol;

// Returns the application signature of the preferred browser.
BString get_preferred_browser();

void open_url(const char *url);

void show_help(const char *help_id);

BPath get_app_dir(); 
BPath get_app_path();

#endif // TM_HTML_HELP_H