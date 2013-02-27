#ifndef VIRTUAL_VIS_OBJECT_H
#define VIRTUAL_VIS_OBJECT_H

#include "visobject.h"


class VirtualVisObject : public VisObject
{
public:
	VirtualVisObject()
		{
			showXYGrid(50, 1);
			xy = true;
			showCross();
			cross = true;
			showAxes();
			axes = true;

		};
	bool isPickable(){return false;};

private:
	void showXYGrid(int size, int unit);
	void showXZGrid(int size, int unit);
	void showYZGrid(int size, int unit);

	void showCross();
	void showAxes();

	bool xy;
	bool xz;
	bool yz;
	bool cross;
	bool axes;
};

#endif

