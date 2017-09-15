#ifndef _H_GFX
#define _H_GFX

#ifndef NULL
#define NULL 0
#endif
#include "GfxDevice.hpp"
#include "GfxFont.h"

#include "glcdfont.inc"

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

template <uint8_t W, uint8_t H, typename T>
class Gfx
{
public:
	Gfx()
	{
		_device = NULL;
        _wrap = true;
        _gfx_font = NULL;
    	_text_size = 1;
	};
	void swap(T& x, T& y)
	{
		T t = x;
		x = y;
		y = t;
	};
	void setDevice(GfxDevice<W,H,T> *device)
	{
		_device = device;
	};
	void clean()
	{
		_device->clean();
	};
	void begin()
	{
		_device->begin();
	};
	void display()
	{
		_device->display();
	};
	void drawFixel(T x, T y, uint8_t color = COLOR_WHITE)
	{
		_device->drawFixel(x, y, color);
	};
	void drawLine(T x0, T y0, T x1, T y1, uint8_t color = COLOR_WHITE)
	{
		int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	    if (steep) {
	        swap(x0, y0);
	        swap(x1, y1);
	    }

	    if (x0 > x1) {
	        swap(x0, x1);
	        swap(y0, y1);
	    }

	    int16_t dx, dy;
	    dx = x1 - x0;
	    dy = abs(y1 - y0);

	    int16_t err = dx / 2;
	    int16_t ystep;

	    if (y0 < y1) {
	        ystep = 1;
	    } else {
	        ystep = -1;
	    }

	    for (; x0<=x1; x0++) {
	        if (steep) {
	            _device->drawFixel(y0, x0, color);
	        } else {
	            _device->drawFixel(x0, y0, color);
	        }
	        err -= dy;
	        if (err < 0) {
	            y0 += ystep;
	            err += dx;
	        }
	    }
	};
	void drawRect(T x, T y, T w, T h, uint8_t color = COLOR_WHITE)
	{
		drawLine(x, y, x + w, y, color);
		drawLine(x + w, y, x + w, y + h, color);
		drawLine(x + w, y + h, x , y + h, color);
		drawLine(x, y + h, x , y, color);
	}
	void drawCircle(T x, T y, T r, uint8_t density = 8, uint8_t color = COLOR_WHITE)
	{
		if (density < 1)return;
		T xx1 = x + (T)(0 * r);
		T yy1 = y + (T)(1 * r);
		float delta = (360 / ((4 * density) - 1));
		for (int angle = 0; angle <= 360; angle += delta)
		{
			T xx = x + (T)(sin((double)angle*3.1416f / 180.0f) * r);
			T yy = y + (T)(cos((double)angle*3.1416f / 180.0f) * r);

			drawLine(xx1, yy1, xx, yy, color);
			xx1 = xx;
			yy1 = yy;
		}
		drawLine(xx1, yy1, x + (T)(0 * r), y + (T)(1 * r), color);
	}

	void drawTriangle(T x0, T y0, T x1, T y1, T x2, T y2, uint8_t color = COLOR_WHITE)
	{
		drawLine(x0, y0, x1, y1);
		drawLine(x1, y1, x2, y2);
		drawLine(x0, y0, x2, y2);
	}
	void fillRect(T x, T y, T w, T h, uint8_t color = COLOR_WHITE)
	{
		for (int16_t i=x; i<x+w; i++)
		{
	        _device->drawFastVLine(i, y, h, color);
	    }
	}
	void fillCircleHelper(T x0, T y0, T r,
        uint8_t cornername, int16_t delta, uint8_t color = COLOR_WHITE) {

	    int16_t f     = 1 - r;
	    int16_t ddF_x = 1;
	    int16_t ddF_y = -2 * r;
	    int16_t x     = 0;
	    int16_t y     = r;

	    while (x<y) {
	        if (f >= 0) {
	            y--;
	            ddF_y += 2;
	            f     += ddF_y;
	        }
	        x++;
	        ddF_x += 2;
	        f     += ddF_x;

	        if (cornername & 0x1)
			{
	            _device->drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
	            _device->drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
	        }
	        if (cornername & 0x2)
			{
	            _device->drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
	            _device->drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
	        }
	    }
	}
	void fillCircle(T x, T y, T r, uint8_t color = COLOR_WHITE)
	{
		_device->drawFastVLine(x, y-r, 2*r+1, color);
   		fillCircleHelper(x, y, r, 3, 0, color);
	}
	void fillTriangle(T x0, T y0, T x1, T y1, T x2, T y2, uint8_t color = COLOR_WHITE) {

	    uint8_t a, b, y, last;

	    // Sort coordinates by Y order (y2 >= y1 >= y0)
	    if (y0 > y1) {
	        swap(y0, y1); swap(x0, x1);
	    }
	    if (y1 > y2) {
	        swap(y2, y1); swap(x2, x1);
	    }
	    if (y0 > y1) {
	        swap(y0, y1); swap(x0, x1);
	    }

	    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
	        a = b = x0;
	        if(x1 < a)      a = x1;
	        else if(x1 > b) b = x1;
	        if(x2 < a)      a = x2;
	        else if(x2 > b) b = x2;
	        _device->drawFastHLine(a, y0, b-a+1, color);
	        return;
	    }

	    int16_t
	    dx01 = x1 - x0,
	    dy01 = y1 - y0,
	    dx02 = x2 - x0,
	    dy02 = y2 - y0,
	    dx12 = x2 - x1,
	    dy12 = y2 - y1;
	    int32_t
	    sa   = 0,
	    sb   = 0;

	    // For upper part of triangle, find scanline crossings for segments
	    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	    // is included here (and second loop will be skipped, avoiding a /0
	    // error there), otherwise scanline y1 is skipped here and handled
	    // in the second loop...which also avoids a /0 error here if y0=y1
	    // (flat-topped triangle).
	    if(y1 == y2) last = y1;   // Include y1 scanline
	    else         last = y1-1; // Skip it

	    for(y=y0; y<=last; y++) {
	        a   = x0 + sa / dy01;
	        b   = x0 + sb / dy02;
	        sa += dx01;
	        sb += dx02;
	        /* longhand:
	        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
	        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	        */
	        if(a > b) swap(a,b);
	       _device->drawFastHLine(a, y, b-a+1, color);
	    }

	    // For lower part of triangle, find scanline crossings for segments
	    // 0-2 and 1-2.  This loop is skipped if y1=y2.
	    sa = dx12 * (y - y1);
	    sb = dx02 * (y - y0);
	    for(; y<=y2; y++) {
	        a   = x1 + sa / dy12;
	        b   = x0 + sb / dy02;
	        sa += dx12;
	        sb += dx02;
	        /* longhand:
	        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
	        */
	        if(a > b) swap(a,b);
	        _device->drawFastHLine(a, y, b-a+1, color);
	    }
	}
	// Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
	// using the specified foreground color (unset bits are transparent).
	void drawBitmap(T x, T y, const uint8_t bitmap[], T w, T h, uint8_t color = COLOR_WHITE) {

	    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	    uint8_t byte = 0;

	    for(int16_t j=0; j<h; j++, y++) {
	        for(int16_t i=0; i<w; i++) {
	            if(i & 7) byte <<= 1;
	            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
	            if(byte & 0x80) _device->drawFixel(x+i, y, color);
	        }
	    }
	}
	void drawChar(T x, T y, unsigned char c, uint8_t size = 1,
	  uint8_t color = COLOR_WHITE, uint8_t bg = COLOR_BLACK)
	  {

	    if(!_gfx_font)
		{ // 'Classic' built-in font

	        if((x >= W)            || // Clip right
	           (y >= H)           || // Clip bottom
	           ((x + 6 * size - 1) < 0) || // Clip left
	           ((y + 8 * size - 1) < 0))   // Clip top
	            return;

	        //if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

	        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
	            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
	            for(int8_t j=0; j<8; j++, line >>= 1) {
	                if(line & 1) {
	                    if(size == 1)
	                        _device->drawFixel(x+i, y+j, color);
	                    else
	                        fillRect(x+i*size, y+j*size, size, size, color);
	                } else if(bg != color) {
	                    if(size == 1)
	                        _device->drawFixel(x+i, y+j, bg);
	                    else
	                        fillRect(x+i*size, y+j*size, size, size, bg);
	                }
	            }
	        }
	        if(bg != color) { // If opaque, draw vertical line for last column
	            if(size == 1) _device->drawFastVLine(x+5, y, 8, bg);
	            else          fillRect(x+5*size, y, size, 8*size, bg);
	        }

	    } else { // Custom font

	        // Character is assumed previously filtered by write() to eliminate
	        // newlines, returns, non-printable characters, etc.  Calling
	        // drawChar() directly with 'bad' characters of font may cause mayhem!

	        c -= (uint8_t)pgm_read_byte(&_gfx_font->first);
	        GfxGlyph *glyph  = &(((GfxGlyph *)pgm_read_pointer(&_gfx_font->glyph))[c]);
	        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&_gfx_font->bitmap);

	        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
	        uint8_t  w  = pgm_read_byte(&glyph->width),
	                 h  = pgm_read_byte(&glyph->height);
	        int8_t   xo = pgm_read_byte(&glyph->xOffset),
	                 yo = pgm_read_byte(&glyph->yOffset);
	        uint8_t  xx, yy, bits = 0, bit = 0;
	        int16_t  xo16 = 0, yo16 = 0;

	        if(size > 1) {
	            xo16 = xo;
	            yo16 = yo;
	        }

	        // Todo: Add character clipping here

	        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
	        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
	        // has typically been used with the 'classic' font to overwrite old
	        // screen contents with new data.  This ONLY works because the
	        // characters are a uniform size; it's not a sensible thing to do with
	        // proportionally-spaced fonts with glyphs of varying sizes (and that
	        // may overlap).  To replace previously-drawn text when using a custom
	        // font, use the getTextBounds() function to determine the smallest
	        // rectangle encompassing a string, erase the area with fillRect(),
	        // then draw new text.  This WILL infortunately 'blink' the text, but
	        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
	        // only creates a new set of problems.  Have an idea to work around
	        // this (a canvas object type for MCUs that can afford the RAM and
	        // displays supporting setAddrWindow() and pushColors()), but haven't
	        // implemented this yet.

	        for(yy=0; yy<h; yy++) {
	            for(xx=0; xx<w; xx++) {
	                if(!(bit++ & 7)) {
	                    bits = pgm_read_byte(&bitmap[bo++]);
	                }
	                if(bits & 0x80) {
	                    if(size == 1) {
	                        _device->drawFixel(x+xo+xx, y+yo+yy, color);
	                    } else {
	                        fillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,
	                          size, size, color);
	                    }
	                }
	                bits <<= 1;
	            }
	        }

	    } // End classic vs custom font
	}

	void charBounds(char c, int8_t &x, int8_t &y,
	  int8_t &minx, int8_t &miny, int8_t &maxx, int8_t &maxy)
      {

	    if(_gfx_font) {

	        if(c == '\n') { // Newline?
	            x  = 0;    // Reset x to zero, advance y by one line
	            y += _text_size * (uint8_t)pgm_read_byte(&_gfx_font->yAdvance);
	        } else if(c != '\r') { // Not a carriage return; is normal char
	            uint8_t first = pgm_read_byte(&_gfx_font->first),
	                    last  = pgm_read_byte(&_gfx_font->last);
	            if((c >= first) && (c <= last)) { // Char present in this font?
	                GfxGlyph *glyph = &(((GfxGlyph *)pgm_read_pointer(
	                  &_gfx_font->glyph))[c - first]);
	                uint8_t gw = pgm_read_byte(&glyph->width),
	                        gh = pgm_read_byte(&glyph->height),
	                        xa = pgm_read_byte(&glyph->xAdvance);
	                int8_t  xo = pgm_read_byte(&glyph->xOffset),
	                        yo = pgm_read_byte(&glyph->yOffset);
	                if(_wrap && ((x+(((int16_t)xo+gw)*_text_size)) > W)) {
	                    x  = 0; // Reset x to zero, advance y by one line
	                    y += _text_size * (uint8_t)pgm_read_byte(&_gfx_font->yAdvance);
	                }
	                int16_t ts = (int16_t)_text_size,
	                        x1 = x + xo * ts,
	                        y1 = y + yo * ts,
	                        x2 = x1 + gw * ts - 1,
	                        y2 = y1 + gh * ts - 1;
	                if(x1 < minx) minx = x1;
	                if(y1 < miny) miny = y1;
	                if(x2 > maxx) maxx = x2;
	                if(y2 > maxy) maxy = y2;
	                x += xa * ts;
	            }
	        }

	    } else { // Default font

	        if(c == '\n') {                     // Newline?
	            x  = 0;                        // Reset x to zero,
	            y += _text_size * 8;             // advance y one line
	            // min/max x/y unchaged -- that waits for next 'normal' character
	        } else if(c != '\r') {  // Normal char; ignore carriage returns
	            if(_wrap && ((x + _text_size * 6) > W)) { // Off right?
	                x  = 0;                    // Reset x to zero,
	                y += _text_size * 8;         // advance y one line
	            }
	            int x2 = x + _text_size * 6 - 1, // Lower-right pixel of char
	                y2 = y + _text_size * 8 - 1;
	            if(x2 > maxx) maxx = x2;      // Track max x, y
	            if(y2 > maxy) maxy = y2;
	            if(x < minx) minx = x;      // Track min x, y
	            if(y < miny) miny = y;
	            x += _text_size * 6;             // Advance x one char
	        }

	    }
	}
    void drawText(T x, T y, const char* text, uint8_t size = 1,  uint8_t color = COLOR_WHITE, uint8_t bg = COLOR_BLACK)
    {
        char* ptr = text;

        while(*ptr != '\0')
        {
            drawChar(x, y, *ptr, size, color, bg);
            x += 6 * size;
            ptr++;
        }
    }
protected:

private:
	GfxDevice<W, H,T> *_device;
	GfxFont *_gfx_font;
	uint8_t _text_size;
    bool _wrap;
};


#endif//_H_GFX
