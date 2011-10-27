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

/* ABlur.cpp a fast blur implementation by C. Kohlert c@kohlert.de */
#include "pch.h"
#include "ABlur.h"
#include <translation/TranslationUtils.h>

/* size ist die breite der "Matrix", deckung bezieht
   sich auf die helligkeit die ein Pixel hinterher
   insgesamt haben soll... (fuer heller/dunkler-Effekte) */
void CBlur::SetSpreadSize(int32 size,rgb_color _outside,float deckung)
{ if (size<2) size=2;
  uint8 count=0; // possible "Height" of a Color-Component during Blur
  if (size & 1) size++; // ungerade matizen währen ein ungleichgewicht beim 
  // Auf- und abbau der Flanke
  do {
    lubuffer[count].reduce=(uint16)((float)((float)count*deckung*256*2)/size);
    lubuffer[count].addon=lubuffer[count].reduce/(size/2);
    count++;
  } while (count!=0); // full 256 entrys ;) 
  outside=_outside;
  msize=size;
}

// little endian pixel set functions
rgb_color CBlur::GetPixelRGB32(uint8 **ppos)
{ rgb_color ret;
  uint32 *v=(uint32*)*ppos;
  ret.red=(*v)>>16;
  ret.green=(*v)>>8;
  ret.blue=(*v);
  (*ppos)+=4;
  return ret;
}

// 15 und 16 Bits sind mangels Bedarf nicht ausprogrammiert!
rgb_color CBlur::GetPixelRGB15(uint8 **ppos)
{ rgb_color ret={ 0, 0, 0 };	// init to quiet compiler
//  uint16 *v=(uint16*)*ppos;
  (*ppos)+=2;
  return ret;
}

rgb_color CBlur::GetPixelRGB16(uint8 **ppos)
{ rgb_color ret={ 0, 0, 0 };	// init to quiet compiler
//  uint16 *v=(uint16*)*ppos;
  (*ppos)+=2;
  return ret;
}

// big endian pixel set functions
rgb_color CBlur::GetPixelRGB32_Big(uint8 **ppos)
{ rgb_color ret;
  uint32 *v=(uint32*)*ppos;
  ret.red=(*v)>>8;
  ret.green=(*v)>>16;
  ret.blue=(*v)>>24;
  (*ppos)+=4;
  return ret;
}

rgb_color CBlur::GetPixelRGB15_Big(uint8 **ppos)
{ rgb_color ret={ 0, 0, 0 };   // init to quiet compiler
//  uint16 *v=(uint16*)*ppos;
  (*ppos)+=2;
  return ret;
}

rgb_color CBlur::GetPixelRGB16_Big(uint8 **ppos)
{ rgb_color ret={ 0, 0, 0 };  // init to quiet compiler
//  uint16 *v=(uint16*)*ppos;
  (*ppos)+=2;
  return ret;
}

/* Fuer alle die es unbedingt wissen wollen: Hier der VERSUCH einer
   Erklaerung des Algorithmus:
   Die Funktion SetSpreadSize baut einen Lookup-Table fuer die von
   der Matrixgroesse abhaengende Steigung jedes Punktes auf. Reduce
   ist dabei der (ausserhalb der Funktion eigentlich obsolete) Wert,
   den ein Pixel im Zenit des Dreiecks haette. Aus einer Pixelkomponente
   wird ein Dreieck, dessen Volumen dem Energiewert des urspruenglichen
   Pixels entspricht. Dieses Dreieck wird mittels der Steigung, dem
   addon pro pixel aufgebaut.
   Der Blur-Algorithmus basiert nun darauf, das aus allen Punkten
   der Bitmap (zunaechst horizontal dann vertikal) ein Dreieck wird,
   dessen Hoehe lubuffer[pixelkomponente].reduce ist, und dessen
   Grundflaeche matrixsize ist. 
   value entspricht im algorithmus der Energiewert des aktuellen
   Pixels. Dieser jedoch ist die Summe aller aktuellen Werte
   der Dreiecke an dieser Position. Ein einzelnes Dreieck koennte
   man aufbauen, indem man bei der ansteigenden Flanke 
   lubuffer[pixel].addon in eine Variable matrixsize/2 mal hinein-
   gibt und nach erreichen der Spitze seinen negativen Wert 
   abspeichert. 
   Da die einzelnen addons kleiner als ein Byte sind - ist die
   verwaltung der Steigung 8.8 fixkomma und haelt nur ca eine
   Matrixgroesse von 150 aus bevor gravierende Fehler aufgrund
   der ungenauigkeit passieren. Das in diesem Fall die Bitmap
   auch Fixkomma ist muesste nicht sein und ist eher hinderlich.
   Den aufbau aller Dreiecke gleichzeitig uebernimmt die Variable
   subtractor. Sie enthaelt alle auf und absteigenden Steigungen
   aller derzeitig aktiver Dreiecke. Mit diesem Steigungsmischmasch
   wird der Wert value gespeist und enthaelt somit immer die Energie
   des aktuellen Pixels.
   Damit der Aufbau des Dreiecks vorausschauend klappt wird immer
   das Pixel das 1/2 matrixsize vor dem aktuellen Pixel liegt in
   eine Steigung uebersetzt und in ein Array uebertragen das info_band
   heisst. In diesem Array liegt 1/2 matrixsize spaeter derselbe 
   Wert, um die damals steigende Flanke zu einer abfallenden zu
   machen, und schliesslich 1/2 matrixsize danach das Dreieck 
   vollends abzuschliessen.
   Damit die Positionierung der Punkte ohne groessen Rechenaufwand
   klappt verweisen desubtorpos, addatorpos und destructorpos
   jeweils im richtigen abstand auf das info_band.
   Bilder die kleiner als die Matrixgroesse sind koennten auch
   zu einem Problem fuehren?! ...
   Hier noch ein Ascii-Bild, das ungefaehr die Zusammenhaenge
   veranschaulicht:
   
                      |
       addator       /|\  desubtor
                    / | \
                   /  |  \
                  /   |   \
                 /    |    \ hier erste speisung des subtractors
    destructor  /     |     \
   -------------- matrixsize -------------------------
  / DATA      /  value      /   DATA   DATA   DATA   /
  ---------------------------------------------------
 | info_band      info_band     info_band    ...    |
 ----------------------------------------------------
*/
// dest MUSS eine 32Bit-BBitmap sein!
status_t CBlur::BlurH(BBitmap *source, BBitmap *dest)
{ if ((!source) || (!dest)) return B_BAD_VALUE;
  GetPixelFunc get_pixel=NULL;
  switch (source->ColorSpace())
  { case B_RGB32: // 32 Bit-Blur...
      { int32 bmpbprow=source->BytesPerRow();// DWord
        rgb_color* bmpbits=(rgb_color*)source->Bits();
        uint32 cfbprow=dest->BytesPerRow();  // DWord
        rgb_color* cfbits=(rgb_color*)dest->Bits();
        uint32 hi=(uint32)source->Bounds().Height();
        uint32 cx=(uint32)source->Bounds().Width();
        rgb_color* usebmp=bmpbits;
        rgb_color* usecfb=cfbits;
        sbig_color subtractor;
        uint hmsize=msize/2; // round(msize/2) size Flanke1
        big_color *info_band=new big_color[cx+msize+hmsize]; // info's
        big_color *desubtorpos;
        big_color *destructorpos;
        big_color *addatorpos;
        big_color value;
        for (uint a=0;a<(uint)msize;a++) 
        { info_band[a].blue=lubuffer[outside.blue].addon;
          info_band[a].green=lubuffer[outside.green].addon;
          info_band[a].red=lubuffer[outside.red].addon;
        }
        while (hi--) // eine Zeile
        { // initial stuff
          destructorpos=info_band;
          addatorpos=info_band+hmsize;
          desubtorpos=info_band+msize;
          // Vorlauf initialisieren
          subtractor.blue=subtractor.green=subtractor.red=0;
          value.blue=outside.blue<<8;
          value.red=outside.red<<8;
          value.green=outside.green<<8;
          int dox=hmsize; // erste halbe flanke
          if (dox>0) while (dox--)
          { rgb_color pixel=*(usebmp++);
            // Blue
            subtractor.blue+=desubtorpos->blue=lubuffer[pixel.blue].addon; // neuer Wert
            subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
            subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
            value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
            // Green
            subtractor.green+=desubtorpos->green=lubuffer[pixel.green].addon; // neuer Wert
            subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
            subtractor.green+=destructorpos->green; // negative Flanke nullen!
            value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
            // Red
            subtractor.red+=desubtorpos->red=lubuffer[pixel.red].addon; // neuer Wert
            subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
            subtractor.red+=destructorpos->red; // negative Flanke nullen!
            value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
            // Array-fitting
            desubtorpos++;
            addatorpos++;
            destructorpos++; 
          }
          dox=cx-hmsize;
          if (dox>0) while (dox--)
          { rgb_color pixel=*(usebmp++);
            // Blue
            subtractor.blue+=desubtorpos->blue=lubuffer[pixel.blue].addon; // neuer Wert
            subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
            subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
            value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
            rgb_color result_pixel;
            result_pixel.blue=value.blue>>8;
            // Green
            subtractor.green+=desubtorpos->green=lubuffer[pixel.green].addon; // neuer Wert
            subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
            subtractor.green+=destructorpos->green; // negative Flanke nullen!
            value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
            result_pixel.green=value.green>>8;
            // Red
            subtractor.red+=desubtorpos->red=lubuffer[pixel.red].addon; // neuer Wert
            subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
            subtractor.red+=destructorpos->red; // negative Flanke nullen!
            value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
            result_pixel.red=value.red>>8;
            *(usecfb++)=result_pixel;
            // Array-fitting
            desubtorpos++;
            addatorpos++;
            destructorpos++; 
          }
          // Ende Flanke 2
          dox=hmsize;
          if (dox>0) while (dox--)
          { // Blue
            subtractor.blue+=desubtorpos->blue=lubuffer[outside.blue].addon; // neuer Wert
            subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
            subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
            value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
            rgb_color result_pixel;
            result_pixel.blue=value.blue>>8;
            // Green
            subtractor.green+=desubtorpos->green=lubuffer[outside.green].addon; // neuer Wert
            subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
            subtractor.green+=destructorpos->green; // negative Flanke nullen!
            value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
            result_pixel.green=value.green>>8;
            // Red
            subtractor.red+=desubtorpos->red=lubuffer[outside.red].addon; // neuer Wert
            subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
            subtractor.red+=destructorpos->red; // negative Flanke nullen!
            value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
            result_pixel.red=value.red>>8;
            *(usecfb++)=result_pixel;
            // Rest
            desubtorpos++;
            addatorpos++;
            destructorpos++; 
          }
          // next Row-Fittings...
          bmpbits=(rgb_color*)((uint8*)bmpbits+bmpbprow);
          usebmp =bmpbits;
          cfbits =(rgb_color*)((uint8*)cfbits+cfbprow);
          usecfb=cfbits;
          cx=(uint32)source->Bounds().Width();
        }
        delete info_band;
        return B_OK;
      }
      break;
	/* Nun wirds heiss. Die Bitmap liegt nicht in 32Bit vor,
	   es wird der gleiche Algorithmus mit dem Aufruf einer
	   GetPixel-Funktion verbunden (wird im Taskmanager nicht
	   verwendet - nur zu Testzwecken!) */ 
	case B_RGBA32:
	  get_pixel = &GetPixelRGB32;
	  break;
	case B_RGBA15:
	case B_RGB15:
	  get_pixel = &GetPixelRGB15;
	  break;
	case B_RGB16_BIG:
	case B_RGB16:
	  get_pixel = &GetPixelRGB16;
	  break;
	case B_RGB32_BIG:
	case B_RGBA32_BIG:
	  get_pixel = &GetPixelRGB32_Big;
	  break;
	case B_RGB15_BIG:
	case B_RGBA15_BIG:
	  get_pixel = &GetPixelRGB15_Big;
	  break;
    default:
      return B_BAD_TYPE;
  }
  // das Ganze jetzt noch mal in Gelb:
  // ab hier : ungetestet !

  int32 bmpbprow=source->BytesPerRow();
  uint8* bmpbits=(uint8*)source->Bits();
  uint32 cfbprow=dest->BytesPerRow();
  uint8* cfbits=(uint8*)dest->Bits();
  uint32 hi=(uint32)source->Bounds().Height();
  uint32 cx=(uint32)source->Bounds().Width();
  uint8 *usebmpb=bmpbits;
  uint8 *usecfb=cfbits;
  sbig_color subtractor;
  uint hmsize=msize/2; // round(msize/2) size Flanke1
  big_color *info_band=new big_color[cx+msize+hmsize]; // info's
  big_color *desubtorpos;
  big_color *destructorpos;
  big_color *addatorpos;
  big_color value;
  for (uint a=0;a<(uint)msize;a++) 
  { info_band[a].blue=lubuffer[outside.blue].addon;
    info_band[a].green=lubuffer[outside.green].addon;
    info_band[a].red=lubuffer[outside.red].addon;
  }
  while (hi--) // eine Zeile
  { // initial stuff
    destructorpos=info_band;
    addatorpos=info_band+hmsize;
    desubtorpos=info_band+msize;
    // Vorlauf initialisieren
    subtractor.blue=subtractor.green=subtractor.red=0;
    value.blue=outside.blue<<8;
    value.red=outside.red<<8;
    value.green=outside.green<<8;
    int dox=hmsize; // erste halbe flanke
    if (dox>0) while (dox--)
    { rgb_color incoming=get_pixel(&usebmpb);
      // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[incoming.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[incoming.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[incoming.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      // Array-fitting
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    dox=cx-hmsize;
    if (dox>0) while (dox--)
    { rgb_color incoming=get_pixel(&usebmpb);
      // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[incoming.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      uint8 blue=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[incoming.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      uint8 green=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[incoming.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      *(usecfb++)=value.red>>8;
      // Array-fitting
      *(usecfb++)=green;
      *(usecfb++)=blue;
      usecfb++; // Alpha-Pixel
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // Ende Flanke 2
    dox=hmsize;
    if (dox>0) while (dox--)
    { // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[outside.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      uint8 blue=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[outside.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      uint8 green=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[outside.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      *(usecfb++)=value.red>>8;
      // Array-fitting
      *(usecfb++)=green;
      *(usecfb++)=blue;
      usecfb++; // Alpha Pixel
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // next Row-Fittings...
    bmpbits+=bmpbprow;
    usebmpb=bmpbits;
    cfbits+=cfbprow;
    usecfb=cfbits;
    cx=(uint32)source->Bounds().Width();
  }
  delete info_band;
  return B_OK; 
}

/* Diese Funktion bluert vertikal von einer BBitmap aus... 
   dest MUSS eine 32Bit-BBitmap sein! */
status_t CBlur::BlurVB(BBitmap *source,BBitmap *dest)
{ if (msize<2) return B_OK;
  uint32 cfbprow=dest->BytesPerRow(); 
  uint8 *cfbits=(uint8*)dest->Bits();
  int32 bmpbprow=source->BytesPerRow();
  uint8 *bmpbits=(uint8*)source->Bits();
  uint8 *usebmpb=bmpbits;
  uint wi=(uint)dest->Bounds().Width();
  uint32 cy=(uint32)dest->Bounds().Height();
  sbig_color subtractor;
  uint hmsize=(uint)((float)((float)msize+0.5)/2); // round(msize/2) size Flanke1
  big_color *info_band=new big_color[cy+msize+hmsize]; // info's
  big_color *desubtorpos;
  big_color *destructorpos;
  big_color *addatorpos;
  big_color value;
  for (uint a=0;a<(uint)msize;a++) 
  { info_band[a].blue=lubuffer[outside.blue].addon;
    info_band[a].green=lubuffer[outside.green].addon;
    info_band[a].red=lubuffer[outside.red].addon;
  }
  uint8 *usecfbdest=cfbits;
  while (wi--) // eine Reihe
  { // initial stuff
    destructorpos=info_band;
    addatorpos=info_band+hmsize;
    desubtorpos=info_band+msize;
    // Vorlauf initialisieren
    subtractor.blue=subtractor.green=subtractor.red=0;
    value.blue=outside.blue<<8;
    value.red=outside.red<<8;
    value.green=outside.green<<8;
    int doy=hmsize;
    if (doy>0) while (doy--)
    { // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[*(usebmpb++)].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[*(usebmpb++)].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[*usebmpb].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      // Array-fitting
      usebmpb+=bmpbprow-2;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    doy=cy-hmsize;
    if (doy>0) while (doy--)
    { // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[*(usebmpb++)].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      *(usecfbdest++)=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[*(usebmpb++)].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      *(usecfbdest++)=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[*usebmpb].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      *usecfbdest=value.red>>8;      
      // Array-fitting
      usebmpb+=bmpbprow-2;      
      usecfbdest+=cfbprow-2;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // Ende Flankenteil 1
    doy=hmsize;
    if (doy>0) while (doy--)
    { // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[outside.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      *(usecfbdest++)=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[outside.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      *(usecfbdest++)=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[outside.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      *usecfbdest=value.red>>8;
      // Array-fitting
      usecfbdest+=cfbprow-2;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // next Row-Fittings...
    bmpbits+=4;
    usebmpb=bmpbits;
    cfbits+=3;
    usecfbdest=cfbits;
    cy=(uint32)dest->Bounds().Height();
  }
  delete info_band;
  return B_OK;
}

/* Dies Funktion bluert vertikal die Input-Bitmap (32 Bit) in sich selbst */
status_t CBlur::BlurV(BBitmap *sdest)
{ if (msize<2) return B_OK;
  uint32 cfbprow=sdest->BytesPerRow()/sizeof(rgb_color); // DWord
  rgb_color* cfbits=(rgb_color*)sdest->Bits();
  uint wi=(uint)sdest->Bounds().Width();
  uint32 cy=(uint32)sdest->Bounds().Height();
  rgb_color* usecfb=cfbits;
  sbig_color subtractor;
  uint hmsize=(uint)((float)((float)msize+0.5)/2); // round(msize/2) size Flanke1
  big_color *info_band=new big_color[cy+msize+hmsize]; // info's
  big_color *desubtorpos;
  big_color *destructorpos;
  big_color *addatorpos;
  big_color value;
  for (uint a=0;a<(uint)msize;a++) 
  { info_band[a].blue=lubuffer[outside.blue].addon;
    info_band[a].green=lubuffer[outside.green].addon;
    info_band[a].red=lubuffer[outside.red].addon;
  }
  rgb_color *usecfbdest;
  while (wi--) // eine Reihe
  { // initial stuff
    destructorpos=info_band;
    addatorpos=info_band+hmsize;
    desubtorpos=info_band+msize;
    // Vorlauf initialisieren
    subtractor.blue=subtractor.green=subtractor.red=0;
    value.blue=outside.blue<<8;
    value.red=outside.red<<8;
    value.green=outside.green<<8;
    int doy=hmsize;
    usecfbdest=usecfb;
    if (doy>0) while (doy--)
    { // Blue
      rgb_color pixel=*usecfb;
      subtractor.blue+=desubtorpos->blue=lubuffer[pixel.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[pixel.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[pixel.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      // Array-fitting
      usecfb+=cfbprow;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    doy=cy-hmsize;
    if (doy>0) while (doy--)
    { rgb_color pixel=*usecfb;
      // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[pixel.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      pixel.blue=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[pixel.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      pixel.green=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[pixel.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      pixel.red=value.red>>8;      
      // Array-fitting
      *usecfbdest=pixel;
      usecfb+=cfbprow;
      usecfbdest+=cfbprow;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // Ende Flankenteil 1
    doy=hmsize;
    if (doy>0) while (doy--)
    { rgb_color pixel;
      // Blue
      subtractor.blue+=desubtorpos->blue=lubuffer[outside.blue].addon; // neuer Wert
      subtractor.blue-=addatorpos->blue*2; // wegnehmen, abziehen
      subtractor.blue+=destructorpos->blue; // negative Flanke nullen!
      value.blue+=subtractor.blue; // quasi alle Aenderungen aktivieren
      pixel.blue=value.blue>>8;
      // Green
      subtractor.green+=desubtorpos->green=lubuffer[outside.green].addon; // neuer Wert
      subtractor.green-=addatorpos->green*2; // wegnehmen, abziehen
      subtractor.green+=destructorpos->green; // negative Flanke nullen!
      value.green+=subtractor.green; // quasi alle Aenderungen aktivieren
      pixel.green=value.green>>8;
      // Red
      subtractor.red+=desubtorpos->red=lubuffer[outside.red].addon; // neuer Wert
      subtractor.red-=addatorpos->red*2; // wegnehmen, abziehen
      subtractor.red+=destructorpos->red; // negative Flanke nullen!
      value.red+=subtractor.red; // quasi alle Aenderungen aktivieren
      pixel.red=value.red>>8;
      // Array-fitting
      *usecfbdest=pixel;      
      usecfbdest+=cfbprow;
      desubtorpos++;
      addatorpos++;
      destructorpos++; 
    }
    // next Row-Fittings...
    cfbits++;
    usecfb=cfbits;
    cy=(uint)sdest->Bounds().Height();
  }
  delete info_band;
  return B_OK;
}

status_t CBlur::BlurAll(BBitmap *source, BBitmap *dest)
{ if (BlurH(source,dest)==B_OK) return BlurV(dest);
  else return B_ERROR;
}

//--------------------------------
/* the Alpha-Algorithm use this formula:
   dest = source1 * alpha + source2 * (1-alpha)
   dest = source1 * alpha + source2*1 - source2*alpha
   dest = (source1-source2)*alpha + source2
   finalColor = destColor + ( sourceColor - destColor) * alpha
*/ 
/* bmp2 wird in bmp1 hineingealphed ! */   
void CAlphaBlend::Blend(BBitmap *bmp1,BBitmap *bmp2,float mix)
{ if (mix==0.0) return;
  unsigned int width=(unsigned int)bmp1->Bounds().Width();
  if ((unsigned int)bmp2->Bounds().Width()<width) 
    width=(unsigned int)bmp2->Bounds().Width();
  unsigned int height=(unsigned int)bmp1->Bounds().Height();
  if ((unsigned int)bmp2->Bounds().Height()<height) 
    height=(unsigned int)bmp2->Bounds().Height();
  rgb_color *dest=(rgb_color*)bmp1->Bits();
  if ((!dest) || (!width) || (!height)) return; // Fehler!
  rgb_color *source=(rgb_color*)bmp2->Bits();
  uint32 destroww=bmp1->BytesPerRow()/4;   // DWord
  uint32 sourceroww=bmp2->BytesPerRow()/4; // DWord
  uint16 fixedalpha=(uint16)(mix*256);
  rgb_color *usource=source;
  rgb_color *udest=dest;
  unsigned int cx;
  while (height--)
  { cx=width;
    while (cx--)
    { rgb_color spixel=*(usource++);
      register rgb_color dpixel=*udest;
      // R
      register uint16 t2=(spixel.red-dpixel.red)*fixedalpha;
      dpixel.red=dpixel.red+(t2>>8); 
      // G
      t2=(spixel.green-dpixel.green)*fixedalpha;
      dpixel.green=dpixel.green+(t2>>8); 
      // B
      t2=(spixel.blue-dpixel.blue)*fixedalpha;
      dpixel.blue=dpixel.blue+(t2>>8); 
      *(udest++)=dpixel;
    }
    dest+=destroww;
    udest=dest;
    source+=sourceroww;
    usource=source;
  }
}

//--------------------------------
CMixedEffect::CMixedEffect(BBitmap *bmp1,BBitmap *bmp2,int32 _maxsize)
{ outcolor.red=outcolor.green=outcolor.blue=0xFF; // White
  last=100;
  cfbmp1=NULL;
  cfbmp2=NULL;
  bmp1store=bmp1;
  bmp2store=bmp2;  
  maxsize=_maxsize;
}

CMixedEffect::~CMixedEffect()
{ if (bmp1store) delete bmp1store;
  if (bmp1store) delete bmp2store;
  if (cfbmp2) delete cfbmp2;
}

BBitmap *CMixedEffect::GetEffect(float mixed)
{ if (mixed<0.0) mixed=0.0;
  if (mixed>1.0) mixed=1.0;
  last=mixed;
  cfbmp1=new BBitmap(BRect(0.0,0.0,bmp1store->Bounds().Width(),bmp1store->Bounds().Height()),B_RGB32);
  cb.SetSpreadSize((float)8*maxsize*mixed,outcolor);
//  cb.BlurAll(bmp1store,cfbmp1);

  if (cb.BlurH(bmp1store,cfbmp1)==B_OK) 
  { cb.SetSpreadSize((float)maxsize*mixed,outcolor);
    cb.BlurV(cfbmp1);
  }

  if (cfbmp2) // diese Optimierung bringt kaum etwas :(
  { BRect rect(0.0,0.0,bmp2store->Bounds().Width(),bmp2store->Bounds().Height());
    if (cfbmp2->Bounds()!=rect)
    { delete cfbmp2;
      cfbmp2=new BBitmap(rect,B_RGB32);
    }
  } else {
    BRect rect(0.0,0.0,bmp2store->Bounds().Width(),bmp2store->Bounds().Height());
    cfbmp2=new BBitmap(rect,B_RGB32);  
  }
  cb.SetSpreadSize((float)1*maxsize*(1.0-mixed),outcolor);
//  cb.BlurAll(bmp2store,cfbmp2);

  if (cb.BlurH(bmp2store,cfbmp2)==B_OK) 
  { cb.SetSpreadSize((float)maxsize*(1.0-mixed),outcolor);
    cb.BlurV(cfbmp2);
  }

  CAlphaBlend::Blend(cfbmp1,cfbmp2,mixed);
  return cfbmp1;
}

//-------------- ENTE ------------------
