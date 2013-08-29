#ifndef VISOBJECT_H
#define VISOBJECT_H

#include<osg/Geode>

class VisObject : public osg::Geode
{
public:
	virtual bool isPickable() = 0;


};

#endif