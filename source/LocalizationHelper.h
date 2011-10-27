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

#ifndef LOCALIZATION_HELPER_H
#define LOCALIZATION_HELPER_H

#include "PointerList.h"

#ifdef __INTEL__
using namespace std;
#endif

string trim(const string &s);
void replace(string &s, const string &t1, const string &t2);

class CLocalizedString : public BString
{
	public:
	CLocalizedString();
	CLocalizedString(const char *key);
	
	CLocalizedString &operator<<(const char *string);
	CLocalizedString &operator<<(const BString &string);
	CLocalizedString &operator<<(char c);
	CLocalizedString &operator<<(uint32 val);
	CLocalizedString &operator<<(int32 val);
	CLocalizedString &operator<<(uint64 val);
	CLocalizedString &operator<<(int64 val);
	CLocalizedString &operator<<(float val);
	
	void Load(const char *key);
	
	protected:
	int replaceIndex;
};

class CLocalizationHelper
{
	public:
	CLocalizationHelper(const char *language);
	
	status_t InitCheck() const { return initResult; }
	
	const char *String(const char *key) const;
	
	const char *Language() const { return lang.String(); }
	
	BPath FilePath() const;
	
	static CLocalizationHelper *GetDefaultInstance();
	
	protected:
	void Load();
	
	typedef map<string, string> map_type;
	
	map_type langMap;
	status_t initResult;
	BString lang;
	
	static CPointer<CLocalizationHelper> globalInstance;
};

#endif // LOCALIZATION_HELPER_H