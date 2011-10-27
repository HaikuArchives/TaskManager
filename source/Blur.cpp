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
 
#include "commondefines.h"
#include "pch.h"
#include "Blur.h"
#include "PointerList.h"

#include "my_assert.h"

// don't compile, if fast blur is used.
#ifndef _TMGR_FULLBLUR

// SupportedInputColorSpace
// Returns true, if the passed color_space is a valid color_space for
// the source bitmap of 'blur'
bool CBlur::SupportedInputColorSpace(color_space in_color_space)
{
	switch(in_color_space) {
		case B_GRAY1:
		case B_RGB32:
			return true;
		default:
			return false;
	}
}

// SupportedOutputColorSpace
// Returns true, if the passed color_space is a valid color_space for
// the destination bitmap of 'blur'
bool CBlur::SupportedOutputColorSpace(color_space out_color_space)
{
	switch(out_color_space) {
		case B_RGB32:
		case B_RGBA32:
		case B_RGB15:
		case B_RGBA15:
		case B_RGB16:
		case B_RGB32_BIG:
		case B_RGBA32_BIG:
		case B_RGB15_BIG:
		case B_RGBA15_BIG:
		case B_RGB16_BIG:	
			return true;
		default:
			return false;
	}
}

// Blur (1)
// Blurs the source bitmap and stored the result in dest. The area outside the
// rectangle (mx/2,my/2) - (width-mx/2, height-my/2) of the destionation bitmap
// remains unchanged. The source and destionation bitmaps must be of the
// same size. Their color_space can differ.
status_t CBlur::Blur(BBitmap *source, BBitmap *dest, int32 mx, int32 my)
{
	if(mx <= 0 || my <= 0)
		return B_BAD_VALUE;

	int32 sx = mx/2;
	int32 sy = my/2;
	
	int32 ex = (int32)source->Bounds().Width() - sx + (mx%2 == 0 ? 2 : 1);
	int32 ey = (int32)source->Bounds().Height() - sy + (my%2 == 0 ? 2 : 1);

	uchar *s_bits = (uchar *)source->Bits();
	uchar *r_bits = (uchar *)dest->Bits(); 

	int32 s_bytes_per_row = source->BytesPerRow();
	int32 r_bytes_per_row = dest->BytesPerRow();

	SetPixelFunc set_pixel = PixelFunc(dest->ColorSpace());

	if(set_pixel == NULL) {
		// Invalid destination colorspace
		return B_BAD_VALUE;
	}

	switch(source->ColorSpace()) {
		case B_GRAY1:
			return BlurGray1(
				s_bits,				// source buffer info
				s_bytes_per_row,
				r_bits,				// dest buffer info
				r_bytes_per_row,
				set_pixel,			// set pixel function
				0,					// start point in dest
				0,
				sx,					// blur area
				sy,
				ex,
				ey,
				mx,					// matrix size
				my
			);
		case B_RGB32:
			return BlurRGB32(
				s_bits,				// source buffer info
				s_bytes_per_row,
				r_bits,				// dest buffer info
				r_bytes_per_row,
				set_pixel,			// set pixel function
				0,					// start point in dest
				0,
				sx,					// blur area
				sy,
				ex,
				ey,
				mx,					// matrix size
				my
			);
		default:
			// Invalid source colorspace
			return B_BAD_VALUE;
	}
}

// Blur (2)
// Can be used to blur directly into the frambuffer (using BDirectWindow).
// The clipping_rects in 'rects' can be created using the 'ClipRects' function
// of this class. Otherwise the size of the area which isn't blured must
// be considered during the creation of the clipping rects.
status_t CBlur::Blur(
	BBitmap *source, 			// source bitmap
	uchar *r_bits, 				// destimation buffer
	int32 r_bytes_per_row,		// bytes per row of dest buffer
	color_space r_pixel_format,	// color space of dest buffer
	clipping_rect *dest_rect,	// destination of the blur operation (in screen coordinates)
	clipping_rect *rects,		// visible region of dest buffer (in screen coordinates)
	int32 num_rects,			// number of clipping_rect structs in 'rects'
	int32 mx,					// size of blur matrix
	int32 my
)
{
	if(mx <= 0 || my <= 0)
		return B_BAD_VALUE;

	uchar *s_bits = (uchar *)source->Bits();

	int32 s_bytes_per_row = source->BytesPerRow();

	SetPixelFunc set_pixel = PixelFunc(r_pixel_format);

	if(set_pixel == NULL) {
		// Invalid destination colorspace
		return B_BAD_VALUE;
	}

	if(!SupportedInputColorSpace(source->ColorSpace())) {
		// Invalid source colorspace
		return B_BAD_VALUE;
	}
	
	for(int rect=0 ; rect<num_rects ; rect++) {
		clipping_rect *cur_rect = &rects[rect];
	
		switch(source->ColorSpace()) {
			case B_GRAY1:
				BlurRGB32(
					s_bits,
					s_bytes_per_row,
					r_bits,
					r_bytes_per_row,
					set_pixel,
					dest_rect->left,						// dest_x
					dest_rect->top,							// dest_y
					cur_rect->left - dest_rect->left,		// sx
					cur_rect->top - dest_rect->top,			// sy
					cur_rect->right - dest_rect->left + 1,	// ex
					cur_rect->bottom - dest_rect->top + 1,	// ey
					mx,
					my);
					
				break;
			case B_RGB32:
				BlurRGB32(
					s_bits,
					s_bytes_per_row,
					r_bits,
					r_bytes_per_row,
					set_pixel,
					dest_rect->left,						// dest_x
					dest_rect->top,							// dest_y
					cur_rect->left - dest_rect->left,		// sx
					cur_rect->top - dest_rect->top,			// sy
					cur_rect->right - dest_rect->left + 1,	// ex
					cur_rect->bottom - dest_rect->top + 1,	// ey
					mx,
					my);
					
				break;
			default:
				// to quiet compiler...
				break;
		}
	}
	
	return B_OK;
}

// ClipRects
// Creates an array of clipping rects for Blur() from the data in the
// direct_buffer_info struct. Because the blur operation leaves areas which
// can't be blured untouched (the borders of the destination area outside
// (mx/2, my/2) - (w-mx/2, h-my/2) remain untouched), these areas must
// be considered during the clipping. This method is faster than
// creating the correct clipping region every time it's needed.
// The returned array belongs to the caller. The number of structs
// in the array is retunred in num_rects.
clipping_rect *CBlur::ClipRects(
	const clipping_rect *rects,		// visible area (see direct_buffer_info::clip_list)
	int32 &num_rects,				// [in][out] number of rectangles
	BRect dest_rect,				// destination of blur operation (in screen coordinates)
	int32 mx,						// maximum size of blur matrix
	int32 my
)
{
	int32 sx = mx/2;
	int32 sy = my/2;
	
	int32 ex = (int32)dest_rect.Width() - sx + (mx%2 == 0 ? 2 : 1);
	int32 ey = (int32)dest_rect.Height() - sy + (my%2 == 0 ? 2 : 1);

	BRect clip_rect(dest_rect.left+sx, dest_rect.top+sy,
					dest_rect.left+ex, dest_rect.top+ey);
	
	CPointerList<clipping_rect> rect_list;
	
	for(int i=0 ; i<num_rects ; i++) {
		BRect result_rect(rects[i].left, rects[i].top,
						  rects[i].right, rects[i].bottom);
								  
		result_rect = result_rect & clip_rect;
		
		if(result_rect.IsValid()) {
			clipping_rect *result = new clipping_rect;
		
			result->left   = (int32)result_rect.left;
			result->right  = (int32)result_rect.right;
			result->top	   = (int32)result_rect.top;
			result->bottom = (int32)result_rect.bottom;
			
			rect_list.AddItem(result);
		}
	}

	num_rects = rect_list.CountItems();

	clipping_rect *result_array = new clipping_rect[num_rects];

	for(int i=0 ; i<num_rects ; i++) {
		result_array[i] = *rect_list.ItemAt(i);
	}
	
	return result_array;
}

// PixelFunc
// Get the correct pixel set function for this color space.
// Returns NULL, if the color space is invalid.
CBlur::SetPixelFunc CBlur::PixelFunc(color_space pixel_format)
{
	SetPixelFunc set_pixel;

	switch(pixel_format) {
		case B_RGB32:
		case B_RGBA32:
			set_pixel = &SetPixelRGB32;
			break;
		case B_RGB15:
		case B_RGBA15:
			set_pixel = &SetPixelRGB15;
			break;
		case B_RGB16:
			set_pixel = &SetPixelRGB16;
			break;
		case B_RGB32_BIG:
		case B_RGBA32_BIG:
			set_pixel = &SetPixelRGB32_Big;
			break;
		case B_RGB15_BIG:
		case B_RGBA15_BIG:
			set_pixel = &SetPixelRGB15_Big;
			break;
		case B_RGB16_BIG:	
			set_pixel = &SetPixelRGB16_Big;
			break;
		default:
			// Invalid pixel format
			set_pixel = NULL;
	}
	
	return set_pixel;
}

status_t CBlur::BlurRGB32(
	uchar *s_bits,				// source buffer
	int32  s_bytes_per_row,		// bytes per row of source buffer
	uchar *r_bits,				// destination buffer
	int32  r_bytes_per_row,		// bytes per row of dest buffer
	SetPixelFunc set_pixel,		// pointer to set pixel function
	int32  dest_x,				// position were drawing starts in dest
	int32  dest_y,
	int32  sx,					// upper left corner of blur-area
	int32  sy,
	int32  ex,					// bottom right corner of blur-area
	int32  ey,
	int32  matrixX,				// size of blur "matrix"
	int32  matrixY
)
{
	int32 mxh = matrixX/2;
	int32 myh = matrixY/2;

	uchar num_pixel = matrixX * matrixY;

	uchar *r_row_start = GetRowStart(r_bits, r_bytes_per_row, sy+dest_y);

	for(int32 y=sy ; y<ey ; y++) {
		
		for(int32 x=sx ; x<ex ; x++) {
			uint32 data_r=0, data_g=0, data_b=0; 

			int32 max = x - mxh;
			int32 may = y - myh;

			int32 mex = max + matrixX;
			int32 mey = may + matrixY;

			uchar *s_row_start = GetRowStart(s_bits, s_bytes_per_row, may);

			for(int32 my=may ; my<mey ; my++) {
				for(int32 mx=max ; mx<mex ; mx++) {
					int32 pixel = *((int32 *)(s_row_start + (mx << 2)));
		
					data_r += pixel & 0x000000FF;
					data_g += pixel & 0x0000FF00;
					data_b += pixel & 0x00FF0000;
				}
				
				s_row_start += s_bytes_per_row;
			}
			
			data_g >>= 8;
			data_b >>= 16;
			
			data_r /= num_pixel;
			data_g /= num_pixel;
			data_b /= num_pixel;

			if(data_r > 255) data_r = 255;
			if(data_g > 255) data_g = 255;
			if(data_b > 255) data_b = 255;

			set_pixel(r_row_start, x+dest_x, data_r, data_g, data_b);
		}
		
		r_row_start += r_bytes_per_row;
	}
	
	return B_OK;
}

status_t CBlur::BlurGray1(
	uchar *s_bits,				// source buffer
	int32  s_bytes_per_row,		// bytes per row of source buffer
	uchar *r_bits,				// destination buffer
	int32  r_bytes_per_row,		// bytes per row of dest buffer
	SetPixelFunc set_pixel,		// pointer to set pixel function
	int32  dest_x,				// position were drawing starts in dest
	int32  dest_y,
	int32  sx,					// upper left corner of blur-area
	int32  sy,
	int32  ex,					// bottom right corner of blur-area
	int32  ey,
	int32  matrixX,				// size of blur "matrix"
	int32  matrixY
)
{
	static uchar masks[] = {
		0x0001 << 7,
		0x0001 << 6,
		0x0001 << 5,
		0x0001 << 4,
		0x0001 << 3,
		0x0001 << 2,
		0x0001 << 1,
		0x0001
	};

	int32 mxh = matrixX/2;
	int32 myh = matrixY/2;

	uchar num_pixel = matrixX * matrixY;

	uchar *r_row_start = GetRowStart(r_bits, r_bytes_per_row, sy+dest_y);

	for(int32 y=sy ; y<ey ; y++) {
		for(int32 x=sx ; x<ex ; x++) {
			uint32 data=0;
			
			int32 max = x - mxh;
			int32 may = y - myh;

			int32 mex = max + matrixX;
			int32 mey = may + matrixY;
			
			for(int32 my=may ; my<mey ; my++) {
				uchar *row_start = GetRowStart(s_bits, s_bytes_per_row, my);
				uchar pixel;
				
				for(int32 mx=max ; mx<mex ; mx++) {
					pixel = (*(row_start + (mx >> 3)) & masks[mx%8]) ? 0 : 1;
				
					data += pixel;
				}
			}
			
			data = (255 * data) / num_pixel; 

			if(data > 255) data = 255;
			
			set_pixel(r_row_start, x+dest_x, data, data, data);
		}
		
		r_row_start += r_bytes_per_row;
	}
	
	return B_OK;
}

void CBlur::SetPixelRGB32(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	*((int32 *)(row_start + (x<<2))) = 0xFF000000 | ((int32)b << 16) | ((int32)g << 8) | r;
}

void CBlur::SetPixelRGB15(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	r >>= 3;
	g >>= 3;
	b >>= 3;
	
	*((int16 *)(row_start + (x<<1))) = 0x8000 | ((int16)b << 10) | ((int16)g << 5) | r;
}

void CBlur::SetPixelRGB16(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	r >>= 3;
	g >>= 2;
	b >>= 3;
	
	*((int16 *)(row_start + (x<<1))) = ((int16)b << 11) | ((int16)g << 5) | r;
}

void CBlur::SetPixelRGB32_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	*((int32 *)(row_start + (x<<2))) = 0x000000FF | ((int32)r << 24) | ((int32)g << 16) | ((int32)b << 8);
}

void CBlur::SetPixelRGB15_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	r >>= 3;
	g >>= 3;
	b >>= 3;
	
	*((int16 *)(row_start + (x<<1))) = 0x0080 | ((int16)(g & 0x07) << 13) | 
										((int16)r << 8) | ((int16)b << 2) | (g >> 3);
}

void CBlur::SetPixelRGB16_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b)
{
	r >>= 3;
	g >>= 2;
	b >>= 3;
	
	int16 a = ((int16)(g & 0x07) << 13) | ((int16)r << 8) | 
										((int16)b << 3) | (g >> 3);
										
	*((int16 *)(row_start + (x<<1))) = a;
}

#endif // _TMGR_FULLBLUR