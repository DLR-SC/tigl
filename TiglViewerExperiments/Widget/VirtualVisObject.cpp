#include "VirtualVisObject.h"
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <iostream>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

void VirtualVisObject::setXYGrid(bool active, int size, int unit){

	if(!xyGeode) initXYGeode(size, unit);

	if(active)
		this->addChild(xyGeode);
	else
		this->removeChild(xyGeode);

	xy = active;
}

void VirtualVisObject::setXZGrid(bool active, int size, int unit){
	if(!xzGeode) initXZGeode(size, unit);

	if(active)
		this->addChild(xzGeode);
	else
		this->removeChild(xzGeode);

	xz = active;
}


void VirtualVisObject::setYZGrid(bool active, int size, int unit){
	if(!yzGeode) initYZGeode(size, unit);

	if(active)
		this->addChild(yzGeode);
	else
		this->removeChild(yzGeode);

	yz=active;
}


void VirtualVisObject::setAxes(bool active){
	if(!axesGeode) initAxesGeode();

	if(active)
		this->addChild(axesGeode);
	else
		this->removeChild(axesGeode);

	axes = active;
}

void VirtualVisObject::initXYGeode(int size, int unit){

	xyGeode = new osg::Geode();
	osg::Geometry* linesX = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(-size,i*unit,0));
		vertices->push_back(osg::Vec3(size,i*unit,0));
		vertices->push_back(osg::Vec3(-size,-i*unit,0));
		vertices->push_back(osg::Vec3(size,-i*unit,0));
	}

	linesX->setVertexArray(vertices);
	linesX->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesY = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(i*unit,-size,0));
		vertices->push_back(osg::Vec3(i*unit,size,0));
		vertices->push_back(osg::Vec3(-i*unit, -size,0));
		vertices->push_back(osg::Vec3(-i*unit,size,0));
	}

	linesY->setVertexArray(vertices);
	linesY->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	xyGeode->addDrawable(linesX);
	xyGeode->addDrawable(linesY);
}

void VirtualVisObject::initXZGeode(int size,int unit){

	xzGeode = new osg::Geode();
	osg::Geometry* linesX = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(size, 0, i*unit));
		vertices->push_back(osg::Vec3(-size,0, i*unit));
		vertices->push_back(osg::Vec3(-size,0, -i*unit));
		vertices->push_back(osg::Vec3(size ,0, -i*unit));
	}

	linesX->setVertexArray(vertices);
	linesX->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesZ = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(i*unit,0,-size));
		vertices->push_back(osg::Vec3(i*unit,0,size));
		vertices->push_back(osg::Vec3(-i*unit,0,-size));
		vertices->push_back(osg::Vec3(-i*unit,0,size));
	}

	linesZ->setVertexArray(vertices);
	linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	xzGeode->addDrawable(linesX);
	xzGeode->addDrawable(linesZ);
}


void VirtualVisObject::initYZGeode(int size, int unit){
	

	yzGeode = new osg::Geode();

	osg::Geometry* linesY = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(0, size, i*unit));
		vertices->push_back(osg::Vec3(0, -size, i*unit));
		vertices->push_back(osg::Vec3(0, -size, -i*unit));
		vertices->push_back(osg::Vec3(0, size , -i*unit));
	}

	linesY->setVertexArray(vertices);
	linesY->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesZ = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size/unit+1; i++){
		vertices->push_back(osg::Vec3(0,i*unit,-size));
		vertices->push_back(osg::Vec3(0,i*unit,size));	
		vertices->push_back(osg::Vec3(0,-i*unit,size));
		vertices->push_back(osg::Vec3(0,-i*unit,-size));
	}

	linesZ->setVertexArray(vertices);
	linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));


	yzGeode->addDrawable(linesY);
	yzGeode->addDrawable(linesZ);
}

void VirtualVisObject::initAxesGeode(){

	axesGeode = new osg::Geode();

	

	osg::ref_ptr<osg::Cylinder> xAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);
	xAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,1,0)));
	osg::ref_ptr<osg::Cylinder> yAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);
	yAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(1,0,0)));
	osg::ref_ptr<osg::Cylinder> zAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);

	axesGeode->addDrawable(new osg::ShapeDrawable(xAxis));
	axesGeode->addDrawable(new osg::ShapeDrawable(yAxis));
	axesGeode->addDrawable(new osg::ShapeDrawable(zAxis));

}
