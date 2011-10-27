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
 
#ifndef BLUR_H
#define BLUR_H

class CBlur
{
	public:
	static status_t Blur(
		BBitmap *source, 
		BBitmap *dest, 
		int32 mx, 
		int32 my
	);
	
	static status_t Blur(
		BBitmap *source, 			// source bitmap
		uchar *r_bits, 				// destimation buffer
		int32 r_bytes_per_row,		// bytes per row of dest buffer
		color_space r_pixel_format,	// color space of dest buffer
		clipping_rect *dest_rect,	// destination of the blur operation
		clipping_rect *rects,		// visible region of dest buffer (in screen coordinates)
		int32 num_rects,			// number of clipping_rect structs in 'rects'
		int32 mx,					// size of blur matrix
		int32 my
	);
		
	static clipping_rect *ClipRects(
		const clipping_rect *rects, 
		int32 &num_rects,
		BRect dest_rect,
		int32 mx,
		int32 my
	);

	static bool SupportedInputColorSpace(color_space in_color_space);
	static bool SupportedOutputColorSpace(color_space out_color_space);

	protected:
	typedef void (*SetPixelFunc)(uchar *, int32, uchar, uchar, uchar); 

	static SetPixelFunc PixelFunc(color_space pixel_format);

	static status_t BlurGray1(
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
	);

	static status_t BlurRGB32(
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
	);

	static inline uchar *GetRowStart(uchar *bits, int32 bytes_per_row, int32 y)
	{
		return bits + bytes_per_row*y;
	}

	// little endian pixel set functions
	static void SetPixelRGB32(uchar *row_start, int32 x, uchar r, uchar g, uchar b);
	static void SetPixelRGB15(uchar *row_start, int32 x, uchar r, uchar g, uchar b);
	static void SetPixelRGB16(uchar *row_start, int32 x, uchar r, uchar g, uchar b);

	// big endian pixel set functions
	static void SetPixelRGB32_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b);
	static void SetPixelRGB15_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b);
	static void SetPixelRGB16_Big(uchar *row_start, int32 x, uchar r, uchar g, uchar b);
};

#endif // BLUR_H