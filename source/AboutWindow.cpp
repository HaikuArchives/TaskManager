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
#include "commondefines.h"
#include "LocalizationHelper.h"
#ifdef _TMGR_FULLBLUR
  #include "ABlur.h"
#else
  #include "Blur.h"
#endif
#include "AboutView.h"
#include "AboutWindow.h"

// protected constructor
CAboutWindow::CAboutWindow() : 
	CSingletonWindow(
		BRect(0,0,50,50), 
		CLocalizationHelper::GetDefaultInstance()->String("AboutWindow.Title"), 
		B_TITLED_WINDOW, 
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, 
		B_CURRENT_WORKSPACE)
{
	// the window width is fixed, the window height is
	// automatically adjusted to the height of the
	// AboutView.
	const float width  = 300;
	float height;

	BView *aboutView = new CAboutView(BRect(0,0,width,100));

	aboutView->GetPreferredSize(NULL, &height);
	aboutView->ResizeToPreferred();

	height += 10;

	BRect screenRect = BScreen(this).Frame();

	// center window
	float wx = (screenRect.Width()-width)/2;
	float wy = (screenRect.Height()-height)/2;

	MoveTo(wx, wy);
	ResizeTo(width, height);

	AddChild(aboutView);
	Show();
}

CAboutWindow::~CAboutWindow()
{
	RemoveFromList(ClassName());
}

CAboutWindow *CAboutWindow::CreateInstance()
{
	// Initialize to quiet compiler.
	CAboutWindow *window = NULL;

	return CreateSingleton(window, "CAboutWindow");
}
