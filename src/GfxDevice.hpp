#ifndef _H_GFXDEVICE
#define _H_GFXDEVICE

#define COLOR_WHITE	1
#define COLOR_BLACK 0
#define COLOR_INVERSE	2
//W: width of screen
//H: height of screen
template <uint8_t W, uint8_t H, typename T>
class GfxDevice
{
public:
	//return width of screen
	uint8_t getWidth()
	{
		return W;
	}

	//return height of screen
	uint8_t getHeight()
	{
		return H;
	}

	virtual void begin() = 0;
	virtual void display() = 0;
	virtual void clean() = 0;
	virtual void drawFixel(T x, T y, uint8_t color = COLOR_WHITE) = 0;
	virtual void drawFastVLine(T x, T y, T h, uint8_t color = COLOR_WHITE) = 0;
  	virtual void drawFastHLine(T x, T y, T w, uint8_t color = COLOR_WHITE) = 0;

protected:
private:

};

#endif //_H_GFXDEVICE
