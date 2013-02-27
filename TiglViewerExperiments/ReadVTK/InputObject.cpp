#include "InputObject.h"



InputObject::InputObject(osg::ref_ptr<osg::Vec3Array> vertices, osg::ref_ptr<osg::DrawElementsUInt> indices)
{
	this->vertices = vertices;
	this->indices = indices;
}
