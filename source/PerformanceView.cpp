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
#include "common.h"
#include "my_assert.h"
#include "msg_helper.h"
#include "GraphView.h"
#include "BugfixedDragger.h"
#include "MenuField.h"
#include "FlickerFreeButton.h"
#include "LocalizationHelper.h"
#include "TaskManagerPrefs.h"
#include "ListViewEx.h"
#include "ColorSelectListItem.h"
#include "ColorSelectMenuItem.h"
#include "ColorSelectDialog.h"
#include "PerformanceCounter.h"
#include "BorderView.h"
#include "PerformanceView.h"
#include "CounterNamespaceImpl.h"

// ==== globals ====

// message fields for MSG_SELECT_DATA_PROVIDER
const char * const MESSAGE_DATA_ID_PERF_COUNTER_PATH  = "SELECTDP:PerfCounterPath";
const char * const MESSAGE_DATA_ID_EXPAND_FOUND		  = "SELECTDP:ExpandFound";

// localization keys
const char * const LOC_KEY_ADD_BUTTON_LABEL			  = "PerformanceAddView.AddButton.Label";
const char * const LOC_KEY_CHANGE_BUTTON_LABEL		  = "PerformanceAddView.ChangeButton.Label";
const char * const LOC_KEY_DONE_BUTTON_LABEL		  = "PerformanceAddView.DoneButton.Label";
const char * const LOC_KEY_COLOR_SELECTOR_LABEL		  = "PerformanceAddView.ColorSelector.Label";
const char * const LOC_KEY_SCALE_SELECTOR_LABEL		  = "PerformanceAddView.ScaleSelector.Label";
const char * const LOC_KEY_COLOR_AUTOMATIC			  = "PerformanceAddView.ColorList.Automatic.Label";
const char * const LOC_KEY_COLOR_SELECT				  = "PerformanceAddView.ColorList.Select.Label";

// ==== CPerformanceAddView ====

// static data
const float CPerformanceAddView::dist = 10.0;
const int32 CPerformanceAddView::MAX_USERDEFINED_COLORS = 5;

CPerformanceAddView::CPerformanceAddView(BRect frame, BHandler *handler) :
	CLocalizedDialogBase(frame,
		"AddDialog",
		B_FOLLOW_ALL_SIDES,
		0,
		CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_CHANGE_BUTTON_LABEL),
		CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_DONE_BUTTON_LABEL),
		true),
	BInvoker(NULL, handler)
{
	// init members

	selColor = CColor::Black;
	autoSelectColor = true;
	selScale = 1.0;
	
	SetMessage(new BMessage(MSG_ADD_DATA_PROVIDER));
	
	treeView = NULL;
	colorSelect = scaleSelect = NULL;
	
	SetHelpID("tabs.html#perf_tab_add_dialog");
	
	CreateColorMenuField();
	CreateScaleMenuField();
}

CPerformanceAddView::~CPerformanceAddView()
{
}

// create color select menu field
void CPerformanceAddView::CreateColorMenuField()
{
	struct color_message_info
	{
		rgb_color color;
		const char *key;	// The full key is: PerformanceAddView.ColorList.<key>.Label
	};

	static const color_message_info colorList[] =
	{
		{ { 255,   0,   0, 255 },	"Red"			},
		{ {   0, 255,   0, 255 },   "Green"			},
		{ {   0,   0, 255, 255 },   "Blue"			},
		{ { 255,   0, 255, 255 },  	"Magenta"		},
		{ {   0, 255, 255, 255 },	"Cyan"			},
		{ { 255, 255,   0, 255 },	"Yellow"		},
		{ { 160,  20,  20, 255 },	"DarkRed"		},
		{ {  20, 160,  20, 255 },	"DarkGreen"		},
		{ {  20,  20, 180, 255 },	"DarkBlue"		},
		{ { 240,  90,  90, 255 },	"LightRed"		},
		{ { 120, 240, 120, 255 },	"LightGreen"	},
		{ {  64, 162, 255, 255 },	"LightBlue"		},
		{ { 154, 110,  45, 255 },	"Brown"			},
		{ {   0,   0,   0, 255 },	""				} 	// terminate list
	};

	LoadUserdefinedColors();

	BMenu *colorMenu = new BPopUpMenu("Color");

	BMenuItem *automaticItem = 
		new BMenuItem(
			CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_COLOR_AUTOMATIC),
			new BMessage(MSG_COLOR_SELECTED));
	automaticItem->SetMarked(true);
	
	colorMenu->AddItem(automaticItem);
	colorMenu->AddItem(
		new BMenuItem(
			CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_COLOR_SELECT),
			new BMessage(MSG_VIEW_COLOR_SELECTOR)));
	colorMenu->AddSeparatorItem();
	
	for(const color_message_info * messageInfo = colorList ; 
		messageInfo->key[0] ; ++messageInfo) {
		BString colorKey;

		colorKey << "PerformanceAddView.ColorList." << messageInfo->key << ".Label";
		const char *colorName = CLocalizationHelper::GetDefaultInstance()->String(colorKey.String());
		
		BMenuItem *item = CreateColorItem(messageInfo->color, colorName);

		colorMenu->AddItem(item);
	}

	for(int i=0 ; i<userDefinedColors.CountItems() ; i++) {
		BMenuItem *item = CreateColorItem(*userDefinedColors.ItemAt(i));
		
		colorMenu->AddItem(item);
	}
	
	colorMenu->SetTargetForItems(this);
	
	colorSelect = create_menu_field(
						B_ORIGIN,
						"ColorSelector",
						CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_COLOR_SELECTOR_LABEL),
						colorMenu,
						B_FOLLOW_LEFT | B_FOLLOW_BOTTOM
						/*B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS*/);
}

// create scale select field
void CPerformanceAddView::CreateScaleMenuField()
{
	BMenu *scaleMenu = new BPopUpMenu("Scale");
	
	for(int i=-3 ; i<=3 ; i++) {
		char name[70];
		float scale = pow(10, i);
		
		int prec = (i<0) ? -i : 0;
		
		sprintf(name, "%.*f", prec, scale);
	
		BMessage *message = new BMessage(MSG_SCALE_SELECTED);
		message->AddFloat(MESSAGE_DATA_ID_SCALE, scale);
	
		BMenuItem *menuItem = new BMenuItem(name, message);	
		
		// Label for scale '1' is default item.
		menuItem->SetMarked(i == 0);
	
		scaleMenu->AddItem(menuItem);
	}

	scaleSelect = create_menu_field(
						B_ORIGIN, 
						"ScaleSelector",
						CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_SCALE_SELECTOR_LABEL),
						scaleMenu, 
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	
	/*
	new BMenuField(BRect(colorSelectRight+dist, 0, bounds.right-dist, 10),
						"ScaleSelector",
						CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_SCALE_SELECTOR_LABEL),
						scaleMenu, 
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM,
						B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
	*/
}

void CPerformanceAddView::AttachedToWindow()
{
	CLocalizedDialogBase::AttachedToWindow();

	SetViewColor(CColor::BeBackgroundGray);

	BRect bounds = Bounds();

	SetOkButtonLabel(CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_ADD_BUTTON_LABEL));
	EnableOkButton(false);

	float buttonHeight = okButton->Bounds().Height();

	// initialized color select menu field

	float colorSelectRight = (bounds.Width() - dist) / 2;

	colorSelect->Menu()->SetTargetForItems(this);

	float colorSelectWidth, colorSelectHeight;

	colorSelect->GetPreferredSize(&colorSelectWidth, &colorSelectHeight);
	colorSelect->MoveTo(dist, bounds.bottom-4*dist-buttonHeight-colorSelectHeight);

	// initialized scale select menu field

	scaleSelect->Menu()->SetTargetForItems(this);
	
	scaleSelect->MoveTo(colorSelectRight+dist, bounds.bottom-4*dist-buttonHeight-colorSelectHeight);

	// create outline list view and scrollview

	BRect treeViewRect(dist, dist, bounds.right-dist-B_V_SCROLL_BAR_WIDTH, 
						bounds.bottom-5*dist-buttonHeight-colorSelectHeight);

	// COutlineListViewEx automatically deletes the tree items.
	treeView = new COutlineListViewEx(treeViewRect, "ListView", B_SINGLE_SELECTION_LIST,
						B_FOLLOW_ALL);
	
	treeView->SetInvocationMessage(new BMessage(MSG_OK));
	treeView->SetSelectionMessage(new BMessage(MSG_SELECTION_CHANGED));
	
	BScrollView *scrollView = new BScrollView("ListViewScrollView",
						treeView, B_FOLLOW_ALL, 0, false, true);
	scrollView->SetFlags(scrollView->Flags() | B_FULL_UPDATE_ON_RESIZE);

	FillTreeView();	

	// create horizontal line
	
	CBorderView *box = new CBorderView(
						BRect(0, bounds.bottom-2*dist-buttonHeight-1,
						bounds.right+1, bounds.bottom-2*dist-buttonHeight),
						"HR", 1, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);

	// add children

	AddChild(box);
	AddChild(colorSelect);
	AddChild(scaleSelect);
	AddChild(scrollView);

	// additional init

	treeView->SetTarget(this);
}

void CPerformanceAddView::FillTreeView()
{
	CPointer<IPerformanceCounter> root = global_Namespace->Root();
	
	AddChildren(root, NULL); 

	// Sorting the tree simply takes too long (about 2 secs).	
	// treeView->FullListSortItems(ItemCompareFunc);
}

void CPerformanceAddView::AddChildren(IPerformanceCounter *counter, BListItem *parent)
{
	for(int i=0 ; i<counter->CountChildren() ; i++) {
		IPerformanceCounter *child = counter->ChildAt(i);
		
		BStringItem *item = new CCounterItem(child);
		
		if(parent == NULL) {
			treeView->AddItem(item);
		} else {
			treeView->AddUnder(item, parent);
			treeView->Collapse(item);
		}

		AddChildren(child, item);
	}
}

rgb_color CPerformanceAddView::SelectedColor()
{
	if(autoSelectColor) {
		int32 offset		= IndexOfColor(selColor);
		int32 itemCount		= colorSelect->Menu()->CountItems();
		int32 colorCount	= itemCount-3;
		bool foundColor		= false;
	
		if(offset < 0) {
			// invalid selColor. Set to 2 because 1 is added afterwards.
			offset = 2;
		}

		// Get list of currently used colors
		BMessage usedColors;
		GetClientProperty(DATA_INFO_PROP_COLOR, &usedColors);
	
		for(int i=1 ; i<colorCount ; i++) {
			int32 index = i + offset;

			if(index >= itemCount)
				index -= colorCount;
				
			CColorSelectMenuItem *item = 
				dynamic_cast<CColorSelectMenuItem *>(colorSelect->Menu()->ItemAt(index));

			MY_ASSERT(item);
			
			rgb_color color = item->Color();
			bool colorUsed = false;
			rgb_color usedColor;
			
			for(int k=0 ; FindColor(&usedColors, "result", k, usedColor) == B_OK ; k++) {
				colorUsed = (usedColor == color);
				
				if(colorUsed) break;
			}
			
			if(!colorUsed) {
				// we found an unused color
				selColor = color;
				foundColor = true;
				break;
			} else {
				// try with next color
			}
		}
		
		if(!foundColor) {
			// Still no unused color found. Simply use next color.
			int32 index = offset+1;
			
			if(index >= itemCount)
				index -= colorCount;
			
			CColorSelectMenuItem *item = 
				dynamic_cast<CColorSelectMenuItem *>(colorSelect->Menu()->ItemAt(index));

			MY_ASSERT(item);

			selColor = item->Color();			
		}
	} 

	return selColor;
}

void CPerformanceAddView::GetPreferredSize(float *width, float *height)
{
	float buttonAreaWidth, buttonAreaHeight;

	CLocalizedDialogBase::GetPreferredSize(&buttonAreaWidth, &buttonAreaHeight);

	if(width) {
		// Those menufields aren't resizable. They don't calculate the
		// size of the embedded menubar correctly. This can't be corrected
		// by a derived class, because the size is either automatically
		// resetted (if resizeToFit is set) or the menu bar doesn't return
		// the correct preferred size (if resizeToFit isn't set).
		// Therefore I don't allow the dialog to be resized vertically.
		// To do this I have to estimate the size of the menu-fields.
		// This is done in the function create_menu_field().
	
		*width = MAX(buttonAreaWidth, 
					 colorSelect->Bounds().Width()+scaleSelect->Bounds().Width()+3*dist);
	}
		
	if(height) {
		// This is only an estimated size. I simply say that the menu fields are
		// at least as high as a button.
		// The minimal height of the tree view should be about 100 pixel.
		*height = buttonAreaHeight + 2*dist + 100.0;
	}
}

int32 CPerformanceAddView::IndexOfColor(rgb_color color)
{
	int32 itemCount = colorSelect->Menu()->CountItems();

	for(int i=3 ; i<itemCount ; i++) {
		CColorSelectMenuItem *item = 
			dynamic_cast<CColorSelectMenuItem *>(colorSelect->Menu()->ItemAt(i));

		if(item && item->Color() == color)
			return i;	
	}
	
	return -1;
}

void CPerformanceAddView::LoadUserdefinedColors()
{
	CTaskManagerPrefs prefs;

	for(int32 i=0 ; i<MAX_USERDEFINED_COLORS ; i++) {
		char prefName[255];
		rgb_color color;
		
		sprintf(prefName,"%s%ld", PrefBaseName(), i);
	
		if(!prefs.IsSpecified(prefName))
			break;
	
		prefs.Read(prefName, color, CColor::Red);
			
		userDefinedColors.AddItem(new CColor(color));
	}
}

void CPerformanceAddView::SaveUserdefinedColors()
{
	CTaskManagerPrefs prefs;

	for(int32 i=0 ; i<userDefinedColors.CountItems() ; i++) {
		char prefName[255];
		sprintf(prefName, "%s%ld", PrefBaseName(), i);
	
		prefs.Write(prefName, *userDefinedColors.ItemAt(i));
	}
}

void CPerformanceAddView::AddToUserdefinedColors(const rgb_color &color)
{
	// Add at head of list
	userDefinedColors.AddItem(new CColor(color), 0);
			
	if(userDefinedColors.CountItems() > MAX_USERDEFINED_COLORS) {
		// Too many items in list. Remove last item.
		delete userDefinedColors.RemoveItem(userDefinedColors.CountItems()-1); 
	}
	
	SaveUserdefinedColors();
}

BMenuItem *CPerformanceAddView::CreateColorItem(const rgb_color &color, const char *name)
{
	const char *itemName;
	char hexName[20];

	if(name != NULL)
		itemName = name;
	else {
		sprintf(hexName,"#%02X%02X%02X", color.red, color.green, color.blue);
		
		itemName = hexName;
	}
		
	BMessage *colorSelMessage = new BMessage(MSG_COLOR_SELECTED);
	AddColor(colorSelMessage, MESSAGE_DATA_ID_COLOR, color);
		
	BMenuItem *colorSelItem = new CColorSelectMenuItem(itemName, color, 
									colorSelMessage);
		
	return colorSelItem;
}

void CPerformanceAddView::SelectColor(rgb_color color, bool addColor)
{
	int32 index = IndexOfColor(color);
					
	if(index < 0) {
		if(addColor) {
			// New color. Add it to list.
			
			AddToUserdefinedColors(color);
		
			BMenuItem *colorSelItem = CreateColorItem(color);
		
			colorSelItem->SetMarked(true);
			colorSelItem->SetTarget(this);
			colorSelect->Menu()->AddItem(colorSelItem);
		}
	} else {
		// Color already in list. Select it.
		
		BMenuItem *item = colorSelect->Menu()->ItemAt(index);
		
		if(item)
			item->SetMarked(true);
	}
	
	selColor = color;
	autoSelectColor = false;
}

void CPerformanceAddView::SelectAutomatic()
{
	// Select "Automatic" color selection.

	BMenuItem *item = colorSelect->Menu()->ItemAt(0);
	item->SetMarked(true);
						
	autoSelectColor = true;
}

void CPerformanceAddView::SelectScale(float scale)
{
	float exp = log10(scale);
	
	if(exp >= -3 && exp <= 3) {
		BMenuItem *item = scaleSelect->Menu()->ItemAt((int32)exp+3);

		if(item)
			item->SetMarked(true);		
	}
	
	selScale = scale;
}

// Returns a property of the clients 'DataInfo' properties in the 'result' field of the
// caller allocated BMessage. If 'index' is negative that property is requested for
// all 'DataInfo' properties. Otherwise it's only requested for the DataInfo at the
// passed index.
status_t CPerformanceAddView::GetClientProperty(const char *propName, BMessage *propMsg, 
	int32 index)
{
	BMessage scriptMessage(B_COUNT_PROPERTIES), reply;
						
	scriptMessage.AddSpecifier(GRAPH_VIEW_PROP_DATA_INFO);

	status_t result;
	int32 dataInfoCount=0;

	if(index < 0) {
		RETURN_IF_FAILED( Messenger().SendMessage(&scriptMessage, &reply) );

		if(reply.FindInt32("error", &result) == B_OK && result != B_OK)
			return result;

		RETURN_IF_FAILED( reply.FindInt32("result", &dataInfoCount) );
		
		if(dataInfoCount <= 0) {
			// the property list is empty.
			propMsg->MakeEmpty();
			return B_OK;
		}
	}

	reply.MakeEmpty();
			
	scriptMessage.MakeEmpty();
	scriptMessage.what = B_GET_PROPERTY;
			
	scriptMessage.AddSpecifier(propName);

	if(index < 0) {
		scriptMessage.AddSpecifier(GRAPH_VIEW_PROP_DATA_INFO, 0, dataInfoCount);
	} else {
		scriptMessage.AddSpecifier(GRAPH_VIEW_PROP_DATA_INFO, index);
	}
			
	RETURN_IF_FAILED( Messenger().SendMessage(&scriptMessage, propMsg) );
	
	if(propMsg->FindInt32("error", &result) == B_OK && result != B_OK)
		return result;
	
	return B_OK;
}

status_t CPerformanceAddView::GetDataProviderInfo(
	IDataProvider  *dataProvider,
	bool           &partOfGraphView,
	rgb_color      &dataProviderColor,
	float          &dataProviderScale)
{
	partOfGraphView   = false;
	dataProviderColor = CColor::Black;
	dataProviderScale = 1.0;	

	BMessage reply;
	status_t result;

	// Get a list of all data providers which are displayed in the GraphView.
	if((result = GetClientProperty(DATA_INFO_PROP_DATA_PROVIDER, &reply)) == B_OK) {
		IDataProvider *msgDataProvider;
		int32 dataProviderIndex = 0;
		
		partOfGraphView = false;
		
		// look if that counter is already part of the
		// graph view.
		while( reply.FindPointer("result", dataProviderIndex, (void **)&msgDataProvider) == B_OK) {
			if(msgDataProvider->Equal(dataProvider)) {
				// Get color
				reply.MakeEmpty();

				RETURN_IF_FAILED( GetClientProperty(DATA_INFO_PROP_COLOR, &reply, dataProviderIndex) );
				RETURN_IF_FAILED( FindColor(&reply, "result", dataProviderColor) );
	
				// Get scale
				reply.MakeEmpty();

				RETURN_IF_FAILED( GetClientProperty(DATA_INFO_PROP_SCALE, &reply, dataProviderIndex) );
				RETURN_IF_FAILED( reply.FindFloat("result", &dataProviderScale) );
	
				partOfGraphView = true;
	
				break;
			}
			
			dataProviderIndex++;
		}
	}
	
	return B_OK;
}

bool CPerformanceAddView::Ok()
{
	int32 selIndex = treeView->FullListCurrentSelection(0);

	if(selIndex >= 0) {
		BMessage addMessage, reply/*ignored*/;
		
		// Create a copy of the default message.
		addMessage = *Message();
	
		CCounterItem *counterItem = dynamic_cast<CCounterItem *>(
			treeView->FullListItemAt(selIndex));

		MY_ASSERT(counterItem != NULL);
			
		IDataProvider *dataProvider = counterItem->DataProvider();
	
		if(dataProvider && Messenger().IsValid()) {
			// The destination needs to create a clone of that
			// object, when it wants to store it.
			addMessage.AddPointer(MESSAGE_DATA_ID_DATA_PROVIDER, 
				dataProvider);
			addMessage.AddFloat(MESSAGE_DATA_ID_SCALE, selScale);

			AddColor(&addMessage, MESSAGE_DATA_ID_COLOR, SelectedColor());

			status_t status;

			// I don't use Invoke() here, because SendMessage with
			// a reply message ensures that SendMessage doen't return
			// until the message was processed by the destination.
			// This ensures that the passed pointer remains valid.
			if((status = Messenger().SendMessage(&addMessage, &reply)) != B_OK) {
				show_alert(strerror(status));
			}
		}
	} else {
		beep();
	}
	
	// don't close window
	return false;
}

void CPerformanceAddView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_SELECTION_CHANGED:
			{
				int32 selIndex = treeView->FullListCurrentSelection(0);

				bool addButtonEnable = false;

				if(selIndex > 0) {
					 CCounterItem *counterItem = dynamic_cast<CCounterItem *>(
						treeView->FullListItemAt(selIndex));
					
					if(counterItem && counterItem->DataProvider()) {
						addButtonEnable = true;
						
						bool alreadyInGraphView;
						float scale;
						rgb_color color;
						
						status_t result = GetDataProviderInfo(counterItem->DataProvider(), 
							alreadyInGraphView, color, scale);
						
						if(result == B_OK) { 
							if(alreadyInGraphView) {
								// DataProvider is already displayed in graph view!
								// If the user presses the "Add" button the
								// current scale and color settings are used for
								// that DataProvider, but it isn't added again.
								
								// -> Select the color/scale settings of that 
								// DataProvider and change the label of the
								// add button to "Change".
								
								SelectColor(color, true);
								SelectScale(scale);
								
								SetOkButtonLabel(CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_CHANGE_BUTTON_LABEL));
							} else {
								SetOkButtonLabel(CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_ADD_BUTTON_LABEL));
							}
						} else {
							CLocalizedString message("PerformanceAddView.ErrorMessage.ScriptRequestFailed");
							
							message << strerror(result) << result;
							
							show_alert(message);
						}
					} else {
						SetOkButtonLabel(CLocalizationHelper::GetDefaultInstance()->String(LOC_KEY_ADD_BUTTON_LABEL));
					}
				}
				
				EnableOkButton(addButtonEnable);
			}
			break;
		case MSG_COLOR_SELECTED:
			{
				rgb_color color;
				
				if(FindColor(message, MESSAGE_DATA_ID_COLOR, color) == B_OK) {
					selColor = color;
					autoSelectColor = false;
				} else {
					// If no color is attached to the message, 
					// the "Automatic" item was selected.
					autoSelectColor = true;
				}
			}
			break;
		case MSG_VIEW_COLOR_SELECTOR:
			{
				CColorSelectDialog *colorSelDialog =
					new CColorSelectDialog(Window(),
					CLocalizationHelper::GetDefaultInstance()->String("PerformanceAddView.ColorSelectDialog.Title"));
					
				colorSelDialog->SetTarget(this);
				colorSelDialog->SetMessage(new BMessage(MSG_COLOR_SELECTOR_CLOSED));
				colorSelDialog->SetColor(SelectedColor());
				colorSelDialog->Show();
			}
			break;
		case MSG_COLOR_SELECTOR_CLOSED:
			{
				bool canceled;

				if(message->FindBool(MESSAGE_DATA_ID_DIALOG_CANCELED, &canceled) == B_OK &&
					canceled == true) {
					// dialog was canceled!

					if(autoSelectColor)
						SelectAutomatic();
					else {
						// Reselect the old color.
						SelectColor(selColor);
					}
				} else {
					rgb_color color;
				
					if(FindColor(message, MESSAGE_DATA_ID_COLOR, color) == B_OK) {
						SelectColor(color, true);
					}
				}
			}
			break;
		case MSG_SCALE_SELECTED:
			{
				float scale;
				
				if(message->FindFloat(MESSAGE_DATA_ID_SCALE, &scale) == B_OK) {
					selScale = scale;
				} 
			}
			break;
		case MSG_SELECT_DATA_PROVIDER:
			{
				IDataProvider *selDataProvider = NULL;
				const char *selPerfCounterPath = NULL;

				if(message->FindPointer(MESSAGE_DATA_ID_DATA_PROVIDER, (void **)&selDataProvider) == B_OK ||
				   message->FindString(MESSAGE_DATA_ID_PERF_COUNTER_PATH, &selPerfCounterPath) == B_OK) {
					CCounterItem *item;
				
					int32 foundIndex = -1;
				
					for(int32 i=0 ; (item = dynamic_cast<CCounterItem *>(treeView->FullListItemAt(i))) != NULL ; i++) {
						if(selDataProvider) {
							IDataProvider *itemDataProvider = item->DataProvider();

							if(itemDataProvider && itemDataProvider->Equal(selDataProvider)) {
								// found correct data provider
								foundIndex = i;
								break;
							}
						} else if(selPerfCounterPath) {
							if(strcmp(item->Path(), selPerfCounterPath) == 0) {
								// found correct data provider
								foundIndex = i;
								break;
							}
						}
					}
					
					if(foundIndex != -1) {
						treeView->FullListSelect(foundIndex);
						treeView->ScrollToSelection();

						bool expand;
						
						if(message->FindBool(MESSAGE_DATA_ID_EXPAND_FOUND, &expand) != B_OK)
							expand = false;
							
						if(expand)
							treeView->Expand(treeView->FullListItemAt(foundIndex));
					}
				}
			}
			break;
		default:
			CLocalizedDialogBase::MessageReceived(message);
	}
}

// ==== CPerformanceAddView::CCounterItem ====

CPerformanceAddView::CCounterItem::CCounterItem(IPerformanceCounter *counter) :
	BStringItem(counter->Name())
{
	dataProvider = counter->DataProvider() ? counter->DataProvider()->Clone() : NULL;
	path		 = counter->Path();
}

CPerformanceAddView::CCounterItem::~CCounterItem()
{
	delete dataProvider;
}

// ==== CPerformanceAddWindow ====

// protected constructor
// No B_ASYNCHRONOUS_CONTROLS because BOutlineListView and BListView don't work
// correctly, if this flag is set.
CPerformanceAddWindow::CPerformanceAddWindow() :
	CSingletonWindow(BRect(30,30,300,350), 
		CLocalizationHelper::GetDefaultInstance()->String("PerformanceAddWindow.Title"), B_TITLED_WINDOW, 
		/*B_ASYNCHRONOUS_CONTROLS |*/ B_NOT_ZOOMABLE, B_CURRENT_WORKSPACE)
{
	// create view
	
	CPerformanceAddView *view = new CPerformanceAddView(Bounds(), NULL);

	float width, height;

	view->GetPreferredSize(&width, &height);

	SetSizeLimits(width, width, height, 2048);

	height = MAX(height, 320);

	// calculate center position
	
	BRect screenRect = BScreen(this).Frame();

	float wx = (screenRect.Width()-width)/2;
	float wy = (screenRect.Height()-height)/2;

	BRect centerRect = BRect(wx, wy, wx+width, wy+height);
	
	// load position from settings
	
	CTaskManagerPrefs prefs;
	
	BRect windowRect = prefs.AddPreformanceWindowRect(centerRect);
	
	if((windowRect | screenRect) != screenRect) {
		// The windowRect in the prefs lays outside the screen. Use
		// the centerRect instead.
		windowRect = centerRect;
	}

	MoveTo(windowRect.LeftTop());
	ResizeTo(width, windowRect.Height());
	
	view->ResizeTo(width, windowRect.Height());
	
	AddChild(view);
}

CPerformanceAddWindow::~CPerformanceAddWindow()
{
	RemoveFromList(ClassName());
}

bool CPerformanceAddWindow::QuitRequested()
{
	CTaskManagerPrefs prefs;
	
	prefs.SetAddPreformanceWindowRect(Frame());	
	
	return CSingletonWindow::QuitRequested();
}

CPerformanceAddWindow *CPerformanceAddWindow::CreateInstance()
{
	// Initialize to quiet compiler.
	CPerformanceAddWindow *window = NULL;

	return CreateSingleton(window, "CPerformanceAddWindow");
}

void CPerformanceAddWindow::DispatchMessage(BMessage *message, BHandler *target)
{
	switch(message->what) {
		case MSG_SELECT_DATA_PROVIDER:
			// forward to CPerformanceAddView
			ChildAt(0)->MessageReceived(message);
			break;
		default:
			CSingletonWindow::DispatchMessage(message, target);
	}
}

// ==== CLegendListView ====

CLegendListView::CLegendListView(BRect frame, const char *name, 
	list_view_type type, uint32 resizingMode, uint32 flags) :
	CFocusListView(frame, name, type, resizingMode, flags)
{
}
		
void CLegendListView::KeyDown(const char *bytes, int32 numBytes)
{
	if(bytes[0] == B_DELETE) {
		BMessage remove(MSG_REMOVE_PERFORMANCE_OBJECT);
		Messenger().SendMessage(&remove);
	}
		
	CFocusListView::KeyDown(bytes, numBytes);		
}

void CLegendListView::FrameResized(float width, float height)
{
	BListItem *item;
	BFont font;
		
	GetFont(&font);
	
	for(int i=0 ; (item = ItemAt(i)) != NULL ; i++) {
		item->Update(this, &font);
	}
}

// ==== CSplitterView ====

CSplitterView::CSplitterView(BRect frame, BView *left, BView *right, 
	bool horz, uint32 resizingMode, rgb_color bgColor) : 
	MakSplitterView(frame, left, right, horz, resizingMode, bgColor)
{
	SetFlags(Flags() | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
}

void CSplitterView::Draw(BRect updateRect)
{
	float width  = Bounds().Width();
	float height = Bounds().Height();
	
	int32 end = (int)((width-1)/2)*2;
	
	float lineHeight = MIN(70, height-4);
	float lineStart  = (height - lineHeight)/2;
	float lineEnd	 = lineStart + lineHeight;
	
	for(int32 i=0 ; i<end ; i++) {
		SetHighColor((i%2)==0 ? CColor::BeHighlight : CColor::BeShadow);
		StrokeLine(BPoint(i,lineStart), BPoint(i,lineEnd));
	}
		
	SetHighColor(CColor::BeHighlight);
	StrokeLine(BPoint(0, lineStart-1), BPoint(end-1, lineStart-1));
	SetHighColor(CColor::BeShadow);
	StrokeLine(BPoint(0, lineEnd+1), BPoint(end-1, lineEnd+1));
}

// ==== CPerformanceView ====

CPerformanceView::CPerformanceView(BRect frame) :
	BView(frame, "PerformanceView", B_FOLLOW_ALL_SIDES, 0)
{
	const float dist = 10;
	
	// --- Create child views at dummy position.
	
	BRect dummyPos = BRect(0, 0, 40, 40);
	
	addButton = new CFlickerFreeButton(dummyPos, "AddButton", 
			CLocalizationHelper::GetDefaultInstance()->String("PerformanceView.AddButton.Label"),
			new BMessage(MSG_VIEW_PERFORMANCE_OBJECTS),
			B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
								
	removeButton = new CFlickerFreeButton(dummyPos, "RemoveButton", 
			CLocalizationHelper::GetDefaultInstance()->String("PerformanceView.RemoveButton.Label"),
			new BMessage(MSG_REMOVE_PERFORMANCE_OBJECT), B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);

	CBorderView *borderView = new CBorderView(BRect(0,0,40,40), 
			"GraphViewBorder", 1, B_FOLLOW_ALL);

	graphView = new COverlayGraphView(dummyPos, "PreformanceGraphView", true, 100);

	CBugfixedDragger *dragger = new CBugfixedDragger(BRect(0,0,7,7), graphView,
			B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);

	listView = new CLegendListView(dummyPos, "Legend", B_SINGLE_SELECTION_LIST,
			B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE);

	// Container for GraphView, BorderView and dragger.
	leftPane  = new BView(dummyPos, "LeftPane", B_FOLLOW_ALL, 0);
	
	// Container for buttons and legend
	rightPane = new BView(dummyPos, "RightPane", B_FOLLOW_TOP_BOTTOM | B_FOLLOW_RIGHT, 0);

	// --- Move child views to correct position

	BRect bounds = Bounds();
	
	const float minButtonHeight=22;
	const float minButtonWidth=100;
	
	float buttonHeight;
	float buttonWidth;
	
	removeButton->GetPreferredSize(&buttonWidth, &buttonHeight);
	
	buttonWidth  = MAX(minButtonWidth, buttonWidth + 40);
	buttonHeight = MAX(minButtonHeight, buttonHeight);

	CTaskManagerPrefs prefs;

	float rightPaneWidth = prefs.PerformanceLegendBarWidth(buttonWidth + 2*dist);
	float rightPaneLeft  = bounds.right - rightPaneWidth;
	
	rightPane->MoveTo(rightPaneLeft, 0);
	rightPane->ResizeTo(rightPaneWidth, bounds.Height());

	addButton->MoveTo(dist, dist);
	addButton->ResizeTo(rightPaneWidth - 2*dist, buttonHeight);
	
	removeButton->MoveTo(dist, dist*2 + buttonHeight);
	removeButton->ResizeTo(rightPaneWidth - 2*dist, buttonHeight);

	listView->MoveTo(dist, dist*3+2*buttonHeight);
	listView->ResizeTo(rightPaneWidth - 2*dist, rightPane->Bounds().Height()-listView->Frame().top-dist);
	listView->SetSelectionMessage(new BMessage(MSG_SELECTION_CHANGED));
	listView->SetInvocationMessage(new BMessage(MSG_CHANGE_PRERFORMANCE_OBJECT));
	
	leftPane->MoveTo(0, 0);
	leftPane->ResizeTo(bounds.Width()-rightPaneWidth-dist/2, bounds.Height());

	borderView->MoveTo(dist, dist);
	borderView->ResizeTo(leftPane->Bounds().Width()  - 2*dist, 
						 leftPane->Bounds().Height() - 2*dist);

	graphView->MoveTo(borderView->ClientRect().LeftTop() + borderView->Frame().LeftTop());
	graphView->ResizeTo(borderView->ClientRect().Width(), borderView->ClientRect().Height());
	graphView->SetAutoScale(true);

	dragger->MoveTo(borderView->Frame().RightBottom() - BPoint(2,2));
	dragger->ResizeTo(7, 7);

	// Add children

	leftPane->AddChild(borderView);
	leftPane->AddChild(dragger);
	leftPane->AddChild(graphView);
	
	rightPane->AddChild(addButton);
	rightPane->AddChild(removeButton);
	rightPane->AddChild(listView);

	AddChild(leftPane);
	AddChild(rightPane);

	// Create splitter
	
	BRect splitterRect(leftPane->Frame().right, 0, rightPane->Frame().left, bounds.Height());
	
	CSplitterView *splitter = new CSplitterView(
		splitterRect, leftPane, rightPane, MAK_H_SPLITTER,
		B_FOLLOW_TOP_BOTTOM | B_FOLLOW_RIGHT, CColor::BeBackgroundGray);

	AddChild(splitter);
}

void CPerformanceView::AttachedToWindow()
{
	BView::AttachedToWindow();
	SetViewColor(CColor::BeBackgroundGray);

	listView->SetTarget(this);

	addButton->SetTarget(this);											
	removeButton->SetTarget(this);
	removeButton->SetEnabled(false);
	
	graphView->SetNotification(this);
	
	leftPane->SetViewColor(CColor::BeBackgroundGray);
	rightPane->SetViewColor(CColor::BeBackgroundGray);
}

void CPerformanceView::AllAttached()
{
	BView::AllAttached();
	
	listView->SetViewColor(CColor::BeBackgroundGray);
}

void CPerformanceView::DetachedFromWindow()
{
	CTaskManagerPrefs prefs;
	
	prefs.SetPerformanceLegendBarWidth(rightPane->Bounds().Width());
	
	BView::DetachedFromWindow();
}

void CPerformanceView::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case MSG_VIEW_PERFORMANCE_OBJECTS:
			// open add dialog
			{
				CPerformanceAddWindow *window = 
					CPerformanceAddWindow::CreateInstance();
				
				window->Invoker()->SetTarget(graphView);
				window->Show();
			}
			break;
		case MSG_CHANGE_PRERFORMANCE_OBJECT:
			{
				// get selected data provider
				
				int32 selIndex = listView->CurrentSelection(0);
				
				if(selIndex >= 0) {
					const CDataInfo *dataInfo = graphView->DataProviderAt(selIndex);
					
					if(dataInfo) {
						IDataProvider *dataProvider = dataInfo->DataProvider();
					
						if(dataProvider) {
							// Open dialog and select data provider.
						
							CPerformanceAddWindow *window = CPerformanceAddWindow::CreateInstance();
				
							window->Invoker()->SetTarget(graphView);
							window->Show();
		
							BMessage selectMsg(MSG_SELECT_DATA_PROVIDER);
							selectMsg.AddPointer(MESSAGE_DATA_ID_DATA_PROVIDER, dataProvider);
					
							window->PostMessage(&selectMsg);
						}
					}
				}
			}
			break;
		case MSG_REMOVE_PERFORMANCE_OBJECT:
			{
				int32 selIndex = listView->CurrentSelection(0);
				
				if(selIndex >= 0) {
					BMessage removeDataInfo(B_DELETE_PROPERTY);
					removeDataInfo.AddSpecifier(GRAPH_VIEW_PROP_DATA_INFO, selIndex);
					
					BMessenger graphViewMessenger(graphView);
					
					graphViewMessenger.SendMessage(&removeDataInfo);
				}
			}			
			break;
		case MSG_SELECTION_CHANGED:
			{
				int32 currentSel = listView->CurrentSelection(0);
		

				BMessage setOverlay(B_SET_PROPERTY);
				setOverlay.AddSpecifier(OVERLAY_VIEW_PROP_OVERLAY_INDEX);
				setOverlay.AddInt32("data", currentSel);

				BMessenger graphViewMessenger(graphView);
					
				graphViewMessenger.SendMessage(&setOverlay);
		
				removeButton->SetEnabled(currentSel >= 0);
			}
			break;
		case MSG_NOTIFY_DATA_PROVIDER_ADDED:
			{
				rgb_color color;
				IDataProvider *dataProvider;
				
				if( message->FindPointer(MESSAGE_DATA_ID_DATA_PROVIDER, (void **)&dataProvider) == B_OK &&
					FindColor(message, MESSAGE_DATA_ID_COLOR, color) == B_OK ) {
		
					listView->AddItem(new CColorSelectListItem(dataProvider->DisplayName().String(), color));
				}
			}
			break;
		case MSG_NOTIFY_DATA_INFO_CHANGED:
			{
				rgb_color color;
				int32 index;
				
				if( message->FindInt32(MESSAGE_DATA_ID_INDEX, &index) == B_OK &&
					FindColor(message, MESSAGE_DATA_ID_COLOR, color) == B_OK ) {
					// update entry
					CColorSelectListItem *item = 
						dynamic_cast<CColorSelectListItem* >(listView->ItemAt(index));
						
					if(item) {
						item->SetColor(color);
						listView->InvalidateItem(index);
					}
				}				
			}
			break;
		case MSG_NOTIFY_DATA_INFO_DELETED:
			{
				int32 index;
				
				if( message->FindInt32(MESSAGE_DATA_ID_INDEX, &index) == B_OK ) {
					BListItem *item = listView->RemoveItem(index);
					delete item;
				}
			}
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}

void CPerformanceView::DisplayAddDialog(const char *selCounterPath)
{
	// GraphView in Performance Tab.
	BView *target = be_app->WindowAt(0)->ChildAt(1)->ChildAt(0)->ChildAt(0)->ChildAt(2)->
							ChildAt(0)->ChildAt(2);
							
	CPerformanceAddWindow *window = CPerformanceAddWindow::CreateInstance();
				
	window->Invoker()->SetTarget(target);
	window->Show();
	
	BMessage selectMsg(MSG_SELECT_DATA_PROVIDER);
	selectMsg.AddString(MESSAGE_DATA_ID_PERF_COUNTER_PATH, selCounterPath);
	selectMsg.AddBool(MESSAGE_DATA_ID_EXPAND_FOUND, true);
				
	window->PostMessage(&selectMsg);
}
