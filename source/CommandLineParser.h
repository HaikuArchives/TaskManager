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
 
#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include "PointerList.h"

typedef CPointerList<BPath> 	CPathList;
typedef CPointerList<BString>	CStringList;

class CCommandLineParser
{
	public:
	
	static BPath	AppPathFromCmdLine(const char *cmdLine);
	static BString	Unescape(BString &str);
	static BString	Escape(const char *str);
	static bool		LaunchTeam(const char *cmdLine);
	static bool 	FillPathList(CPathList &pathList);
	static bool 	SplitCommandLine(const char *cmdLine, CStringList &components);
	static bool		GetFullAppPath(const char *currentAppPath, BPath *fullPath, BEntry *fullEntry);
};

#endif // COMMAND_LINE_PARSER_H