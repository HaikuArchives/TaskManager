/*
 * Copyright 2000 by Christopher Kohlert
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

/**********************************************************************
* CBlur implementiert einen schnellen Blur- und Alpha-Blend Effekt
* fuer den Be Taskmanager von Thomas Krammer. 
* Algorithmus: 03/2000 Christopher Kohlert c@kohlert.de
* Code: Christopher Kohlert und Thomas Krammer
***********************************************************************/
#ifndef __ABLUR_H
#define __ABLUR_H

#include "my_assert.h"
//#include "CFBitmap.h"
#include "pch.h"

// Blur-Klasse
/* diese Klasse ist leicht eingeschraenkt, sie unterstuetzt
   quasi nur ein Blur von BBitmap in einigen Aufloesungen
   horizontal oder horizontal und vertikal in eine 32Bit - BBitmap */
class CBlur
{ public:
    void SetSpreadSize(int32 size,rgb_color outside,float deckung=1);
    status_t BlurH(BBitmap *source, BBitmap *dest);
    status_t BlurV(BBitmap *sourceanddest);
    status_t BlurVB(BBitmap *source,BBitmap *dest);
    status_t BlurAll(BBitmap *source, BBitmap *dest);
    
  protected:
	struct lui // Lookup information
	{ uint16 addon;
	  uint16 reduce;
	};
	lui lubuffer[256];
	rgb_color outside;
	int32 msize;
	struct big_color
	{ uint16 red;
	  uint16 green;
	  uint16 blue;
	};
	
	struct sbig_color
	{ int32 red;
	  int32 green;
	  int32 blue;
	};
		
    typedef rgb_color (*GetPixelFunc)(uint8 **); 
	// little endian pixel set functions
	static rgb_color GetPixelRGB32(uint8 **ppos);
	static rgb_color GetPixelRGB15(uint8 **ppos);
	static rgb_color GetPixelRGB16(uint8 **ppos);

	// big endian pixel set functions
	static rgb_color GetPixelRGB32_Big(uint8 **ppos);
	static rgb_color GetPixelRGB15_Big(uint8 **ppos);
	static rgb_color GetPixelRGB16_Big(uint8 **ppos);
};

class CAlphaBlend
{ public:
    static void Blend(BBitmap *bmp1,BBitmap *bmp2,float mix=0.5);
};

class CMixedEffect
{ public:
    CMixedEffect(BBitmap *bmp1,BBitmap *bmp2,int32 maxsize=50);
    virtual ~CMixedEffect();

    BBitmap *GetEffect(float mixed=0.5);
  protected:
    CBlur cb;
    rgb_color outcolor;
    BBitmap *cfbmp1;
    BBitmap *cfbmp2;
    BBitmap *bmp1store;
    BBitmap *bmp2store;
    float last;
    int32 maxsize;
};

#endif // __ABLUR_H
