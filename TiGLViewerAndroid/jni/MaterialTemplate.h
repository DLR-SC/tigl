#ifndef MATERIALTEMPLATE_H
#define MATERIALTEMPLATE_H

#include<osg/Material>

class MaterialTemplate
{
public:
	static osg::ref_ptr<osg::Material> getMaterial(int number);
};

#endif 
