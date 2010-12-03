#ifndef I_PIXEL_SELECTOR_H_
#define I_PIXEL_SELECTOR_H_

typedef int ScreenCoordinate;
typedef int StackCoordinate;

class IPixelSelector
{
public:
	virtual ~IPixelSelector(){}
	virtual StackCoordinate pick(ScreenCoordinate coord) = 0;
};


class PixelSelector : public IPixelSelector
{
public:
	PixelSelector();
	StackCoordinate pick(ScreenCoordinate coord);
};

#endif//I_PIXEL_SELECTOR_H_
