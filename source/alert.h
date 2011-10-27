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
 
#ifndef TSKMGR_ALERT_H
#define TSKMGR_ALERT_H

#include <interface/Alert.h>
#include <app/Handler.h>

const char* ok_button_label();
const char* help_button_label();
const char* cancel_button_label();

void show_alert(const char *text, const char *title=NULL, alert_type type=B_INFO_ALERT);
void show_alert(const BString &string, const char *title=NULL, alert_type type=B_INFO_ALERT);

void show_alert_with_help(const char *text, const char *help_id, const char *title=NULL, alert_type type=B_INFO_ALERT);
void show_alert_with_help(const BString &text, const char *help_id, const char *title=NULL, alert_type type=B_INFO_ALERT);

status_t show_alert_async(const char *text, BHandler *target, const char *title=NULL, alert_type type=B_INFO_ALERT);
status_t show_alert_async(const BString &string, BHandler *target, const char *title=NULL, alert_type type=B_INFO_ALERT);

#endif // TSKMGR_ALERT_H
