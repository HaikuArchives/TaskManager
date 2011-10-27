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
#include "TaskManagerPrefs.h"

#include "LocalizationHelper.h"

// ===== globals ======

string trim(const string &s)
{
	string::size_type i1, i2;
	
	const char *whitespaces = " \t";
	
	i1 = s.find_first_not_of(whitespaces);

	if(i1 == string::npos)
		return string("");
	
	i2 = s.find_last_not_of(whitespaces);
	
	return s.substr(i1, i2+1);
}

void replace(string &s, const string &t1, const string &t2)
{
	string::size_type i=0;

	while((i=s.find(t1, i)) != string::npos) {
		s.replace(i, t1.length(), t2);
		i+=t2.length();
	}
}

// ====== CLocalizedString ====

CLocalizedString::CLocalizedString()
{
	replaceIndex = 0;
}

CLocalizedString::CLocalizedString(const char *key) :
	BString(CLocalizationHelper::GetDefaultInstance()->String(key))
{
	replaceIndex = 0;
}

CLocalizedString &CLocalizedString::operator<<(const char *string)
{
	char replaceString[20];
	sprintf(replaceString, "\\%d", replaceIndex++);
	ReplaceFirst(replaceString, string);
	return *this;
}

CLocalizedString &CLocalizedString::operator<<(const BString &string)
{
	return operator<<(string.String());
}

CLocalizedString &CLocalizedString::operator<<(char c)
{
	BString s;
	s << c;
	return operator<<(s.String());
}

CLocalizedString &CLocalizedString::operator<<(uint32 val)
{
	BString s;
	s << val;
	return operator<<(s.String());
}

CLocalizedString &CLocalizedString::operator<<(int32 val)
{
	BString s;
	s << val;
	return operator<<(s.String());
}

CLocalizedString &CLocalizedString::operator<<(uint64 val)
{
	BString s;
	s << val;
	return operator<<(s.String());
}

CLocalizedString &CLocalizedString::operator<<(int64 val)
{
	BString s;
	s << val;
	return operator<<(s.String());
}

CLocalizedString &CLocalizedString::operator<<(float val)
{
	BString s;
	s << val;
	return operator<<(s.String());
}

void CLocalizedString::Load(const char *key)
{
	SetTo(CLocalizationHelper::GetDefaultInstance()->String(key));
}

// ====== CLocalizationHelper =====

CPointer<CLocalizationHelper> CLocalizationHelper::globalInstance;

//: Initializes the a instance from a language file.
// The name of the language file must be identical to the string passed
// to this constructor. The language files are located in the "language"
// sub-directory of the application's install directory.
// Call InitCheck to check if the initialization was successful.
CLocalizationHelper::CLocalizationHelper(const char *language)
{
	lang = language;
	Load();
}

//: Get the file from which the strings in this instance where loaded.
BPath CLocalizationHelper::FilePath() const
{
	BPath langFile = get_app_dir();
	
	langFile.Append("language");
	langFile.Append(lang.String());
	
	return langFile;
}

//: Loads a language file and stores the data in this instance.
void CLocalizationHelper::Load()
{
	BPath langFile = FilePath();

	ifstream file(langFile.Path());

	if(file.good()) {
		string line;

		while(!file.eof()) {
			char l[1024];
			file.getline(l, sizeof(l));
			
			line = l;
			
			string::size_type index = line.find_first_of("=");
			
			if(index != string::npos) {
				string key		= line.substr(0, index);
				string value	= line.substr(index+1);

				key		= trim(key);
				value	= trim(value);

				replace(value, "\\n",	"\n");
				replace(value, "\\t",	"\t");
				replace(value, "\\\\",	"\\");

				langMap[key] = value;
			}
		}
		
		initResult = B_OK;
	} else {
		initResult = B_ENTRY_NOT_FOUND;
	}
}


//: Get a localized string.
// Returns an empty string if the specified key can't be found.
const char *CLocalizationHelper::String(const char *key) const
{
	map_type::const_iterator i = langMap.find(key);
	
	if(i != langMap.end()) {
		return (*i).second.c_str();
	} else {
		return "";
	}
}

//: Get the default instance of this class.
// The class will be initialized with the language file the user
// specified during setup.
CLocalizationHelper *CLocalizationHelper::GetDefaultInstance()
{
	if(globalInstance == NULL) {
		// Language the user specified during setup.
		BString language = CTaskManagerPrefs().Language();
	
		// Try to load language file.
		globalInstance = new CLocalizationHelper(language.String());
	
		if(globalInstance->InitCheck() != B_OK) {
			// Loading of language file failed.
		
			BString message;
	
			message << "Can't find language file for " << globalInstance->Language()
					<< "\nExpected path was: " << globalInstance->FilePath().Path();
	
			// NOTE: show_alert handles the case when no language file
			// is loaded and still displays a correct OK button text.
			show_alert(message);

			// Try to load default language file.			
			globalInstance = new CLocalizationHelper("English");
			
			// If that fails as well there is not much I can do.
			// The GUI will display only empty strings in that
			// case.
		}
	}
	
	return globalInstance;
}