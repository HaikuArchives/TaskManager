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

#ifndef LED_VIEW_H
#define LED_VIEW_H

// ====== Includes =======

#include "PulseView.h"

// ====== Archive Fields ======

// CLedView
extern const char * const LED_VIEW_ARCHIVE_LED_SIZE;				// int32
extern const char * const LED_VIEW_ARCHIVE_LED_DIST;				// int32
extern const char * const LED_VIEW_ARCHIVE_DISTANCE;				// int32
extern const char * const LED_VIEW_ARCHIVE_MAX_VALUE;				// int32
extern const char * const LED_VIEW_ARCHIVE_LED_MAX_WIDTH;			// int32
extern const char * const LED_VIEW_ARCHIVE_LED_ON_COLOR;			// rgb_color
extern const char * const LED_VIEW_ARCHIVE_LED_OFF_COLOR;			// rgb_color
extern const char * const LED_VIEW_ARCHIVE_DATA_PROVIDER;			// CArchivableDataProvider

// ====== Scripting Properties ======

// CLedView
extern const char * const LED_VIEW_PROP_LED_ON_COLOR;				// rgb_color
extern const char * const LED_VIEW_PROP_LED_OFF_COLOR;				// rgb_color
extern const char * const LED_VIEW_PROP_LED_SIZE;					// float
extern const char * const LED_VIEW_PROP_DATA_PROVIDER;				// IDataProvider *
extern const char * const LED_VIEW_PROP_MAX_VALUE;					// float

// ====== Colors ======

extern const rgb_color DEFAULT_LED_ON_COLOR;
extern const rgb_color DEFAULT_LED_OFF_COLOR;
extern const rgb_color DEFAULT_LED_VIEW_BG;

// ====== Class Defs ======

class IDataProvider;

class _EXPORT CLedView : public CPulseView
{
	public:
	CLedView(BRect frame, const char *title, int32 _maxValue, int32 _distance, int32 _ledSize, int32 _ledMaxWidth, int32 _ledDist);
	CLedView(BMessage *archive);
	
	virtual ~CLedView();
	
	static BArchivable *Instantiate(BMessage *archive);
	
	virtual	status_t Archive(BMessage *data, bool deep = true) const;
		
	virtual void Draw(BRect updateRect);
	virtual void AttachedToWindow();
	virtual void Pulse();
	virtual void MessageReceived(BMessage *msg);
	virtual BHandler *ResolveSpecifier(BMessage *message, int32 index, 
					BMessage *specifier, int32 what, const char *property);
	virtual status_t GetSupportedSuites(BMessage *message);

	int32 MaxValue() { return maxValue; }
	void  SetMaxValue(int32 mv) { maxValue = mv; }
	void  SetDataProvider(IDataProvider *provider);
	
	protected:
	void Init();
	
	virtual bool GetNextValue(float &value);
	virtual bool GetNextString(char *string, size_t len);

	BRect TextRect();
	BRect LedRect();

	void GetTextFont(BFont *font);

	int32 ledSize;
	int32 ledDist;
	int32 distance;
	int32 ledMaxWidth;

	float value;
	int32 maxValue;

	char displayString[255];

	rgb_color ledOnColor;
	rgb_color ledOffColor;
	
	IDataProvider			*dataProvider;
};

#endif // LED_VIEW_H