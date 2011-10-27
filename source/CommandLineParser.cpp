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
#include "alert.h"
#include "LocalizationHelper.h"
#include "CommandLineParser.h"

#include "my_assert.h"

// LaunchTeam
// Launches a new team with the command line 'cmdLine'. If the first token of the
// command line is an executable this executable is started. If the executable
// isn't specified as full path this function tries to find it in the directories
// defined in $PATH. If the first part of the comammand line is a simple datafile
// the default application for that type of files is started.
// If any error occurs this function displays an alert with an descriptive string
// and returns false.
// The command line must be specified in the same format as in the shell.
//
// Example:
// 		'/salmon/BeOS\ Exchange/SomeApp "aaa bbb" "aaa\bbb" aaa\\bbb'
//    starts the application
//		'/salmon/BeOS Exchange/SomeApp' with the parameters
//		'aaa bbb'
//		'aaa\bbb'
//		'aaa\bbb'
bool CCommandLineParser::LaunchTeam(const char *cmdLine)
{
	CStringList parsedCmdLine;
	
	// Split and unescape command line.
	if(!SplitCommandLine(cmdLine, parsedCmdLine) || parsedCmdLine.CountItems() < 1) {
		// empty command line
		return false;
	}

	BPath	appPath;
	BEntry	appEntry;

	// try to find application file. (in $PATH if no directory is specified)
	if(!GetFullAppPath(parsedCmdLine.ItemAt(0)->String(), &appPath, &appEntry)) {
		// file not found.
		CLocalizedString message("CommandLineParser.ErrorMessage.FileNotFound");
		
		message << parsedCmdLine.ItemAt(0)->String();

		show_alert(message);
		
		return false;
	}

	CAPointer<char*>	argv = NULL;
	int32				argc = 0;

	// copy parameters (pointers) into argv array
	if(parsedCmdLine.CountItems() >= 2) {
		argc = parsedCmdLine.CountItems()-1;
	
		argv = new char * [ argc ];
		
		for(int32 i=0 ; i<argc ; i++) {
			argv[i] = (char *)parsedCmdLine.ItemAt(i+1)->String();
		}
	}
	
	entry_ref appEntryRef;
	
	appEntry.GetRef(&appEntryRef);
	
	BVolume volume(appEntryRef.device);

	if(volume.InitCheck() != B_OK || !volume.IsPersistent()) {
		// BRoster::Lauch crashes, if I pass a entry_ref to an entry
		// in a virtual file system.
		// This bug seems to be removed in R5.
		show_alert(CLocalizationHelper::GetDefaultInstance()->String("CommandLineParser.ErrorMessage.FileOnVFS"));
		
		return false;
	}

	status_t result;

	// start application
	if((result = be_roster->Launch(&appEntryRef, argc, argv, NULL)) != B_OK) {
		if(result != B_ALREADY_RUNNING) {
			CLocalizedString message("CommandLineParser.ErrorMessage.LaunchFailed");
		
			message << strerror(result);
		
			show_alert(message);
			
			return false;
		}
	}
	
	return true;
}

// Unescape
// Replaces '\ ' by ' ' and '\\' by '\'
BString CCommandLineParser::Unescape(BString &str)
{
	BString result;

	bool escaped = false;

	for(int32 i=0 ; i<str.Length() ; i++) {
		switch(str[i]) {
			case '\\':
				if(escaped) {
					result.Append(str[i], 1);
					escaped = false;
				} else {
					escaped=true;
				}
				
				break;
			case ' ':
				if(escaped) {
					result.Append(str[i], 1);
				} else {
					// new string starts here.
					// should never happen.
				}
				
				break;
			default:
				escaped = false;
				result.Append(str[i], 1);
				break;
		}
	}
	
	return result;
}

// Escape
// Replaces ' ' by '\ ' and '\' by '\\'
BString CCommandLineParser::Escape(const char *str)
{
	int32 len=strlen(str);
	BString result;

	for(int32 i=0 ; i<len ; i++) {
		switch(str[i]) {
			case ' ':
				result.Append("\\ ");
				break;
			case '\\':
				result.Append("\\\\");
				break;
			default:
				result.Append(str[i], 1);
				break;
		}
	}
	
	return result;
}

// AppPathFromCmdLine
// Returns the full path of the first token in the command line.
// If any error occurs an uninitalized BPath object is returned.
BPath CCommandLineParser::AppPathFromCmdLine(const char *cmdLine)
{
	BPath result;
	CStringList parsedCmdLine;
	
	if(SplitCommandLine(cmdLine, parsedCmdLine) && parsedCmdLine.CountItems() >= 1) {
		GetFullAppPath(parsedCmdLine.ItemAt(0)->String(), &result, NULL);
	}
	
	return result;
}

// GetFullAppPath
// Returns the full path (path and entry) of an application. If the
// path in 'currentAppPath' is already a full path this path is returned.
// Otherwise this functions tries to find an application with the name
// passed in 'currentAppPath' in any directory in $PATH.
// Either 'fullPath' or 'fullEntry' can be NULL.
bool CCommandLineParser::GetFullAppPath(const char *currentAppPath, BPath *fullPath, BEntry *fullEntry)
{
	BPath	appPath(currentAppPath);
	BEntry	appEntry(appPath.Path(), true);

	if(appEntry.InitCheck() != B_OK || !appEntry.Exists()) {
		bool found = false;
	
		bool leafOnly = (strchr(currentAppPath, '/') == NULL);
	
		CPathList pathList;
	
		// look for file in $PATH
		if(leafOnly && CCommandLineParser::FillPathList(pathList)) {
			for(int32 i=0 ; i<pathList.CountItems() ; i++) {
				BPath file(pathList.ItemAt(i)->Path(), appPath.Leaf());
				
				BEntry testEntry(file.Path(), true);
				
				if(testEntry.InitCheck() == B_OK && testEntry.Exists()) {
					appEntry = testEntry;
				
					found = true;
					
					break;
				}	
			}
		} 

		if(!found) {
			return false;
		}
	}

	if(fullPath) {
		// Don't set 'fullPath' to 'appPath' here. Because when the
		// testEntry traversed (resolved a symlink) the entry
		// may point to a different file.
		appEntry.GetPath(fullPath);
	}
	
	if(fullEntry){
		*fullEntry = appEntry;
	}
	
	return true;
}

// SplitCommandLine
// Splits the tokens of a command line and stores them in 'components'.
// The returned components are unescaped.
bool CCommandLineParser::SplitCommandLine(const char *cmdLine, CStringList &components)
{
	int32 len=strlen(cmdLine);

	bool	quoted=false, 		// this range of chars is quoted (started by '"');
			escaped=false, 		// this char is escaped (preceeded by '\')
			last_space=false;	// last char was a space

	BString currentComponent;

	for(int32 i=0 ; i<len ; i++) {
		switch(cmdLine[i]) {
			case '"':
				quoted = !quoted;
				break;
			case '\\':
				if(escaped || quoted) {
					currentComponent.Append(cmdLine[i], 1);
					escaped = false;
				} else {
					escaped = true;
				}
				break;
			case ' ':
				if(escaped || quoted) {
					currentComponent.Append(cmdLine[i], 1);
					escaped = false;
				} else {
					if(!last_space) {
						components.AddItem(new BString(currentComponent));
						currentComponent.SetTo("");
					}
						
					last_space = true;
				}
				
				break;
			default:
				if(escaped) {
					// ignore escape of normal char
					escaped = false;
				}
			
				currentComponent.Append(cmdLine[i], 1);
				
				last_space = false;
				
				break;
		}
	}

	if(currentComponent != "")
		components.AddItem(new BString(currentComponent));

	return true;
}

// FillPathList
// Parses the $PATH environment variable and stores all directories specified
// in $PATH in a path list.
bool CCommandLineParser::FillPathList(CPathList &pathList)
{
	char *path = getenv("PATH");

	if(path == NULL)
		return false;

	int32 start=0, len=strlen(path);
	
	if(len == 1) {
		pathList.AddItem(new BPath(path));
	} else {
		for(int32 i=0 ; i<=len ; i++) {
			if(i==len || path[i] == ':') {
				if(i-start > 0) {
					BString component;
					component.SetTo(&path[start], i-start);

					pathList.AddItem(new BPath(component.String()));
				}
				
				start = i+1;
			}
		}
	}

	return true;	
}
