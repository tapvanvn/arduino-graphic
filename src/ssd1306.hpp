#ifndef _H_SSD1306_FOR_GRAPHIC
#define _H_SSD1306_FOR_GRAPHIC

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266)
 #include <util/delay.h>
#endif

#include "GfxDevice.hpp"
#include <stdint.h>
#include <Wire.h>

//default ssd1306 device is SSD1306_128_64
#if !defined(SSD1306_128_32) && !defined(SSD1306_128_64) && !defined(SSD1306_96_16)

#	define SSD1306_128_64

#endif

//vcc type
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#ifndef _swap_uint8_t
#define _swap_uint8_t(a, b) { uint8_t t = a; a = b; b = t; }
#endif



template <uint8_t W, uint8_t H, typename T>

class Ssd1306 : public GfxDevice <W,H, T>
{
public:
	Ssd1306(uint8_t i2c_device_address, uint8_t switch_vcc = SSD1306_SWITCHCAPVCC)
	{
		_i2c_device_address = i2c_device_address;
		_switch_vcc = switch_vcc;
	}

	void command(uint8_t c)
	{

	    // I2C
	    uint8_t control = 0x00;   // Co = 0, D/C = 0
	    Wire.beginTransmission(_i2c_device_address);
	    Wire.write(control);
	    Wire.write(c);
	    Wire.endTransmission();

	}

	//implement GfxDevice
	virtual void begin()
	{
		Wire.begin();
/*
#ifdef __SAM3X8E__
		// Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
		TWI1->TWI_CWGR = 0;
		TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
*/
		// Init sequence
		command(SSD1306_DISPLAYOFF);                    // 0xAE
		command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
		command(0x80);                                  // the suggested ratio 0x80

		command(SSD1306_SETMULTIPLEX);                  // 0xA8
		command(H - 1);

		command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
		command(0x0);                                   // no offset
		command(SSD1306_SETSTARTLINE | 0x0);            // line #0
		command(SSD1306_CHARGEPUMP);                    // 0x8D
		if (_switch_vcc == SSD1306_EXTERNALVCC)
		{
			command(0x10);
		}
		else
		{
			command(0x14);
		}
		command(SSD1306_MEMORYMODE);                    // 0x20
		command(0x00);                                  // 0x0 act like ks0108
		command(SSD1306_SEGREMAP | 0x1);
		command(SSD1306_COMSCANDEC);


        if(H == 32)
        {
    		command(SSD1306_SETCOMPINS);                    // 0xDA
    		command(0x02);
    		command(SSD1306_SETCONTRAST);                   // 0x81
    		command(0x8F);
        }
        else if(H==64)
        {
    		command(SSD1306_SETCOMPINS);                    // 0xDA
    		command(0x12);
    		command(SSD1306_SETCONTRAST);                   // 0x81
    		if (_switch_vcc == SSD1306_EXTERNALVCC)
    		{
    			command(0x9F);
    		}
    		else
    		{
    			command(0xCF);
    		}
        }
        else if(W == 96 && H==16){
    		command(SSD1306_SETCOMPINS);                    // 0xDA
    		command(0x2);   //ada x12
    		command(SSD1306_SETCONTRAST);                   // 0x81
    		if (_switch_vcc == SSD1306_EXTERNALVCC)
    		{
    			command(0x10);
    		}
    		else
    		{
    			command(0xAF);
    		}
        }
		command(SSD1306_SETPRECHARGE);                  // 0xd9
		if (_switch_vcc == SSD1306_EXTERNALVCC)
		{
			command(0x22);
		}
		else
		{
			command(0xF1);
		}
		command(SSD1306_SETVCOMDETECT);                 // 0xDB
		command(0x40);
		command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
		command(SSD1306_NORMALDISPLAY);                 // 0xA6

		command(SSD1306_DEACTIVATE_SCROLL);

		command(SSD1306_DISPLAYON);//--turn on oled panel
	}

	//implement GfxDevice
	virtual void display()
	{
		command(SSD1306_COLUMNADDR);
		command(0);   // Column start address (0 = reset)
		command(W - 1); // Column end address (127 = reset)

		command(SSD1306_PAGEADDR);
		command(0); // Page start address (0 = reset)
		command(H/8 - 1);

	// save I2C bitrate
/*
#ifdef TWBR
		uint8_t twbrbackup = TWBR;
		TWBR = 12; // upgrade to 400KHz!
#endif
*/

		//Serial.println(TWBR, DEC);
		//Serial.println(TWSR & 0x3, DEC);

		// I2C
		for (uint16_t i = 0; i<((W*H) / 8); i++)
		{
			// send a bunch of data in one xmission
			Wire.beginTransmission(_i2c_device_address);
			Wire.write(0x40);
			for (uint8_t x = 0; x<16; x++)
			{
				Wire.write(buffer[i]);
				i++;
			}
			i--;
			Wire.endTransmission();
		}
/*
#ifdef TWBR
		TWBR = twbrbackup;
#endif
*/
	}

	//implement GfxDevice
	virtual void clean()
	{
		for( uint16_t i = 0; i< (W * H)/8; i++)
		{
			buffer[i] = 0x00;
		}

    }

    virtual void drawFixel(T x, T y, uint8_t color = COLOR_WHITE)
	{
        switch (color)
        {
        case COLOR_WHITE:   buffer[x+ (y/8)*W] |=  (1 << (y&7)); break;
        case COLOR_BLACK:   buffer[x+ (y/8)*W] &= ~(1 << (y&7)); break;
        case COLOR_INVERSE: buffer[x+ (y/8)*W] ^=  (1 << (y&7)); break;
        }
    }
    virtual void drawFastVLine(T x, T y, T h, uint8_t color)
    {
        drawFastVLineInternal(x, y, h, color);
    }
    virtual void drawFastHLine(T x, T y, T w, uint8_t color)
    {
        drawFastHLineInternal(x, y, w, color);
    }
protected:
    inline void drawFastVLineInternal(T x, T __y, T __h, uint8_t color) __attribute__((always_inline))
    {
        // do nothing if we're off the left or right side of the screen
        if(x < 0 || x >= W) { return; }

        // make sure we don't try to draw below 0
        if(__y < 0) {
        // __y is negative, this will subtract enough from __h to account for __y being 0
        __h += __y;
        __y = 0;

        }

        // make sure we don't go past the height of the display
        if( (__y + __h) > H) {
        __h = (H - __y);
        }

        // if our height is now negative, punt
        if(__h <= 0) {
        return;
        }

        // this display doesn't need ints for coordinates, use local byte registers for faster juggling
        register uint8_t y = __y;
        register uint8_t h = __h;


        // set up the pointer for fast movement through the buffer
        register uint8_t *pBuf = buffer;
        // adjust the buffer pointer for the current row
        pBuf += ((y/8) * W);
        // and offset x columns in
        pBuf += x;

        // do the first partial byte, if necessary - this requires some masking
        register uint8_t mod = (y&7);
        if(mod) {
        // mask off the high n bits we want to set
        mod = 8-mod;

        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        // register uint8_t mask = ~(0xFF >> (mod));
        static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        register uint8_t mask = premask[mod];

        // adjust the mask if we're not going to reach the end of this byte
        if( h < mod) {
        mask &= (0XFF >> (mod-h));
        }

        switch (color)
        {
            case COLOR_WHITE:   *pBuf |=  mask;  break;
            case COLOR_BLACK:   *pBuf &= ~mask;  break;
            case COLOR_INVERSE: *pBuf ^=  mask;  break;
        }

        // fast exit if we're done here!
        if(h<mod) { return; }

        h -= mod;

        pBuf += W;
        }


        // write solid bytes while we can - effectively doing 8 rows at a time
        if(h >= 8)
        {
            if (color == COLOR_INVERSE)
            {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
                do  {
                    *pBuf=~(*pBuf);

                    // adjust the buffer forward 8 rows worth of data
                    pBuf += W;

                    // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                    h -= 8;
                } while(h >= 8);
            }
            else
            {
                // store a local value to work with
                register uint8_t val = (color == COLOR_WHITE) ? 255 : 0;

                do  {
                    // write our value in
                    *pBuf = val;

                    // adjust the buffer forward 8 rows worth of data
                    pBuf += W;

                    // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                    h -= 8;
                } while(h >= 8);
            }
        }

        // now do the final partial byte, if necessary
        if(h) {
            mod = h & 7;
            // this time we want to mask the low bits of the byte, vs the high bits we did above
            // register uint8_t mask = (1 << mod) - 1;
            // note - lookup table results in a nearly 10% performance improvement in fill* functions
            static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
            register uint8_t mask = postmask[mod];
            switch (color)
            {
                case COLOR_WHITE:   *pBuf |=  mask;  break;
                case COLOR_BLACK:   *pBuf &= ~mask;  break;
                case COLOR_INVERSE: *pBuf ^=  mask;  break;
            }
        }
    }
    inline void drawFastHLineInternal(T x, T y, T w, uint8_t color = COLOR_WHITE) __attribute__((always_inline))
    {
        // Do bounds/limit checks
         if(y < 0 || y >= H) { return; }

         // make sure we don't try to draw below 0
         if(x < 0)
         {
           w += x;
           x = 0;
         }

         // make sure we don't go off the edge of the display
         if( (x + w) > W)
         {
           w = (H - x);
         }

         // if our width is now negative, punt
         if(w <= 0) { return; }

         // set up the pointer for  movement through the buffer
         register uint8_t *pBuf = buffer;
         // adjust the buffer pointer for the current row
         pBuf += ((y/8) * W);
         // and offset x columns in
         pBuf += x;

         register uint8_t mask = 1 << (y&7);

         switch (color)
         {
            case COLOR_WHITE:         while(w--) { *pBuf++ |= mask; }; break;
            case COLOR_BLACK: mask = ~mask;   while(w--) { *pBuf++ &= mask; }; break;
            case COLOR_INVERSE:         while(w--) { *pBuf++ ^= mask; }; break;
         }
    }
private:
	uint8_t _i2c_device_address;
	uint8_t _switch_vcc;

	static uint8_t buffer[(W * H) / 8];
};

template <uint8_t W, uint8_t H, typename T>
uint8_t Ssd1306<W,H,T>::buffer[(W * H) / 8] = {0};


#endif//_H_SSD1306_FOR_GRAPHIC
