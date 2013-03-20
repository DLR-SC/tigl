#ifndef GEOMETRIC_VIS_OBJECT_H
#define GEOMETRIC_VIS_OBJECT_H

#include <osg/Geode>
#include "InputObject.h"
#include "CTiglPolyData.h"

class GeometricVisObject : public osg::Geode
{
public:
	GeometricVisObject(char* filename, char* objectName);
	GeometricVisObject(tigl::CTiglPolyData& polyData, char* objectName);
	~GeometricVisObject(void);
	void pick();
	void unpick();
	int getId(){return id;};
	bool isPicked(){return this->picked;};
	void setPicked(bool v){this->picked = v;};

private:
	void create(tigl::CTiglPolyData& inputObject);
	void createFromVTK(InputObject* inputObject);
	InputObject* readVTK(char* filename);
	int id;
	bool picked;
	osg::BoundingBox box;

};

#endif 