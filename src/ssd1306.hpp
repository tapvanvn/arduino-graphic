#ifndef _H_SSD1306_FOR_GRAPHIC
#define _H_SSD1306_FOR_GRAPHIC
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



template <int8_t W, int8_t H>

class Ssd1306 : public GfxDevice <W,H>
{
public:
	Ssd1306(uint8_t i2c_device_address, uint8_t switch_vcc = SSD1306_SWITCHCAPVCC)
	{
		_i2c_device_address = i2c_device_address;
		_switch_vcc = switch_vcc;
	}

	//implement GfxDevice
	virtual void begin()
	{
		Wire.begin();
#ifdef __SAM3X8E__
		// Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
		TWI1->TWI_CWGR = 0;
		TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
		// Init sequence
		command(SSD1306_DISPLAYOFF);                    // 0xAE
		command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
		command(0x80);                                  // the suggested ratio 0x80

		command(SSD1306_SETMULTIPLEX);                  // 0xA8
		command(SSD1306_SCREEN_HEIGHT - 1);

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

#if defined SSD1306_128_32
		command(SSD1306_SETCOMPINS);                    // 0xDA
		command(0x02);
		command(SSD1306_SETCONTRAST);                   // 0x81
		command(0x8F);

#elif defined SSD1306_128_64
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

#elif defined SSD1306_96_16
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

#endif

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
		command(SSD1306_SCREEN_WIDTH - 1); // Column end address (127 = reset)

		command(SSD1306_PAGEADDR);
		command(0); // Page start address (0 = reset)
#if SSD1306_LCDHEIGHT == 64
		command(7); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 32
		command(3); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 16
		command(1); // Page end address
#endif

					// save I2C bitrate
#ifdef TWBR
		uint8_t twbrbackup = TWBR;
		TWBR = 12; // upgrade to 400KHz!
#endif

		//Serial.println(TWBR, DEC);
		//Serial.println(TWSR & 0x3, DEC);

		// I2C
		for (uint16_t i = 0; i<((W*H) / 8); i++) {
			// send a bunch of data in one xmission
			Wire.beginTransmission(_i2c_addr);
			Wire.write(0x40);
			for (uint8_t x = 0; x<16; x++) {
				Wire.write(buffer[i]);
				i++;
			}
			i--;
			Wire.endTransmission();
		}
#ifdef TWBR
		TWBR = twbrbackup;
#endif
	}

	//implement GfxDevice
	virtual void clean()
	{

	}

protected:
private:
	uint8_t _i2c_device_address;
	uint8_t _switch_vcc;

	static uint8_t buffer[(W * H) / 8];
};

#endif//_H_SSD1306_FOR_GRAPHIC