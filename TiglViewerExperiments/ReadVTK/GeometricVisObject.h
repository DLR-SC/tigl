#ifndef GEOMETRIC_VIS_OBJECT_H
#define GEOMETRIC_VIS_OBJECT_H

#include <osg/Geode>
#include "VisObject.h"

class GeometricVisObject : public VisObject
{
public:
	static int noOfGVO;
	GeometricVisObject();

	~GeometricVisObject(void);
    void pick();
    void unpick();

	int readVTK(const char* filename);

	int getId(){return id;};
	bool isPickable(){return false;};
    bool isPicked(){return this->picked;};
    void setPicked(bool v){this->picked = v;};

private:
	int id;
	bool picked;
	osg::BoundingBox box;


};

#endif 
