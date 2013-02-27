#include "VirtualVisObject.h"
#include <osg/Geometry>
#include <osg/ShapeDrawable>

void VirtualVisObject::showCross(){

	osg::Cylinder* xCylinder = new osg::Cylinder(osg::Vec3(0.5f,0,0), 0.07f, 1.0f);
	xCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
	osg::Cylinder* yCylinder = new osg::Cylinder(osg::Vec3(0,0.5f,0), 0.07f, 1.0f);
	yCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
	osg::Cylinder* zCylinder = new osg::Cylinder(osg::Vec3(0,0,0.5f), 0.07f, 1.0f);
	
	osg::Cone* xCone = new osg::Cone(osg::Vec3(1.0f,0,0), 0.12f,0.2f);
	xCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
	osg::Cone* yCone = new osg::Cone(osg::Vec3(0,1.0f,0), 0.12f,0.2f);
	yCone->setRotation(osg::Quat(-osg::PI_2, osg::Vec3(0.5f,0,0)));
	osg::Cone* zCone = new osg::Cone(osg::Vec3(0,0,1.0f), 0.12f,0.2f);

	this->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0,0,0), 0.125f)));
	this->addDrawable(new osg::ShapeDrawable(xCylinder));
	this->addDrawable(new osg::ShapeDrawable(yCylinder));
	this->addDrawable(new osg::ShapeDrawable(zCylinder));
	this->addDrawable(new osg::ShapeDrawable(xCone));
	this->addDrawable(new osg::ShapeDrawable(yCone));
	this->addDrawable(new osg::ShapeDrawable(zCone));
}

void VirtualVisObject::showXYGrid(int size, int unit){

	osg::Geometry* linesX = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(-size,i*unit,0));
		vertices->push_back(osg::Vec3(size,i*unit,0));
		vertices->push_back(osg::Vec3(-size,-i*unit,0));
		vertices->push_back(osg::Vec3(size,-i*unit,0));
	}

	linesX->setVertexArray(vertices);
	linesX->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesY = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(i*unit,-size,0));
		vertices->push_back(osg::Vec3(i*unit,size,0));
		vertices->push_back(osg::Vec3(-i*unit, -size,0));
		vertices->push_back(osg::Vec3(-i*unit,size,0));
	}

	linesY->setVertexArray(vertices);
	linesY->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	this->addDrawable(linesX);
	this->addDrawable(linesY);
}

void VirtualVisObject::showXZGrid(int size, int unit){
	osg::Geometry* linesX = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(size, 0, i*unit));
		vertices->push_back(osg::Vec3(-size,0, i*unit));
		vertices->push_back(osg::Vec3(-size,0, -i*unit));
		vertices->push_back(osg::Vec3(size ,0, -i*unit));
	}

	linesX->setVertexArray(vertices);
	linesX->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesZ = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(i*unit,0,-size));
		vertices->push_back(osg::Vec3(i*unit,0,size));
		vertices->push_back(osg::Vec3(-i*unit,0,-size));
		vertices->push_back(osg::Vec3(-i*unit,0,size));
	}

	linesZ->setVertexArray(vertices);
	linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	this->addDrawable(linesX);
	this->addDrawable(linesZ);
}


void VirtualVisObject::showYZGrid(int size, int unit){
	osg::Geometry* linesY = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(0, size, i*unit));
		vertices->push_back(osg::Vec3(0, -size, i*unit));
		vertices->push_back(osg::Vec3(0, -size, -i*unit));
		vertices->push_back(osg::Vec3(0, size , -i*unit));
	}

	linesY->setVertexArray(vertices);
	linesY->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

	osg::Geometry* linesZ = new osg::Geometry();

	vertices = new osg::Vec3Array();
	for(int i = 0; i < size; i++){
		vertices->push_back(osg::Vec3(0,i*unit,-size));
		vertices->push_back(osg::Vec3(0,i*unit,size));	
		vertices->push_back(osg::Vec3(0,-i*unit,size));
		vertices->push_back(osg::Vec3(0,-i*unit,-size));
	}

	linesZ->setVertexArray(vertices);
	linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));


	this->addDrawable(linesY);
	this->addDrawable(linesZ);
}


void VirtualVisObject::showAxes(){
	osg::ref_ptr<osg::Cylinder> xAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);
	xAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,1,0)));
	osg::ref_ptr<osg::Cylinder> yAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);
	yAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(1,0,0)));
	osg::ref_ptr<osg::Cylinder> zAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);

	this->addDrawable(new osg::ShapeDrawable(xAxis));
	this->addDrawable(new osg::ShapeDrawable(yAxis));
	this->addDrawable(new osg::ShapeDrawable(zAxis));


}
