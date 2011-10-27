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

#ifndef PREFERENCES_H
#define PREFERENCES_H

class CPreferences
{
	public:
	CPreferences() {}
	virtual ~CPreferences() {}
	
	bool Read(const char *name, char *buffer, size_t buffer_size);
	bool Read(const char *name, void *array, size_t elementSize, int32 elements);
	void Read(const char *name, int32 &value, int32 defaultValue=0);
	void Read(const char *name, uchar &value, uchar defaultValue=0);
	void Read(const char *name, float &value, float defaultValue=0.0);
	void Read(const char *name, bool &value, bool defaultValue=false);
	void Read(const char *name, BRect &value, const BRect &defaultValue=BRect(-1,-1,-1,-1));
	void Read(const char *name, bigtime_t &value, const bigtime_t defaultValue=0);

	bool Write(const char *name, const char *buffer, size_t buffer_size);
	bool Write(const char *name, void *array, size_t elementSize, int32 elements);
	bool Write(const char *name, int32 value);
	bool Write(const char *name, uchar value);
	bool Write(const char *name, float value);
	bool Write(const char *name, bool value);
	bool Write(const char *name, BRect value);
	bool Write(const char *name, bigtime_t value);
	
	protected:
	virtual bool ReadRaw(const char *name, type_code type, void *data, size_t length) = 0;
	virtual bool WriteRaw(const char *name, type_code type, void *data, size_t length) = 0;
	virtual off_t DataSize(const char *name) = 0;
};

class CFilePreferences : public CPreferences
{
	public:
	CFilePreferences(const char *settingsFile);
	virtual ~CFilePreferences();
	
	protected:
	virtual bool ReadRaw(const char *name, type_code type, void *data, size_t length);
	virtual bool WriteRaw(const char *name, type_code type, void *data, size_t length);
	virtual off_t DataSize(const char *name);

	BFile *file;
};

#endif // PREFERENCES_H