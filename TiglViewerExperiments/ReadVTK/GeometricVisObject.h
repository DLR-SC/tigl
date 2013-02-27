#ifndef GEOMETRIC_VIS_OBJECT_H
#define GEOMETRIC_VIS_OBJECT_H

#include "VisObject.h"
#include "InputObject.h"

class GeometricVisObject : public VisObject
{
public:
	static int noOfGVO;
	GeometricVisObject(char* filename, char* objectName);
	GeometricVisObject(InputObject* inputOject, char* objectName);
	~GeometricVisObject(void);
	void pick();
	void unpick();
	int getId(){return id;};
	bool isPickable(){return true;};
	bool isPicked(){return this->picked;};
	void setPicked(bool v){this->picked = v;};

private:
	void create(InputObject* inputObject);
	InputObject* readVTK(char* filename);
	int id;
	bool picked;


};

#endif 