#ifndef _H_GFXDEVICE
#define _H_GFXDEVICE

//W: width of screen
//H: height of screen
template <int8_t W, int8_t H>
class GfxDevice
{
public:
	//return width of screen
	int8_t getWidth() 
	{
		return W;
	}

	//return height of screen
	int8_t getHeight()
	{
		return H;
	}

	virtual void begin() = 0;
	virtual void display() = 0;
	virtual void clean() = 0;

protected:
private:
};

#endif //_H_GFXDEVICE