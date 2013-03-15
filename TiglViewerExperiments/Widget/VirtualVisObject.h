#ifndef VIRTUAL_VIS_OBJECT_H
#define VIRTUAL_VIS_OBJECT_H

#include <osg/Group>
#include <osg/Geode>

class VirtualVisObject : public osg::Group
{
public:
	VirtualVisObject()
		{
			setXYGrid(true);
			setCross(true);
			setAxes(true);

		};

	void initGeodes();

	bool isPickable(){return false;};

	bool xyActive(){return xy;};
	bool xzActive(){return xz;};
	bool yzActive(){return yz;};
	bool crossActive(){return cross;};
	bool axesActive(){return axes;};

	void setXYGrid(bool active, int size=70, int unit=10);
	void setXZGrid(bool active, int size=70, int unit=10);
	void setYZGrid(bool active, int size=70, int unit=10);
	void setCross(bool active);
	void setAxes(bool active);


private:
	osg::ref_ptr<osg::Geode> axesGeode;
	osg::ref_ptr<osg::Geode> crossGeode;
	osg::ref_ptr<osg::Geode> xyGeode;
	osg::ref_ptr<osg::Geode> xzGeode;
	osg::ref_ptr<osg::Geode> yzGeode;

	void initXYGeode(int size, int unit);
	void initXZGeode(int size, int unit);
	void initYZGeode(int size, int unit);
	void initCrossGeode();
	void initAxesGeode();



	bool xy;
	bool xz;
	bool yz;
	bool cross;
	bool axes;
};

#endif

