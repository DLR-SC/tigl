#ifndef VIRTUAL_VIS_OBJECT_H
#define VIRTUAL_VIS_OBJECT_H

#include <osg/Group>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

class VirtualVisObject : public osg::Group
{
public:
	VirtualVisObject()
		{
			setXYGrid(true);
			setAxes(true);

		};

	void initGeodes();

	bool isPickable(){return false;};

	bool xyActive(){return xy;};
	bool xzActive(){return xz;};
	bool yzActive(){return yz;};
	bool axesActive(){return axes;};

	void setXYGrid(bool active, int size=70, int unit=10);
	void setXZGrid(bool active, int size=70, int unit=10);
	void setYZGrid(bool active, int size=70, int unit=10);
	void setAxes(bool active);

	osg::Camera* camera;


private:
	osg::ref_ptr<osg::Geode> axesGeode;
	osg::ref_ptr<osg::Geode> xyGeode;
	osg::ref_ptr<osg::Geode> xzGeode;
	osg::ref_ptr<osg::Geode> yzGeode;

	osg::ref_ptr<osg::PositionAttitudeTransform> hudAxesTransform;

	void initXYGeode(int size, int unit);
	void initXZGeode(int size, int unit);
	void initYZGeode(int size, int unit);
	void initAxesGeode();



	bool xy;
	bool xz;
	bool yz;
	bool axes;
};

#endif

