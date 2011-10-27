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
#include "my_assert.h"
#include "errno.h"
#include "common.h"
#include "Plugin.h"
#include "LM78Plugin.h"

// ==== globals ====

const char * const LM78_DATA_PROVIDER_ARCHIVE_NAME		= "LM78:Name";
const char * const LM78_DATA_PROVIDER_ARCHIVE_INDEX		= "LM78:Index";

const char * const ADD_ON_SIGNATURE	= "application/x-vnd.task_manager_lm78";

// ==== entry point ====

status_t __declspec(dllexport) CreateCounterPlugin(IPerformanceCounterPlugin **plugin)
{
	*plugin = new CLM78Plugin();

	return (*plugin) != NULL ? B_OK : B_NO_MEMORY;
}

// ==== CLM87Plugin ====

CLM78Plugin::CLM78Plugin()
{
	BNode driverNode("/dev/misc/lm78");

	// driverPresent is true, even if the node isn't accessable now.
	// When the current 'owner' of the node releases it, I can access
	// the data. 
	driverPresent = driverNode.InitCheck() == B_OK ||
					driverNode.InitCheck() == B_BUSY;
}

CLM78Plugin::~CLM78Plugin()
{
}

status_t CLM78Plugin::EnumChildren(
	IPerformanceCounterNamespace *counterNamespace, 
	const char *path, 
	BList *children)
{
	if(driverPresent) {
		if(strcmp(path, "/") == 0) {
			// Root
			children->AddItem(new CPerformanceCounter(counterNamespace, path, "Sensor", NULL));
		} else if(strcmp(path, "/Sensor") == 0) {
			children->AddItem(
				new CPerformanceCounter(counterNamespace, path, "Motherboard Temperature", 
					new CLM78DataProvider("Motherboard Temperature", 6)));
			
			children->AddItem(
				new CPerformanceCounter(counterNamespace, path, "CPU Fan", 
					new CLM78DataProvider("CPU Fan", 8)));
			
			children->AddItem(
				new CPerformanceCounter(counterNamespace, path, "Power Fan", 
					new CLM78DataProvider("Power Fan", 9)));
					
			children->AddItem(
				new CPerformanceCounter(counterNamespace, path, "Chassis Fan", 
					new CLM78DataProvider("Chassis Fan", 7)));
		}
	}
	
	return B_OK;
}

// ==== CLM78DataInfo ====

CLM78DataInfo::CLM78DataInfo()
{
	// open driver
	lm78_driver = fopen("/dev/misc/lm78", "r");
		
	if(lm78_driver == NULL && errno == B_BUSY) {
		BAlert *alert = new BAlert(
			"Driver in use",
			"The LM78 driver is in use by another program.\n"
			"The sensor information is disabled until the driver " 
			"is released",
			"OK",
			NULL,
			NULL,
			B_WIDTH_AS_USUAL,
			B_WARNING_ALERT);
			
		alert->Go();	
	}

	lastUpdate = 0;
}

CLM78DataInfo::~CLM78DataInfo()
{
	// close driver
	fclose(lm78_driver);
}
	
void CLM78DataInfo::Lock()
{
	lock.Lock();
}

void CLM78DataInfo::Update()
{
	// one data block consist of 10 lines:
	// Vcore: xxxV
	// +3.3V: xxxV
	// +5.0V: xxxV
	// -5.0V: xxxV
	// +12V : xxxV
	// -12V : xxxV
	// Mainboard temperature: xxx degrees
	// Chassis fan: xxx rpm / not present
	// CPU fan: xxx rpm / not present
	// Power fan: xxx rpm / not present

	if(lm78_driver == NULL) {
		// Try to access driver. It could be that the driver was in
		// use, when this object was created, but was released aferwards.
		lm78_driver = fopen("/dev/misc/lm78", "r");
		
		if(lm78_driver == NULL) {
			// Still in use or not present.
			return;
		}
	}

	bigtime_t dist = system_time() - lastUpdate;
	
	if(dist >= FAST_PULSE_RATE/2) {
		data.MakeEmpty();
	
		for(int i=0 ; i<10 ; i++) {
			char line[255], c;
			int k=0;
		
			while((c = fgetc(lm78_driver)) != '\n') {
				line[k++] = c;
			}
		
			line[k++] = '\0';
		
			data.AddItem(new BString(line));
		}
	}
	
	lastUpdate = system_time();
}

void CLM78DataInfo::Unlock()
{
	lock.Unlock();
}
	
// ==== CLM78DataProvider ====

CPointer<CLM78DataInfo> CLM78DataProvider::dataInfo;

CLM78DataProvider::CLM78DataProvider(const char *name, int32 index)
{
	dataName = name;
	dataIndex = index;
}

CLM78DataProvider::CLM78DataProvider(BMessage *archive)
{
	dataName = archive->FindString(LM78_DATA_PROVIDER_ARCHIVE_NAME);
	dataIndex = archive->FindInt32(LM78_DATA_PROVIDER_ARCHIVE_INDEX);
}

status_t CLM78DataProvider::Archive(BMessage *archive, bool deep) const
{
	RETURN_IF_FAILED(CArchivableDataProvider::Archive(archive, deep));
	
	RETURN_IF_FAILED(archive->AddString("add_on", ADD_ON_SIGNATURE));
	
	RETURN_IF_FAILED(archive->AddString(LM78_DATA_PROVIDER_ARCHIVE_NAME, 
		dataName.String()));
	RETURN_IF_FAILED(archive->AddInt32(LM78_DATA_PROVIDER_ARCHIVE_INDEX, 
		dataIndex));

	return B_OK;
}

BArchivable *CLM78DataProvider::Instantiate(BMessage *archive)
{
	if(!validate_instantiation(archive, "CLM78DataProvider"))
		return NULL;

	return new CLM78DataProvider(archive);
}

CLM78DataProvider::~CLM78DataProvider()
{
}

bool CLM78DataProvider::GetNextValue(float &value)
{
	if(!dataInfo) {
		// The dataInfo locks the driver. Therefore I create it
		// only, if a DataProvider exists.
		dataInfo = new CLM78DataInfo();
	}

	dataInfo->Lock();
	dataInfo->Update();

	BString *p = dataInfo->Data().ItemAt(dataIndex);

	BString dataString = p ? *p : "";

	dataInfo->Unlock();
	
	if(dataString.FindFirst("not present") == B_ERROR) {
		int32 dataBegin = dataString.FindFirst(':');
		int32 dataEnd   = dataString.FindLast(' ');
		
		BString data;
		dataString.CopyInto(data, dataBegin+1, dataEnd-dataBegin);
		
		value = atof(data.String());
		
		return true;
	}
	
	return false;
}

bool CLM78DataProvider::Equal(IDataProvider *other)
{
	CLM78DataProvider *o = dynamic_cast<CLM78DataProvider *>(other);
	
	return (o && o->dataName == dataName && o->dataIndex == dataIndex);
}

uint32 CLM78DataProvider::Unit()
{ 
	switch(dataIndex) {
		case 6:
			return DP_UNIT_DEGREES;
		case 7:
		case 8:
		case 9:
			return DP_UNIT_RPM;
	}
	
	return DP_UNIT_NONE;
} 

