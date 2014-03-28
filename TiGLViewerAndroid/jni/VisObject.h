#ifndef VISOBJECT_H
#define VISOBJECT_H

#include<osg/Geode>

class VisObject : public osg::Group
{
public:
    virtual bool isPickable() = 0;

};

#endif
