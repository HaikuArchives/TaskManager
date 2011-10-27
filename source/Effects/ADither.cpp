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

/* class for hilbert-curve dithering
   by C.Kohlert c@kohlert.de */
#include "pch.h"
#include "ADither.h"
/* - class Adither ---------------------------------------------------------------------- */

/* does nothing */
CDither::CDither()
{ 
}

/* does nothing */
CDither::~CDither()
{ 
}

CDither::color CDither::Dither256(CDither::color coloraddin)
{ rgb_color pixel=*((rgb_color*)srcbuffer);
  int32 b=pixel.red+coloraddin.b; // uint8
  int32 g=pixel.green+coloraddin.g;
  int32 r=pixel.blue+coloraddin.r;
  uint8 rt,gt,bt; 
  if (r>0xFF) { rt=0xFF; r=(r-0xFF)/2+0xFF; } 
    else if (r<0) { rt=0; r=r/2; } else {rt=r;}
  if (g>0xFF) { gt=0xFF; g=(g-0xFF)/2+0xFF; } 
    else if (g<0) { gt=0; g=g/2; } else {gt=g;}
  if (b>0xFF) { bt=0xFF; b=(b-0xFF)/2+0xFF; } 
    else if (b<0) { bt=0; b=b/2; } else {bt=b;}
  uint8 tmp=screen->IndexForColor(rt,gt,bt);
  rgb_color bestFit = cm->color_list[*target=tmp];
  coloraddin.r=(int32)r-bestFit.red;
  coloraddin.g=(int32)g-bestFit.green;
  coloraddin.b=(int32)b-bestFit.blue;  
  return coloraddin;
}

CDither::color CDither::Dither15(CDither::color coloraddin)
{ rgb_color pixel=*((rgb_color*)srcbuffer);
  int32 b=pixel.red  +coloraddin.b; // uint8
  int32 g=pixel.green+coloraddin.g;
  int32 r=pixel.blue +coloraddin.r;
  uint8 rt,gt,bt; 
  if (r>0xFF) { rt=0xFF; r=(r-0xFF)/2+0xFF; } 
    else if (r<0) { rt=0; r=r/2; } else {rt=r;}
  if (g>0xFF) { gt=0xFF; g=(g-0xFF)/2+0xFF; } 
    else if (g<0) { gt=0; g=g/2; } else {gt=g;}
  if (b>0xFF) { bt=0xFF; b=(b-0xFF)/2+0xFF; } 
    else if (b<0) { bt=0; b=b/2; } else {bt=b;}
  int16 color=((bt & 0xF8)>>3) | ((gt & 0xF8)<<2) | ((rt & 0xF8)<<7);
  *((uint16*)target)=color;
  coloraddin.r=(int32)r-(rt & 0xF8);
  coloraddin.g=(int32)g-(gt & 0xF8);
  coloraddin.b=(int32)b-(bt & 0xF8); 
  return coloraddin;
}

CDither::color CDither::Dither16(CDither::color coloraddin)
{ int32 b=*(srcbuffer)  +coloraddin.b; // uint8
  int32 g=*(srcbuffer+1)+coloraddin.g;
  int32 r=*(srcbuffer+2)+coloraddin.r;
  uint8 rt,gt,bt; 
  if (r>0xFF) { rt=0xFF; r=(r-0xFF)/2+0xFF; } 
    else if (r<0) { rt=0; r=r/2; } else {rt=r;}
  if (g>0xFF) { gt=0xFF; g=(g-0xFF)/2+0xFF; } 
    else if (g<0) { gt=0; g=g/2; } else {gt=g;}
  if (b>0xFF) { bt=0xFF; b=(b-0xFF)/2+0xFF; } 
    else if (b<0) { bt=0; b=b/2; } else {bt=b;}
  int16 color=((bt & 0xF8)>>3) | ((gt & 0xFC)<<3) | ((rt & 0xF8)<<8);
  *((uint16*)target)=color;
  coloraddin.r=(int32)r-(rt & 0xF8);
  coloraddin.g=(int32)g-(gt & 0xFC);
  coloraddin.b=(int32)b-(bt & 0xF8); 
  return coloraddin;
}

/* red is the value of choice */
CDither::color CDither::DitherGray8(CDither::color coloraddin)
{ int32 gray=*(srcbuffer);
  gray+=*(srcbuffer+1);
  gray+=*(srcbuffer+2);
  gray=((gray*256)/3)+coloraddin.r; // uint8
  uint8 grayt; 
  if (gray/256>0xFF) { grayt=0xFF; gray=((gray/256-0xFF)/2+0xFF)*256; } 
    else if (gray/256<0) { grayt=0; gray=((gray/256)/2)*256; } else {grayt=gray/256;}
  // *target=grayt;
  *target=screen->IndexForColor(grayt,grayt,grayt); // for B_CMAP8 Kompatibility :(
  coloraddin.r=(int32)gray-(grayt<<8);
  return coloraddin;
}

CDither::color CDither::Dither1(CDither::color coloraddin)
{ // @@ not implemented
  return coloraddin;
}

BBitmap *CreateCloneBitmap(BBitmap *source) // RGB_32 only
{ if (!source) return NULL;
  uint height=(uint)source->Bounds().Height()+1;
  uint width=(uint)source->Bounds().Width()+1;
  int32 sbpr=source->BytesPerRow();
  BBitmap *ret=new BBitmap(source->Bounds(),source->ColorSpace());
  int32 dbpr=ret->BytesPerRow();
  uint8 *sbuffer=(uint8 *)source->Bits();
  uint8 *dbuffer=(uint8 *)ret->Bits();
  while (height--)
  { uint cx=width;
    uint32 *sbuffert=(uint32*)sbuffer;
    uint32 *dbuffert=(uint32*)dbuffer;    
    
    while (cx--) *(dbuffert++)=*(sbuffert++);
    
    dbuffer+=dbpr;
    sbuffer+=sbpr;
  }
  return ret;
}

/* Returns the contents of the bitmap in a BBitmap in
   the defined colorspace. To dither the image it used
   an optimized Riemersma dither - Algorithm (fast!) 
   which is explained under:
   http://www.compuphase.com/riemer.htm
   The Hilbert curve is explained under:
   http://www.compuphase.com/hilbert.htm
   In near future all related documents may be routed
   on our server: http://www.3rd-evolution.de
*/    
BBitmap *CDither::GetAsBitmap(BBitmap &sourcebitmap,color_space cspace)
{ // Parameter der sourcebitmap entsprechend umsetzen:
  buffer=(void*)sourcebitmap.Bits();
  width=(uint32)sourcebitmap.Bounds().Width();
  height=(uint32)sourcebitmap.Bounds().Height();
  cfrowbytes=sourcebitmap.BytesPerRow();
  switch (cspace)
    { case B_RGB32:  // just a copy...
        { BBitmap *bmp=CreateCloneBitmap(&sourcebitmap);
          return bmp;
        }
        break;
      case B_RGB16_BIG: // not supported by BScreen 
      case B_RGB16:  // 16-Bit-Color dither
        { BBitmap *bmp=new BBitmap(BRect(0,0,width,height),B_RGB16);
          if (!bmp) return bmp;
          target=(uint8 *)bmp->Bits();
          targetrowbytes=bmp->BytesPerRow();
          targetstart=target;
          ditherfunction=&CDither::Dither16;
          srcbuffer=(uint8 *)buffer;
          targetmovep=2;
          ResetDitherArray();
          HilbertController(width,height);
          return bmp;
        }      
        break;      
      case B_RGB15_BIG: // not supported by BScreen 
      case B_RGBA15: // not supported by BScreen
      case B_RGBA15_BIG: // not supported by BScreen
      case B_RGB15:  // 15-Bit-Color dither
        { BBitmap *bmp=new BBitmap(BRect(0,0,width,height),B_RGB15);
          if (!bmp) return bmp;
          target=(uint8 *)bmp->Bits();
          targetrowbytes=bmp->BytesPerRow();
          targetstart=target;
          ditherfunction=&CDither::Dither15;
          srcbuffer=(uint8 *)buffer;
          targetmovep=2;
          ResetDitherArray();
          HilbertController(width,height);
          return bmp;
        }
        break;      
      case B_CMAP8:  // 256-Color dither
        { BBitmap *bmp=new BBitmap(BRect(0,0,width,height),B_CMAP8);
          if (!bmp) return bmp;
          target=(uint8 *)bmp->Bits();
          targetrowbytes=bmp->BytesPerRow();
          targetstart=target;
          ditherfunction=&CDither::Dither256;
          srcbuffer=(uint8 *)buffer;
          targetmovep=1;
          BScreen _screen;
          cm=_screen.ColorMap();
          screen=&_screen;
          ResetDitherArray();
          HilbertController(width,height);
          return bmp;
        }
        break;
      case B_GRAY8:  // not supported by BBitmap - it is a B_CMAP8 :(
        { BBitmap *bmp=new BBitmap(BRect(0,0,width,height),B_GRAY8);
          if (!bmp) return bmp;
          target=(uint8 *)bmp->Bits();
          targetrowbytes=bmp->BytesPerRow();
          targetstart=target;
          ditherfunction=&CDither::DitherGray8;
          srcbuffer=(uint8 *)buffer;
          targetmovep=1;

          BScreen _screen;        // for B_CMAP8 Kompatibility :(
          cm=_screen.ColorMap();  // for B_CMAP8 Kompatibility :(
          screen=&_screen;        // for B_CMAP8 Kompatibility :(

          ResetDitherArray();
          HilbertController(width,height);
          return bmp;
        }
        break;
      case B_GRAY1:  // not supported by BScreen
        { BBitmap *bmp=new BBitmap(BRect(0,0,width,height),B_GRAY1);
          if (!bmp) return bmp;
          target=(uint8 *)bmp->Bits();
          targetrowbytes=bmp->BytesPerRow();
          targetstart=target;
          ditherfunction=&CDither::Dither1;
          srcbuffer=(uint8 *)buffer;
          targetmovep=0; // wont work - think again!
          ResetDitherArray();
          HilbertController(width,height);
          return bmp;
        }
        break;
      case B_RGB32_BIG: // not supported by BScreen
      case B_RGBA32_BIG: // not supported
      case B_RGB24_BIG:  // not supported
      case B_RGBA32: // not supported
      case B_RGB24:  // not supported
      case B_NO_COLOR_SPACE: // undefined...
      default:
        return GetAsBitmap(sourcebitmap,B_RGB32);
    } // /switch
}

/* Returns a BBitmap in screen-colorspace */
BBitmap *CDither::GetAsScreenColorSpace(BBitmap &bitmap)
{ 
  return GetAsBitmap(bitmap,BScreen().ColorSpace());
}

void CDither::ResetDitherArray()
{ coll_error.r=coll_error.g=coll_error.b=0;
}

inline void CDither::RESETandmoveLeftBorder()
{ target=targetstart+((int)((target-targetstart)/targetrowbytes))*targetrowbytes;
  srcbuffer=(uint8 *)((uint8 *)buffer+((int)((srcbuffer-(uint8 *)buffer)/cfrowbytes))*cfrowbytes);
  ResetDitherArray();
}

/* inline moveDOWN etc... */
inline void CDither::moveUP()
{ coll_error=(this->*ditherfunction)(coll_error);
  srcbuffer-=cfrowbytes;
  target-=targetrowbytes;
}

inline void CDither::moveLEFT()
{ coll_error=(this->*ditherfunction)(coll_error);
  srcbuffer-=4;
  target-=targetmovep;
}

inline void CDither::moveDOWN()
{ coll_error=(this->*ditherfunction)(coll_error);
  srcbuffer+=cfrowbytes;
  target+=targetrowbytes;  
}

inline void CDither::moveRIGHT()
{ coll_error=(this->*ditherfunction)(coll_error);
  srcbuffer+=4;
  target+=targetmovep;  
}

/* die normale Hilbert-Kurve.
   Start: Top-Left  Ende: Top-Right */
void CDither::hilbertTLTR(int level,int direction)
{ level--;
  if (!level)
  { switch(direction)
    { case LEFT:
        moveRIGHT();
        moveDOWN();
        moveLEFT();
        break;
      case RIGHT:
        moveLEFT();
        moveUP();
        moveRIGHT();
        break;
      case UP:
        moveDOWN();
        moveRIGHT();
        moveUP();
        break;
      case DOWN:
        moveUP();
        moveLEFT();
        moveDOWN();
        break;
    }
  } else {
    if (level==1) // little Optimisation
    { switch(direction)
      { case LEFT:
           moveDOWN();
           moveRIGHT();
           moveUP();
          moveRIGHT();
           moveRIGHT();
           moveDOWN();
           moveLEFT();
          moveDOWN();
           moveRIGHT();
           moveDOWN();
           moveLEFT();
          moveLEFT();
           moveUP();
           moveLEFT();
           moveDOWN();
          break;
        case RIGHT:
           moveUP();
           moveLEFT();
           moveDOWN();
          moveLEFT();
           moveLEFT();
           moveUP();
           moveRIGHT();
          moveUP();
           moveLEFT();
           moveUP();
           moveRIGHT();
          moveRIGHT();
           moveDOWN();
           moveRIGHT();
           moveUP();
          break;
        case UP:
           moveRIGHT();  
           moveDOWN();
           moveLEFT();
          moveDOWN();
           moveDOWN();
           moveRIGHT();
           moveUP();
          moveRIGHT();
           moveDOWN();
           moveRIGHT();
           moveUP();
          moveUP();
           moveLEFT();
           moveUP();
           moveRIGHT();
          break;
        case DOWN:
           moveLEFT();
           moveUP();
           moveRIGHT();
          moveUP();
           moveUP();
           moveLEFT();
           moveDOWN();
          moveLEFT();
           moveUP();
           moveLEFT();
           moveDOWN();
          moveDOWN();
           moveRIGHT();
           moveDOWN();
           moveLEFT();
          break;      
      } // /switch
    } else {
      switch(direction)
      { case LEFT:
          hilbertTLTR(level,UP);
          moveRIGHT();
          hilbertTLTR(level,LEFT);
          moveDOWN();
          hilbertTLTR(level,LEFT);
          moveLEFT();
          hilbertTLTR(level,DOWN);
          break;
        case RIGHT:
          hilbertTLTR(level,DOWN);
          moveLEFT();
          hilbertTLTR(level,RIGHT);
          moveUP();
          hilbertTLTR(level,RIGHT);
          moveRIGHT();
          hilbertTLTR(level,UP);
          break;
        case UP:
          hilbertTLTR(level,LEFT);
          moveDOWN();
          hilbertTLTR(level,UP);
          moveRIGHT();
          hilbertTLTR(level,UP);
          moveUP();
          hilbertTLTR(level,RIGHT);
          break;
        case DOWN:
          hilbertTLTR(level,RIGHT);
          moveUP();
          hilbertTLTR(level,DOWN);
          moveLEFT();
          hilbertTLTR(level,DOWN);
          moveDOWN();
          hilbertTLTR(level,LEFT);
          break;      
      } // /switch
    }
  }
}

/* modifizierte Hilbert-Kurve
   Start: Bottom-Right  Ende: Bottom-Left */
void CDither::hilbertBRBL(int level,int direction)
{ level--;
  if (!level)
  { switch(direction)
    { case LEFT:
        moveLEFT();
        moveUP();
        moveRIGHT();
        break;
      case RIGHT:
        moveRIGHT();
        moveDOWN();
        moveLEFT();
        break;
      case UP:
        moveUP();
        moveLEFT();
        moveDOWN();
        break;
      case DOWN:
        moveDOWN();
        moveRIGHT();
        moveUP();
        break;
    }
  } else {
    if (level==1) // little optimisation
    { switch(direction)
      { case LEFT:
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveRIGHT();    
            moveDOWN();
            moveRIGHT();
            moveUP();
          break;
        case RIGHT:
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          break;
        case UP:
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          break;
        case DOWN:
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          break;      
      } // /switch
    } else {
      switch(direction)
      { case LEFT:
          hilbertBRBL(level,UP);
          moveLEFT();
          hilbertBRBL(level,LEFT);
          moveUP();
          hilbertBRBL(level,LEFT);
          moveRIGHT();
          hilbertBRBL(level,DOWN);
          break;
        case RIGHT:
          hilbertBRBL(level,DOWN);
          moveRIGHT();
          hilbertBRBL(level,RIGHT);
          moveDOWN();
          hilbertBRBL(level,RIGHT);
          moveLEFT();
          hilbertBRBL(level,UP);
          break;
        case UP:
          hilbertBRBL(level,LEFT);
          moveUP();
          hilbertBRBL(level,UP);
          moveLEFT();
          hilbertBRBL(level,UP);
          moveDOWN();
          hilbertBRBL(level,RIGHT);
          break;
        case DOWN:
          hilbertBRBL(level,RIGHT);
          moveDOWN();
          hilbertBRBL(level,DOWN);
          moveRIGHT();
          hilbertBRBL(level,DOWN);
          moveUP();
          hilbertBRBL(level,LEFT);
          break;      
      } // /switch
    } // /else
  }
}

/* modifitierte Hilbert-Kurve
   Start: Top-Left  Ende: Bottom-Left */
void CDither::hilbertTLBL(int level,int direction) 
{ level--;
  if (!level)
  { switch(direction)
    { case LEFT:
        moveDOWN();
        moveRIGHT();
        moveUP();
        break;
      case RIGHT:
        moveUP();
        moveLEFT();
        moveDOWN();
        break;
      case UP:
        moveRIGHT();
        moveDOWN();
        moveLEFT();
        break;
      case DOWN:
        moveLEFT();
        moveUP();
        moveRIGHT();
        break;
    }
  } else {
    if (level==1)
    { switch(direction)
      { case LEFT:
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          break;
        case RIGHT:
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          break;
        case UP:
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          break;
        case DOWN:
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveRIGHT();
            moveDOWN();
            moveRIGHT();
            moveUP();
          break;      
      } // /switch
    } else {
      switch(direction)
      { case LEFT:
          hilbertTLBL(level,UP);
          moveDOWN();
          hilbertTLBL(level,LEFT);
          moveRIGHT();
          hilbertTLBL(level,LEFT);
          moveUP();
          hilbertTLBL(level,DOWN);
          break;
        case RIGHT:
          hilbertTLBL(level,DOWN);
          moveUP();
          hilbertTLBL(level,RIGHT);
          moveLEFT();
          hilbertTLBL(level,RIGHT);
          moveDOWN();
          hilbertTLBL(level,UP);
          break;
        case UP:
          hilbertTLBL(level,LEFT);
          moveRIGHT();
          hilbertTLBL(level,UP);
          moveDOWN();
          hilbertTLBL(level,UP);
          moveLEFT();
          hilbertTLBL(level,RIGHT);
          break;
        case DOWN:
          hilbertTLBL(level,RIGHT);
          moveLEFT();
          hilbertTLBL(level,DOWN);
          moveUP();
          hilbertTLBL(level,DOWN);
          moveRIGHT();
          hilbertTLBL(level,LEFT);
          break;      
      } // /switch
    } // /else
  }
}

/* modifitierte Hilbert-Kurve
   Start: Bottom-Right  Ende: Top-Right */
void CDither::hilbertBRTR(int level,int direction) 
{ level--;
  if (!level)
  { switch(direction)
    { case LEFT:
        moveUP();
        moveLEFT();
        moveDOWN();
        break;
      case RIGHT:
        moveDOWN();
        moveRIGHT();
        moveUP();
        break;
      case UP:
        moveLEFT();
        moveUP();
        moveRIGHT();
        break;
      case DOWN:
        moveRIGHT();
        moveDOWN();
        moveLEFT();
        break;
    }
  } else {
    if (level==1)
    { switch(direction)
      { case LEFT:
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          break;
        case RIGHT:
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          break;
        case UP:
            moveUP();
            moveLEFT();
            moveDOWN();
          moveLEFT();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveUP();
            moveLEFT();
            moveUP();
            moveRIGHT();
          moveRIGHT();
            moveDOWN();
            moveRIGHT();
            moveUP();
          break;
        case DOWN:
            moveDOWN();
            moveRIGHT();
            moveUP();
          moveRIGHT();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveDOWN();
            moveRIGHT();
            moveDOWN();
            moveLEFT();
          moveLEFT();
            moveUP();
            moveLEFT();
            moveDOWN();
          break;      
      } // /switch
    } else {
      switch(direction)
      { case LEFT:
          hilbertBRTR(level,UP);
          moveUP();
          hilbertBRTR(level,LEFT);
          moveLEFT();
          hilbertBRTR(level,LEFT);
          moveDOWN();
          hilbertBRTR(level,DOWN);
          break;
        case RIGHT:
          hilbertBRTR(level,DOWN);
          moveDOWN();
          hilbertBRTR(level,RIGHT);
          moveRIGHT();
          hilbertBRTR(level,RIGHT);
          moveUP();
          hilbertBRTR(level,UP);
          break;
        case UP:
          hilbertBRTR(level,LEFT);
          moveLEFT();
          hilbertBRTR(level,UP);
          moveUP();
          hilbertBRTR(level,UP);
          moveRIGHT();
          hilbertBRTR(level,RIGHT);
          break;
        case DOWN:
          hilbertBRTR(level,RIGHT);
          moveRIGHT();
          hilbertBRTR(level,DOWN);
          moveDOWN();
          hilbertBRTR(level,DOWN);
          moveLEFT();
          hilbertBRTR(level,LEFT);
          break;      
      } // /switch
    } // /else
  }
}

/* void HInit(BBitmap *bmp,BView *view)
{ ptu.bmp=bmp;
  ptu.doffset=0;
  ptu.rowbytes=bmp->BytesPerRow();
  ptu.view=view;
} */

// die Uebergabe erfolgt jeweils im oberen rechten Eck,
// Abgabe im unteren linken
// Rueckgabe: Anzahl der gleichgrossen Bloecke
int CDither::HSubControlOToRight(uint32 width,uint32 height)
{ if (width==1)
  { while (height-=1) moveDOWN(); // @@ ACHTUNG ! Aenderung
    return 0;
  } else {
    uint32 c=0;
    uint32 d=1; // Size
    uint32 tmp=width;
    if (tmp>height) tmp=height;
    while ((tmp=tmp>>1) != 0) 
    { c++;
      d=d<<1;
    }
    if ((d==width) && (c))
    { do
      { hilbertTLBL(c); // eine Reihe Hilbert-Curven bitte...
        if (height-=d) moveDOWN(); 
      } while (height);
      return 0;
    } else {
      if (height==d)
      { hilbertTLTR(c);
        moveRIGHT();
        return 1+HSubControlOToRight(width-d,d);
      }
      do
      { hilbertTLTR(c);
        moveRIGHT();
        HSubControlOToRight(width-d,d);
        moveDOWN();
        HSubControlOToRight(width-d,d);
        moveLEFT();
        hilbertBRBL(c);
        if (height!=2*d) { moveDOWN(); };
      } while (height-=2*d);
      return 0;
    }
  }
}

void CDither::HSubControlODown(uint32 width,uint32 height)
{ if (width==1)
  { while (height-=1) moveDOWN();
  } else {
    uint32 c=0;
    uint32 d=1; // Size
    uint32 tmp=width;
    if (tmp>height) tmp=height;
    while ((tmp=tmp>>1) != 0) 
    { c++;
      d=d<<1;
    }
    if (!c) // ein einziger Strich...
    { while (height-=1) moveDOWN();
      return;
    }
    if ((d==width) && (c))
    { do
      { hilbertTLBL(c); // eine Reihe Hilbert-Curven bitte...
        if (height-=d) moveDOWN(); 
      } while (height);
    } else {
      if (height==d)
      { hilbertTLTR(c-1);
        moveRIGHT();
        hilbertTLTR(c-1);
        moveRIGHT();
        HSubControlODown(width-d,d);
        moveLEFT();
        hilbertBRBL(c-1);
        moveLEFT();
        hilbertBRBL(c-1);
        return;
      }
      do
      { hilbertTLTR(c);
        moveRIGHT();
        HSubControlOToRight(width-d,d);
        moveDOWN();
        HSubControlOToRight(width-d,d);
        moveLEFT();
        hilbertBRBL(c);
        if (height!=2*d) { moveDOWN(); };
      } while (height-=2*d);
    }
  }
}

/* Diese Funktion fuellt den unteren linken Bereich, und
   geht dabei davon aus, das die hoehe (height & ~0x3) ist.
   width ist auf jeden Fall was quadratisches... 
   Start ist rechts unten - ende links unten */
void CDither::HSubControlOToLeft(uint32 width,uint32 height)
{ uint32 tmp=width;
  if (tmp>height) tmp=height;
  uint32 c=0;
  uint32 d=1;
  while ((tmp=tmp>>1) != 0) 
  { c++;
    d=d<<1;
  }  
  if (d==height) // Abschlusskolonne
  { do 
    { hilbertBRBL(c);
      if (width!=d) moveLEFT();
    } while (width-=d);
  } else { // Eine Reihe...
    do 
    { hilbertBRTR(c-1);
      moveUP();
      hilbertBRTR(c-1);
      moveUP();
      HSubControlOToLeft(d,height-d);
      moveDOWN();
      hilbertTLBL(c-1);
      moveDOWN();
      hilbertTLBL(c-1);
      if (width!=d) moveLEFT();
    } while (width-=d);
  }
}

/* Diese Funktion fuellt in dem seltenen Fall, in dem
   die Breite genau eine Potenzzahl einen Teil des
   Bereichs auf... die Hoehe ist eine Potenzzahl */
void CDither::HSubControlUDownPotSub(uint32 d,uint32 c,uint32 width)
{ if (d==width) // Abschlusskolonne
  { do 
    { hilbertTLBL(c);
      if (width!=d) moveDOWN();
    } while (width-=d);
  } else { // Eine Reihe...
    uint32 tmp=width;
    while (tmp>d)
    { hilbertTLTR(c-1);
      moveRIGHT();
      hilbertTLTR(c-1);
      moveRIGHT();
      tmp-=d;
    }
    hilbertTLBL(c);
    tmp=width;
    while (tmp>d)
    { moveLEFT();
      hilbertBRBL(c-1);
      moveLEFT();
      hilbertBRBL(c-1);
      tmp-=d;
    }
  }
}

/* Diese Funktion fuellt in dem seltenen Fall, in dem
   die Breite genau eine Potenzzahl ist den unteren
   Bereich auf */
void CDither::HSubControlUDownPot(uint32 width,uint32 height)
{ while (height>3)
  { uint32 tmp=width;
    if (tmp>height) tmp=height;
    uint32 c=0;
    uint32 d=1;
    while ((tmp=tmp>>1) != 0) 
    { c++;
      d=d<<1;
    }
    HSubControlUDownPotSub(d,c,width);
    height-=d;  
    if (height>3) moveDOWN();
  }
}
   
void CDither::HilbertController(uint32 width,uint32 height)
{ if ((!width) || (!height)) return;
  if ((width==1) || (height==1))
  { if (width==1)
    { if (height==1)
      { moveRIGHT(); // 1 Pixel...
      } else { // width==1
        while (height-=1) moveDOWN();
        moveDOWN();
      }
    } else { // height==1
      while (width-=1) moveRIGHT();
      moveRIGHT();
    }
    return;
  }
  bool lastdown=true;
  uint32 r=width;
  if (r>height) r=height;
  uint32 c=0;
  uint32 d=1;
  uint32 tmp=r;
  while ((tmp=tmp>>1) != 0) 
  { c++;
    d=d<<1;
  }  
  uint32 rest=height-d;
  if (width-d) // Ist da rechts ein Rand?
  { hilbertTLTR(c);
    moveRIGHT();
    int danz=HSubControlOToRight(width-d,d);
    if (height-d) // ist nun unten noch ein Rand?
    { uint32 d2=d+danz*d;
      uint32 _rest=height-d;
      if (_rest>1) moveDOWN();
      if (_rest>3)
      { while (_rest>3) 
        { tmp=_rest;
          uint32 c=0;
          uint32 dr=1;
          while ((tmp=tmp>>1) != 0) 
          { c++;
            dr=dr<<1;
          }
          HSubControlODown(width-d2,dr);
          _rest=_rest-dr;    
          if (_rest>3) moveDOWN();  
        }
        rest=_rest;
        // nun den Bereich unten links fuellen...
        if (height-d>3)
        { moveLEFT();
          HSubControlOToLeft(d+danz*d,height-d-rest);
        }
      } else { // es gab keinen wirklichen unteren Rand
        if (_rest<2) moveDOWN();
        RESETandmoveLeftBorder();
        lastdown=false;
      }
    }
  } else { // kein rechter Rand!
    hilbertTLBL(c);
    if (height-d>3) // unterer Rand? 
    { moveDOWN();
      HSubControlUDownPot(d,height-d);
    } 
  }
  /* in jedem Fall steht der "Pointer" jetzt links unten...
     ist nun noch ein Rand zu zeichnen? */
  tmp=width+1;
  if (lastdown) moveDOWN(); // fuellt das letzte Pixel!
  switch (rest & 3)
  { case 3:
      { do
        { moveDOWN();
          moveDOWN();
          moveRIGHT();
          moveUP();
          moveUP();
          moveRIGHT();
          tmp-=2;
        } while (tmp>2);
        if (!(tmp & 1))
        { moveDOWN();
          moveDOWN();
          moveRIGHT();
        }
      }
      break;
    case 2:
      { do
        { moveDOWN();
          moveRIGHT();
          moveUP();
          moveRIGHT();
          tmp-=2;
        } while (tmp>2);
        if (!(tmp & 1))
        { moveDOWN();
          moveRIGHT();
        }
      }
      break;
    case 1:
      { while (tmp-=1) moveRIGHT();
      }
  }
}

// --------------------------------------------------------------------------------
