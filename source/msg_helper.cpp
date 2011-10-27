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
#include "Color.h"
#include "msg_helper.h"

status_t AddColor(BMessage *msg, const char *name, rgb_color color)
{
	return msg->AddData(name, B_RGB_COLOR_TYPE, &color, sizeof(rgb_color));
}

status_t FindColor(BMessage *msg, const char *name, rgb_color &color)
{
	return FindColor(msg, name, 0, &color);
}

status_t FindColor(BMessage *msg, const char *name, rgb_color *color)
{
	return FindColor(msg, name, 0, color);
}

status_t FindColor(BMessage *msg, const char *name, int32 index, rgb_color &color)
{
	return FindColor(msg, name, index, &color);
}

status_t FindColor(BMessage *msg, const char *name, int32 index, rgb_color *color)
{
	ssize_t numBytes;
	rgb_color *data;
	
	status_t status = msg->FindData(name, B_RGB_COLOR_TYPE, index, (const void **)&data, &numBytes);

	if(status != B_OK)
		return status;

	if(numBytes != sizeof(rgb_color) || data == NULL)
		return B_ERROR;

	*color = *data;

	return B_OK;
}

rgb_color FindColor(BMessage *msg, const char *name)
{
	rgb_color color;
	
	status_t status = FindColor(msg, name, color);
	
	if(status != B_OK)
		color = CColor::Black;
	
	return color;
}

status_t AddTime(BMessage *msg, const char *name, bigtime_t time)
{
	return msg->AddData(name, B_TIME_TYPE, &time, sizeof(bigtime_t));
}

bigtime_t FindTime(BMessage *msg, const char *name)
{
	const bigtime_t *pData=NULL;
	
	ssize_t numBytes;
	
	status_t status = msg->FindData(name, B_TIME_TYPE, 0, (const void **)&pData, &numBytes);
	
	if(status != B_OK || numBytes != sizeof(bigtime_t))
		return 0;
	
	return pData ? *pData : 0;
}

status_t send_script_reply(BMessage &reply, status_t error, BMessage *message)
{
	reply.what = (error == B_OK) ? B_REPLY : B_MESSAGE_NOT_UNDERSTOOD;

	reply.AddInt32("error", error);

	if(error != B_OK) {
		reply.AddString("message", strerror(error));
	}

	BMessage reply2reply/*(ignored)*/;

	// This code crashes, if no reply to reply message is
	// supplied. Even if the receiver of this message
	// doesn't send a reply a default reply is sended,
	// when the message is destroyed, before this function
	// returns. This can happen, if the message is processed
	// by a different thread.
	return message->SendReply(&reply, &reply2reply, 0, 0);
}
