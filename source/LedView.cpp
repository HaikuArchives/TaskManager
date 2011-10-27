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
#include "signature.h"
#include "msg_helper.h"
#include "LedView.h"
#include "DataProvider.h"

#include "msg_helper.h"

// ==== globals ====

// name of archive fields
const char * const LED_VIEW_ARCHIVE_LED_SIZE		= "LEDVIEW:LedSize";
const char * const LED_VIEW_ARCHIVE_LED_DIST		= "LEDVIEW:LedDist";
const char * const LED_VIEW_ARCHIVE_DISTANCE		= "LEDVIEW:Distance";
const char * const LED_VIEW_ARCHIVE_MAX_VALUE		= "LEDVIEW:MaxValue";
const char * const LED_VIEW_ARCHIVE_LED_MAX_WIDTH	= "LEDVIEW:LedMaxWidth";
const char * const LED_VIEW_ARCHIVE_LED_ON_COLOR	= "LEDVIEW:LedOnColor";
const char * const LED_VIEW_ARCHIVE_LED_OFF_COLOR	= "LEDVIEW:LedOffColor";
const char * const LED_VIEW_ARCHIVE_DATA_PROVIDER	= "LEDVIEW:DataProvider";

// scripting properties
const char * const LED_VIEW_PROP_LED_ON_COLOR		= "LEDOnColor";
const char * const LED_VIEW_PROP_LED_OFF_COLOR		= "LEDOffColor";
const char * const LED_VIEW_PROP_LED_SIZE			= "LEDSize";
const char * const LED_VIEW_PROP_DATA_PROVIDER		= "DataProvider";
const char * const LED_VIEW_PROP_MAX_VALUE			= "MaxValue";

// colors
 const rgb_color DEFAULT_LED_ON_COLOR				= { 0, 255, 0, 255 };
 const rgb_color DEFAULT_LED_OFF_COLOR				= { 0, 128, 0, 255 };
 const rgb_color DEFAULT_LED_VIEW_BG				= { 0, 0, 0, 255 };


// ==== CLedView ====

CLedView::CLedView(BRect frame, const char *title, int32 _maxValue, int32 _distance, int32 _ledSize, int32 _ledMaxWidth, int32 _ledDist) :
	CPulseView(frame, title, B_FOLLOW_ALL_SIDES,  B_WILL_DRAW | B_PULSE_NEEDED | B_FULL_UPDATE_ON_RESIZE)
{
	ledSize     = _ledSize;		// Height of a single LED
	ledDist     = _ledDist;		// Distance between LEDs
	distance    = _distance;	// Distance from the border
	maxValue    = _maxValue;	// Maximum value returned by GetNextValue()
	ledMaxWidth = _ledMaxWidth;	// Maximum width of the LED bar
	
	// Color for LEDs that are ON.
	ledOnColor = DEFAULT_LED_ON_COLOR;

	// Color for LEDs that are OFF.
	ledOffColor = DEFAULT_LED_OFF_COLOR;
	
	// init data provider
	dataProvider = NULL;

	Init();
}

CLedView::CLedView(BMessage *archive) :
	CPulseView(archive)
{
	ledSize		= archive->FindInt32(LED_VIEW_ARCHIVE_LED_SIZE);
	ledDist		= archive->FindInt32(LED_VIEW_ARCHIVE_LED_DIST);
	distance	= archive->FindInt32(LED_VIEW_ARCHIVE_DISTANCE);
	maxValue	= archive->FindInt32(LED_VIEW_ARCHIVE_MAX_VALUE);
	ledMaxWidth	= archive->FindInt32(LED_VIEW_ARCHIVE_LED_MAX_WIDTH);
	ledOnColor	= FindColor(archive, LED_VIEW_ARCHIVE_LED_ON_COLOR);
	ledOffColor	= FindColor(archive, LED_VIEW_ARCHIVE_LED_OFF_COLOR);

	BMessage providerArchive;
	
	if(archive->FindMessage(LED_VIEW_ARCHIVE_DATA_PROVIDER, &providerArchive) == B_OK) {
		dataProvider = dynamic_cast<IDataProvider *>(instantiate_object(&providerArchive));
	}

	Init();
}

CLedView::~CLedView()
{
	delete dataProvider;
}

void CLedView::Init()
{
	// set current value to 0.
	value = 0;
	
	// init string
	displayString[0] = '\0';
}

void CLedView::SetDataProvider(IDataProvider *provider)
{
	if(dataProvider)
		delete dataProvider;
		
	dataProvider = provider;
}

status_t CLedView::Archive(BMessage *data, bool deep) const
{
	status_t status = CPulseView::Archive(data, deep);

	if(status == B_OK) {
		data->AddString("add_on", APP_SIGNATURE);
		//data->AddString("class", "CLedView");

		data->AddInt32(LED_VIEW_ARCHIVE_LED_SIZE, ledSize);
		data->AddInt32(LED_VIEW_ARCHIVE_LED_DIST, ledDist);
		data->AddInt32(LED_VIEW_ARCHIVE_DISTANCE, distance);
		data->AddInt32(LED_VIEW_ARCHIVE_MAX_VALUE, maxValue);
		data->AddInt32(LED_VIEW_ARCHIVE_LED_MAX_WIDTH, ledMaxWidth);
		data->AddData(LED_VIEW_ARCHIVE_LED_ON_COLOR, B_RGB_COLOR_TYPE, &ledOnColor, sizeof(rgb_color));
		data->AddData(LED_VIEW_ARCHIVE_LED_OFF_COLOR, B_RGB_COLOR_TYPE, &ledOffColor, sizeof(rgb_color));

		if(deep) {
			BMessage providerArchive;
		
			BArchivable *archivable = dynamic_cast<BArchivable *>(dataProvider);
		
			if(archivable && archivable->Archive(&providerArchive, deep) == B_OK)
				data->AddMessage(LED_VIEW_ARCHIVE_DATA_PROVIDER, &providerArchive);
		}
	}

	return status;
}

BArchivable *CLedView::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "CLedView"))
		return NULL;
		
	return new CLedView(archive);
}

void CLedView::AttachedToWindow()
{
	CPulseView::AttachedToWindow();

	SetViewColor(DEFAULT_LED_VIEW_BG);
}

bool CLedView::GetNextValue(float &nextValue)
{ 
	if(dataProvider && dataProvider->GetNextValue(nextValue)) {
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_RELATIVE)
			nextValue /= ReplicantPulseRate();
			
		if(dataProvider->Flags() & IDataProvider::DP_TYPE_PERCENT)
			nextValue *= 100.0;
			
		return true;
	}
	
	return false;
}

bool CLedView::GetNextString(char *string, size_t len)
{ 
	if(dataProvider) {
		switch(dataProvider->Unit()) {
			case IDataProvider::DP_UNIT_BYTE:
				sprintf(string, "%ld b", (int32)value); 
				break;
			case IDataProvider::DP_UNIT_KILOBYTE:
				sprintf(string, "%ld M", (int32)value); 
				break;
			case IDataProvider::DP_UNIT_MEGABYTE:
				sprintf(string, "%ld K", (int32)value); 
				break;
			case IDataProvider::DP_UNIT_PAGES:
				sprintf(string, "%ld K", (int32)(value*B_PAGE_SIZE)/1024); 
				break;
			case IDataProvider::DP_UNIT_NONE:
				// FALL THROUGH
			default:
				if(dataProvider->Flags() & IDataProvider::DP_TYPE_PERCENT)
					sprintf(string, "%.0f %%", MIN(MAX(value,0),100)); 
				else
					sprintf(string, "%.0f", value); 
		}
		
		return true;
	}
	
	return false;
}

void CLedView::Pulse()
{
	float nextValue;

	if(GetNextValue(nextValue) && (int32)nextValue != (int32)value) {
		BRect invalidRect;
		BRect ledRect;
		BRect oldTextRect;
		
		if(!IsHidden()) {
			// Calculate updated areas.
		
			ledRect     = LedRect();
			oldTextRect = TextRect();

			float numLeds = floor(ledRect.Height() / (ledSize+ledDist));
			float scale	  = numLeds / (float)maxValue;

			float lastNumLedsOn = floor(scale * value);
			float nextNumLedsOn = floor(scale * nextValue);

			float totalLedSize = ledSize + ledDist;

			float lastLedOnTop = (numLeds-lastNumLedsOn)*totalLedSize + ledRect.top;
			float nextLedOnTop = (numLeds-nextNumLedsOn)*totalLedSize + ledRect.top;

			invalidRect.left   = ledRect.left;
			invalidRect.right  = ledRect.right;
			invalidRect.top	   = MIN(lastLedOnTop, nextLedOnTop)-ledDist;
			invalidRect.bottom = MAX(lastLedOnTop, nextLedOnTop)+ledDist;
		}

		value = nextValue;

		GetNextString(displayString, 255);

		if(!IsHidden()) {
			// Invalidate updated areas. Don't invalidate everything
			// to aviod flickering.
			Invalidate(invalidRect);
			Invalidate(oldTextRect | TextRect());
		}
	}
}

void CLedView::Draw(BRect updateRect)
{
	BAutolock autoLocker(Window());
	
	BRect ledRect = LedRect();
	BRect clientRect = Bounds();

	float numLeds   = floor(ledRect.Height() / (ledSize+ledDist));
	float numLedsOn = floor((numLeds / (float)maxValue) * value);

	float center = clientRect.Width()/2 + clientRect.left; 
		
	for(int i=0 ; i<numLeds ; i++) {
		BRect ledRectRight, ledRectLeft;
		
		// Calculate LED recangles.
		ledRectLeft.left   = ledRect.left;
		ledRectLeft.right  = center - ledDist;
		ledRectLeft.top    = i*(ledSize + ledDist) + ledRect.top;
		ledRectLeft.bottom = ledRectLeft.top + ledSize - 1; 

		ledRectRight.left   = center + ledDist;
		ledRectRight.right  = ledRect.right;
		ledRectRight.top    = ledRectLeft.top;
		ledRectRight.bottom = ledRectLeft.bottom; 

		// Display LEDs
		if((numLeds-i) <= numLedsOn) {
			// This LED is ON.
			SetHighColor(ledOnColor);
			
			FillRect(ledRectLeft);
			FillRect(ledRectRight);
		} else {
			// This LED is OFF.
			SetHighColor(ledOffColor);
			SetLowColor(ViewColor());
		
			FillRect(ledRectLeft, B_MIXED_COLORS);		
			FillRect(ledRectRight, B_MIXED_COLORS);
		}
	}

	SetHighColor(ledOnColor);
	SetLowColor(ViewColor());
	
	BFont textFont;
	
	GetTextFont(&textFont);
	SetFont(&textFont);
	
	MovePenTo(TextRect().LeftBottom());
	DrawString(displayString);
}

BRect CLedView::TextRect()
{
	BFont font;
	
	GetTextFont(&font);
	
	BRect textRect;
	BRect clientRect = Bounds();
	
	font_height fontHeight;
	
	font.GetHeight(&fontHeight);
	
	float width  = font.StringWidth(displayString);
	float height = fontHeight.ascent + /*fontHeight.descent +*/ fontHeight.leading;

	textRect.left   = (clientRect.Width() - width) / 2 + clientRect.left;
	textRect.top    = clientRect.bottom - height - distance;

	textRect.right  = textRect.left + width;
	textRect.bottom = textRect.top + height;
	
	return textRect;
}

BRect CLedView::LedRect()
{
	BRect ledRect;
	BRect clientRect = Bounds();

	float width = MIN(ledMaxWidth, clientRect.Width() - 2*distance);
	
	ledRect.top    = distance + clientRect.top;
	ledRect.bottom = clientRect.bottom - 2*distance - TextRect().Height();
	ledRect.left   = (clientRect.Width() - width) / 2 + clientRect.left;
	ledRect.right  = ledRect.left + width;
	
	return ledRect;
}

void CLedView::GetTextFont(BFont *font)
{
	*font = be_fixed_font;

	font->SetSize(18);
}

BHandler *CLedView::ResolveSpecifier(BMessage *message, int32 index, 
	BMessage *specifier, int32 what, const char *property)
{
	switch(message->what) {
		case B_SET_PROPERTY:
			// FALL THROUGH
		case B_GET_PROPERTY:
			if(what == B_DIRECT_SPECIFIER) {
				if( strcmp(property, LED_VIEW_PROP_LED_ON_COLOR) == 0 ||
					strcmp(property, LED_VIEW_PROP_LED_OFF_COLOR) == 0 ||
					strcmp(property, LED_VIEW_PROP_LED_SIZE) == 0 ||
					strcmp(property, LED_VIEW_PROP_DATA_PROVIDER) == 0 ||
					strcmp(property, LED_VIEW_PROP_MAX_VALUE) == 0) {
					
					return this;
				}
			}
			break;
	}
	
	return CPulseView::ResolveSpecifier(message, index, specifier, what, property);
}

status_t CLedView::GetSupportedSuites(BMessage *message)
{
	static property_info prop_list[] = {
		{ 										// 1st property
			(char *)LED_VIEW_PROP_LED_ON_COLOR,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 2nd property
			(char *)LED_VIEW_PROP_LED_OFF_COLOR,	// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 3rd property
			(char *)LED_VIEW_PROP_LED_SIZE,			// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 4th property
			(char *)LED_VIEW_PROP_DATA_PROVIDER,	// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{ 										// 5th property
			(char *)LED_VIEW_PROP_MAX_VALUE,		// name
			{										// commands
				B_SET_PROPERTY,
				B_GET_PROPERTY,
				0
			},
			{ B_DIRECT_SPECIFIER, 0 },				// specifiers
			"",										// usage
			0										// extra_data
		},
		{										// terminate list
			0,
			{ 0 },
			{ 0 },
			0,
			0
		},
	};

	message->AddString("suites", "suite/vnd.Be-TM-LED-view");
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages", &prop_info);

	return CPulseView::GetSupportedSuites(message);
}

void CLedView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case B_ABOUT_REQUESTED:
			{
				show_alert_async( "TaskManager LED View (Replicant)\n\n"
								  B_UTF8_COPYRIGHT" 1999-2002 by Thomas Krammer",
								  this, "About LedView" );
			}
			break;
		case B_GET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					BMessage reply(B_REPLY);
					status_t result;
				
					if(strcmp(property, LED_VIEW_PROP_LED_ON_COLOR) == 0) {
						// GET_PROPERTY for 'LEDOnColor' property.
						result = AddColor(&reply, "result", ledOnColor);
					} else if(strcmp(property, LED_VIEW_PROP_LED_OFF_COLOR) == 0) {
						// GET_PROPERTY for 'LEDOffColor' property.
						result = AddColor(&reply, "result", ledOffColor);
					} else if(strcmp(property, LED_VIEW_PROP_LED_SIZE) == 0) {
						// GET_PROPERTY for 'LEDSize' property.
						result = reply.AddFloat("result", ledSize);
					} else if(strcmp(property, LED_VIEW_PROP_DATA_PROVIDER) == 0) {
						// GET_PROPERTY for 'DataProvider' property.
						result = reply.AddPointer("result", dataProvider);
					} else if(strcmp(property, LED_VIEW_PROP_MAX_VALUE) == 0) {
						// GET_PROPERTY for 'MaxValue' property.
						result = reply.AddFloat("result", maxValue);
					} else {
						CPulseView::MessageReceived(msg);
						return;
					}
					
					send_script_reply(reply, result, msg);
					msg->PopSpecifier();										
				}
			}
			break;
		case B_SET_PROPERTY:
			{
				int32 index;
				BMessage specifier;
				int32 what;
				const char *property;
				
				if(msg->GetCurrentSpecifier(&index, &specifier, &what, &property) == B_OK) {
					status_t result;
					BMessage reply(B_REPLY);
				
					if(strcmp(property, LED_VIEW_PROP_LED_ON_COLOR) == 0) {
						// SET_PROPERTY for 'LEDOnColor' property.
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							ledOnColor = newValue;
							Invalidate();
						}
					} else if(strcmp(property, LED_VIEW_PROP_LED_OFF_COLOR) == 0) {
						// SET_PROPERTY for 'LEDOffColor' property.
						rgb_color newValue;
						
						if((result = FindColor(msg, "data", &newValue)) == B_OK) {
							ledOffColor = newValue;
							Invalidate();
						}
					} else if(strcmp(property, LED_VIEW_PROP_LED_SIZE) == 0) {
						// SET_PROPERTY for 'LEDSize' property.
						float newValue;
						
						if((result = msg->FindFloat("data", &newValue)) == B_OK) {
							int32 oldValue = ledSize;

							ledSize = (int32)(newValue+0.5);

							if(ledSize <= 0) {
								ledSize = oldValue;
								result = B_BAD_VALUE;
							} else {
								Invalidate();
							}
						}
					} else if(strcmp(property, LED_VIEW_PROP_MAX_VALUE) == 0) {
						// SET_PROPERTY for 'MaxValue' property.
						float newValue;
						
						if((result = msg->FindFloat("data", &newValue)) == B_OK) {
							int32 oldValue = maxValue;
						
							maxValue = (int32)(newValue+0.5);
							
							if(maxValue == 0) {
								maxValue = oldValue;
								result = B_BAD_VALUE;
							} else {
								Invalidate();
							}
						}
					} else {
						CPulseView::MessageReceived(msg);
						return;
					}
					
					send_script_reply(reply, result, msg);
					msg->PopSpecifier();										
				}
			}
			break;
		default:
			CPulseView::MessageReceived(msg);
	}
}

