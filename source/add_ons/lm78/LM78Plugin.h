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
 
#ifndef LM78_PLUGIN_H
#define LM78_PLUGIN_H

// Plugin for the LM78 motherboard sensor unsing the driver
// included in "Sensor2" (http://www.bebits.com/app/96/).

class CLM78Plugin : public IPerformanceCounterPlugin
{
	public:
	CLM78Plugin();
	virtual ~CLM78Plugin();
	
	status_t EnumChildren(
		IPerformanceCounterNamespace *counterNamespace, 
		const char *path, 
		BList *children);
		
	protected:
	bool driverPresent;
};

class CLM78DataInfo
{
	public:
	CLM78DataInfo();
	virtual ~CLM78DataInfo();
	
	void Lock();
	void Update();
	void Unlock();
	
	CPointerList<BString> &Data() { return data; }
	
	protected:
	BLocker lock;
	CPointerList<BString> data;
	bigtime_t lastUpdate;	

	FILE *lm78_driver;
};

class _EXPORT CLM78DataProvider : public CArchivableDataProvider
{
	public:
	CLM78DataProvider(const char *name, int32 index);
	CLM78DataProvider(BMessage *archive);
	virtual ~CLM78DataProvider();
	
	virtual status_t Archive(BMessage *archive, bool deep) const;
	static BArchivable *Instantiate(BMessage *archive);
	
	virtual bool GetNextValue(float &value);
	virtual uint32 Flags() { return DP_TYPE_ABSOLUTE; }
	virtual uint32 Unit();
	virtual BString DisplayName() { return dataName; }
	
	virtual IDataProvider *Clone() { return new CLM78DataProvider(dataName.String(), dataIndex); }
	virtual bool Equal(IDataProvider *other); 

	protected:
	static CPointer<CLM78DataInfo> dataInfo;

	BString dataName;
	int32 dataIndex;
};

extern const char * const LM78_DATA_PROVIDER_ARCHIVE_NAME;
extern const char * const LM78_DATA_PROVIDER_ARCHIVE_INDEX;

extern "C" status_t __declspec(dllexport) CreateCounterPlugin(IPerformanceCounterPlugin **plugin);

#endif // LM78_PLUGIN_H