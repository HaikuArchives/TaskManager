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
#include "help.h"
#include "alert.h"
#include "DialogBase.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "alert"

const char* ok_button_label()
{
	return B_TRANSLATE("Ok");
}

const char* help_button_label()
{
	return B_TRANSLATE("Help");
}

const char* cancel_button_label()
{
	return B_TRANSLATE("Cancel");
}

void show_alert(const char *text, const char *title, alert_type type)
{
	BAlert *alert=new BAlert(title ? title : "alert", text, ok_button_label(), NULL, NULL, B_WIDTH_FROM_WIDEST, type);

	alert->Go();
}

void show_alert(const BString &string, const char *title, alert_type type)
{
	show_alert(string.String(), title, type);
}

void show_alert_with_help(const char *text, const char *help_id, const char *title, alert_type type)
{
	BAlert *alert=new BAlert(title ? title : "alert", text, help_button_label(), ok_button_label(), NULL, B_WIDTH_FROM_WIDEST, type);
	
	if(alert->Go() == 0) {
		show_help(help_id);
	}
}

void show_alert_with_help(const BString &text, const char *help_id, const char *title, alert_type type)
{
	show_alert_with_help(text.String(), help_id, title, type);
}

status_t show_alert_async(const char *text, BHandler *target, const char *title, alert_type type)
{
	BInvoker *alertInvoker = NULL;
	
	if(target)
		alertInvoker = new BInvoker(new BMessage(MSG_OK), target);

	BAlert *alert=new BAlert(title ? title : "alert", text, ok_button_label(), NULL, NULL, B_WIDTH_FROM_WIDEST, type);

	return alert->Go(alertInvoker);
}

status_t show_alert_async(const BString &string, BHandler *target, const char *title, alert_type type)
{
	return show_alert_async(string.String(), target, title, type);
}
