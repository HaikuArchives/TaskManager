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

#ifndef MSG_HELPER_H
#define MSG_HELPER_H

#include "my_assert.h"

// BMessage helper functions.

status_t AddColor(BMessage *msg, const char *name, rgb_color color);
status_t FindColor(BMessage *msg, const char *name, rgb_color &color);
status_t FindColor(BMessage *msg, const char *name, rgb_color *color);
status_t FindColor(BMessage *msg, const char *name, int32 index, rgb_color &color);
status_t FindColor(BMessage *msg, const char *name, int32 index, rgb_color *color);
rgb_color FindColor(BMessage *msg, const char *name);

status_t AddTime(BMessage *msg, const char *name, bigtime_t time);
bigtime_t FindTime(BMessage *msg, const char *name);

status_t send_script_reply(BMessage &reply, status_t error, BMessage *message);

#endif // MSG_HELPER_H