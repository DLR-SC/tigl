#ifndef INPUT_OBJECT_H
#define INPUT_OBJECT_H

#include<osg/Array>
#include<osg/Config>
#include<osg/Geometry>

class InputObject
{
public:
	InputObject(osg::ref_ptr<osg::Vec3Array> vertices, osg::ref_ptr<osg::DrawElementsUInt> indices);
	osg::ref_ptr<osg::Vec3Array> getVertices(){return this->vertices;};
	osg::ref_ptr<osg::DrawElementsUInt> getIndices() {return this->indices;};
private:
	osg::ref_ptr<osg::Vec3Array> vertices;
	osg::ref_ptr<osg::DrawElementsUInt> indices;

};

#endif

