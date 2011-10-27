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
 
#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H

//: Interface for data providers.
class IDataProvider
{
	public:
	IDataProvider() {}
	virtual ~IDataProvider() {}

	//: Get the next sample.
	virtual bool GetNextValue(float &value) = 0;
	//: Get the flags.
	// See enumFlags for possible values.
	virtual uint32 Flags() = 0;
	//: Get the unit.
	// See enumUnit for possible values.
	virtual uint32 Unit() = 0;
	//: Get the display name.
	virtual BString DisplayName() = 0;
	
	//: Create a clone.
	virtual IDataProvider *Clone() = 0;
	//: Test two objects for equal content.
	virtual bool Equal(IDataProvider *other) = 0;
	
	enum enumFlags {
		//: Display the value "as is".
		DP_TYPE_ABSOLUTE = 1,
		// The returned value is divided by the time between the the 
		// last two calls of GetNextValue() in milliseconds.
		DP_TYPE_RELATIVE = 2,
		//: The value is in percent.
		// The returned value is multiplyed by 100, before it's displayed
		// to the user. If the result is out of the range [0,100] it is
		// restricted to that range.
		DP_TYPE_PERCENT  = 4,
		//: Currently not supported.
		DT_TYPE_HIDDEN	 = 8,
	};

	enum enumUnit {
		//: No unit.
		DP_UNIT_NONE,
		DP_UNIT_BYTE,
		DP_UNIT_KILOBYTE,
		DP_UNIT_MEGABYTE,
		//: The value is in pages.
		// The converted to kilobytes (multiplyed by B_PAGE_SIZE/1024) 
		// before it is displayed to the user.
		DP_UNIT_PAGES,
		//: Rounds per minute.
		DP_UNIT_RPM,
		DP_UNIT_DEGREES,
		DP_UNIT_VOLT,
		DP_UNIT_WATT,
		DP_UNIT_AMPERE,
	};
};

class ICPUDataProvider : public IDataProvider
{
	public:
	ICPUDataProvider() {}
	virtual ~ICPUDataProvider() {}
	
	virtual int32 CPUNum() const = 0;
	virtual void SetCPUNum(int32 newCpu) = 0;
};

#endif // DATA_PROVIDER_H