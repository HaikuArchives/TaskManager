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
#include "Preferences.h"

// ====== CPreferences ======

bool CPreferences::Read(const char *name, char *buffer, size_t buffer_size)
{
	return ReadRaw(name, B_STRING_TYPE, buffer, buffer_size);
}

bool CPreferences::Read(const char *name, void *array, size_t elementSize, int32 elements)
{
	return ReadRaw(name, B_RAW_TYPE, array, elementSize*elements);
}

void CPreferences::Read(const char *name, int32 &value, int32 defaultValue)
{
	if(!ReadRaw(name, B_INT32_TYPE, &value, sizeof(int32)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, uchar &value, uchar defaultValue)
{
	if(!ReadRaw(name, B_CHAR_TYPE, &value, sizeof(uchar)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, float &value, float defaultValue)
{
	if(!ReadRaw(name, B_FLOAT_TYPE, &value, sizeof(float)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, bool &value, bool defaultValue)
{
	if(!ReadRaw(name, B_BOOL_TYPE, &value, sizeof(bool)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, BRect &value, const BRect &defaultValue)
{
	if(!ReadRaw(name, B_RECT_TYPE, &value, sizeof(BRect)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, bigtime_t &value, bigtime_t defaultValue)
{
	if(!ReadRaw(name, B_TIME_TYPE, &value, sizeof(bigtime_t)))
		value = defaultValue;
}

void CPreferences::Read(const char *name, BString &value, const BString &defaultValue)
{
	char buffer[4096];
	
	if(!ReadRaw(name, B_STRING_TYPE, buffer, sizeof(buffer))) {
		value = defaultValue;
	} else {
		value = buffer;
	}
}

void CPreferences::Read(const char *name, rgb_color &value, const rgb_color &defaultValue)
{
	if(!ReadRaw(name, B_RGB_COLOR_TYPE, &value, sizeof(rgb_color)))
		value = defaultValue;
}



bool CPreferences::Write(const char *name, const char *buffer, int32 length)
{
	if(length == -1) {
		// write string and terminating zero.
		length = strlen(buffer)+1;
	}

	return WriteRaw(name, B_STRING_TYPE, (void *)buffer, length);
}

bool CPreferences::Write(const char *name, const BString &value)
{
	return Write(name, value.String(), value.Length()+1);
}

bool CPreferences::Write(const char *name, void *array, size_t elementSize, int32 elements)
{
	return WriteRaw(name, B_RAW_TYPE, array, elementSize*elements);
}

bool CPreferences::Write(const char *name, int32 value)
{
	return WriteRaw(name, B_INT32_TYPE, &value, sizeof(int32));
}

bool CPreferences::Write(const char *name, uchar value)
{
	return WriteRaw(name, B_CHAR_TYPE, &value, sizeof(uchar));
}

bool CPreferences::Write(const char *name, float value)
{
	return WriteRaw(name, B_FLOAT_TYPE, &value, sizeof(float));
}

bool CPreferences::Write(const char *name, bool value)
{
	return WriteRaw(name, B_BOOL_TYPE, &value, sizeof(bool));
}

bool CPreferences::Write(const char *name, BRect value)
{
	return WriteRaw(name, B_RECT_TYPE, &value, sizeof(BRect));
}

bool CPreferences::Write(const char *name, bigtime_t value)
{
	return WriteRaw(name, B_TIME_TYPE, &value, sizeof(bigtime_t));
}

bool CPreferences::Write(const char *name, rgb_color value)
{
	return WriteRaw(name, B_RGB_COLOR_TYPE, &value, sizeof(rgb_color));
}

// ====== CFilePreferences ======

CFilePreferences::CFilePreferences(const char *settingsFile)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	
	path.Append(settingsFile);
	
	file = new BFile(path.Path(), B_READ_WRITE | B_CREATE_FILE);
}

CFilePreferences::~CFilePreferences()
{
	delete file;
}

off_t CFilePreferences::DataSize(const char *name)
{
	attr_info info;
	
	memset(&info, 0, sizeof(attr_info));
	
	file->GetAttrInfo(name, &info);
	
	return info.size;
}

bool CFilePreferences::ReadRaw(const char *name, type_code type, void *data, size_t length)
{
	return file->ReadAttr(name, type, 0, data, length) >= B_OK;
}

bool CFilePreferences::WriteRaw(const char *name, type_code type, void *data, size_t length)
{
	// Sometimes the size of the attribute isn't correctly resetted, 
	// when the attribute is written.
	// Therefor I remove the attribute before I write it. So I'm sure,
	// that the size is correct. (yet another BeOS bug???).
	// NOTE: This seems to be corrected in BeOS R5.
	file->RemoveAttr(name);

	return file->WriteAttr(name, type, 0, data, length) >= B_OK;
}

bool CFilePreferences::IsSpecified(const char *name)
{
	attr_info info;

	return (file->GetAttrInfo(name, &info) == B_OK);
}
