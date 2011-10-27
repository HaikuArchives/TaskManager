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
 
#ifndef COLOR_H
#define COLOR_H

//! file=Color.h

//: CColor is a simple wrapper class for the rgb_color structure.
// This class contains a number of static predefined colors.
// Most of these colors are the default UI colors of BeOS. 
// The other colors define often used basic colors like black and white.
class CColor : public rgb_color
{
	public:
	CColor();
	CColor(const CColor &other);
	CColor(const rgb_color &other);
	CColor(const rgb_color &other, float tint);
	CColor(color_which uiColor, float tint=B_NO_TINT);
	CColor(uint8 red, uint8 green, uint8 blue, uint8 alpha=255);

	void PrintToStream() const;

	//: Add two colors.
	// Adding two colors means that every color component is added seperatly and
	// restricted to the range beween 0 and 255. The alpha channel is set to 255.
	CColor operator + (const CColor &other);
	
	//: Substract two colors.
	// Substracting two colors means that every color component is substraced seperatly and
	// restricted to the range beween 0 and 255. The alpha channel is set to 255.
	CColor operator - (const CColor &other);

	//: Add color.
	// See operator + for details.
	const CColor &operator += (const CColor &other);
	//: Substract color.
	// See operator - for details.
	const CColor &operator -= (const CColor &other);

	//: Assign a rgb_color.
	const CColor &operator= (const rgb_color &other);
	//: Assign a CColor.
	const CColor &operator= (const CColor &other);
	
	//: Background color for panels and controls.
	static const CColor BeBackgroundGray;
	//: Color of inactive controls.
	static const CColor BeInactiveControlGray;
	//: Color of the rectangle around focused controls.
	static const CColor BeFocusBlue;
	//: Highlight (for 3D look)
	static const CColor BeHighlight;
	//: Shadow (for 3D look)
	static const CColor BeShadow;
	//: Dark shadow (for 3D look)
	static const CColor BeDarkShadow;
	//: Light shadow (for 3D look)
	static const CColor BeLightShadow;
	//: Button background.
	static const CColor BeButtonGray;
	//: Same as BeInactiveControlGray.
	static const CColor BeInactiveGray;
	//: Background color for selected entries in a listview.
	static const CColor BeListSelectGray;
	//: Background color of the title tab.
	static const CColor BeTitleBarYellow;
	//: Background color for menus.
	static const CColor BeMenuBackgroundGray;
	//: Background color for the selected entry in a menu.
	static const CColor BeMenuSelectGray;

	//: Transparent color (B_TRANSPARENT_COLOR).
	static const CColor Transparent;
	
	//: Pure red (r=255, g=0, b=0)
	static const CColor Red;
	//: Pure green (r=0, g=255, b=0)
	static const CColor Green;
	//: Pure blue (r=0, g=0, b=255)
	static const CColor Blue;
	//: Pure white (r=255, b=255, g=255)
	static const CColor White;
	//: Pure black (r=0, b=0, g=0)
	static const CColor Black;
	//: Pure yellow (r=255, g=255, b=0)
	static const CColor Yellow;
	//: Pure cyan (r=0, g=255, b=255)
	static const CColor Cyan;
	//: Pure magenta (r=255, g=0, b=255)
	static const CColor Magenta;
};

#if B_BEOS_VERSION < B_BEOS_VERSION_DANO
// Dano defines these operators in rbg_color

bool operator == (const CColor &o1, const CColor &o2);
bool operator == (const rgb_color &o1, const rgb_color &o2);
bool operator != (const CColor &o1, const CColor &o2);
bool operator != (const rgb_color &o1, const rgb_color &o2);

#endif // B_BEOS_VERSION_DANO

#endif // COLOR_H