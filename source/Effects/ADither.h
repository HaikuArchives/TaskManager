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
* CDither implementiert schnelles Dithering einer B_RGB32-Bitmap zum
* gewuenschten Farbformatbasierend auf der Hilbert-Kurve fuer den 
* Be Taskmanager von Thomas Krammer. 
* Algorithmus: 03/2000 Christopher Kohlert c@kohlert.de
* Code: Christopher Kohlert
***********************************************************************/
#ifndef __ADITHER_H
#define __ADITHER_H

#include "my_assert.h"
#include "pch.h"

// globals
BBitmap *CreateCloneBitmap(BBitmap *source);

// Dither-Class
class CDither
{ public:
    CDither();
    virtual ~CDither();
    
    BBitmap *GetAsScreenColorSpace(BBitmap &bitmap);
    BBitmap *GetAsBitmap(BBitmap &bitmap,color_space cspace);
    
  protected:
    // Bitmap Data
    void *buffer;
    uint32 width;
    uint32 height;
    uint32 cfrowbytes;
    // Hilbert-Curve Stuff (dither)
    enum {
      UP,
      LEFT,
      DOWN,
      RIGHT,
    };
    inline void RESETandmoveLeftBorder();
    inline void moveUP();
    inline void moveLEFT();
    inline void moveDOWN();
    inline void moveRIGHT();
    void hilbertTLTR(int level,int direction=UP);
    void hilbertBRBL(int level,int direction=UP);
    void hilbertTLBL(int level,int direction=UP);
    void hilbertBRTR(int level,int direction=UP);
    int HSubControlOToRight(uint32 width,uint32 height);
    void HSubControlODown(uint32 width,uint32 height);
    void HSubControlOToLeft(uint32 width,uint32 height);
    void HSubControlUDownPotSub(uint32 d,uint32 c,uint32 width);
    void HSubControlUDownPot(uint32 width,uint32 height);
    void HilbertController(uint32 width,uint32 height);
    // Dither-ColorSpace specific Functions
    typedef struct color
    { int16 r;
      int16 g;
      int16 b;
    };
    color Dither256(color coloraddin);
    color Dither15(color coloraddin);
    color Dither16(color coloraddin);
    color DitherGray8(color coloraddin);
    color Dither1(color coloraddin);
    void ResetDitherArray();
    // dithering Data
    uint8 targetmovep;
    uint32 targetrowbytes;
    uint8 *target;
    uint8 *targetstart;
    uint8 *srcbuffer;
    
    typedef color (CDither::*DitherFunc)(color coloraddin); 
    
    DitherFunc ditherfunction;
    const color_map *cm;
    BScreen *screen;
    color coll_error;
};

#endif // __ADITHER_H
