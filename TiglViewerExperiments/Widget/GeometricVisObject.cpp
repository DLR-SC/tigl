#include "GeometricVisObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <time.h>

#include "tixi.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <osg/Geometry>
#include <osg/Material>
#include <osgUtil/SmoothingVisitor>


GeometricVisObject::GeometricVisObject(char* filename, char* objectName)
{
	InputObject* inputObject = readVTK(filename);
	createFromVTK(inputObject);

	this->setName(objectName);
	box = this->getBoundingBox();
	std::cout<< box.center().x() << " "<<box.center().y() << " " << box.center().z() <<std::endl;
}

GeometricVisObject::GeometricVisObject(tigl::CTiglPolyData& inputObject, char* objectName)
{
	create(inputObject);
	this->setName(objectName);
}


GeometricVisObject::~GeometricVisObject(void)
{
}

void GeometricVisObject::createFromVTK(InputObject* inputObject)
{	
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();


	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));
	geometry->setVertexArray(inputObject->getVertices());
	geometry->addPrimitiveSet(inputObject->getIndices());


	this->addDrawable(geometry);
	this->setPicked(false);

	osgUtil::SmoothingVisitor sv;
	sv.setCreaseAngle(osg::DegreesToRadians(80.0f));
	this->accept(sv);
}

void GeometricVisObject::create(tigl::CTiglPolyData& polyData)
{	
	polyData.switchObject(1);
	tigl::CTiglPolyObject& inputObject = polyData.currentObject();

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();


	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));

	osg::Vec3Array* vertices = new osg::Vec3Array();
	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, inputObject.getNPolygons()*3);
	osg::Vec3Array* normals = new osg::Vec3Array();


	for(unsigned int i = 0; i < inputObject.getNVertices();i++){
		tigl::CTiglPoint vertexPoint = inputObject.getVertexPoint(i);
		tigl::CTiglPoint vertexNormal = inputObject.getVertexNormal(i);
		osg::Vec3f normal(vertexNormal.x, vertexNormal.y, vertexNormal.z);
		normal.normalize();
		normals->push_back(normal);
		vertices->push_back(osg::Vec3f(vertexPoint.x, vertexPoint.y, vertexPoint.z));

	}

	for(unsigned int iPoly = 0; iPoly < inputObject.getNPolygons(); iPoly++){
		if(inputObject.getNPointsOfPolygon(iPoly) != 3){
			std::cout << "Error: polygon has to be a triangle!" << std::endl;
		}
		for(unsigned int jPoint = 0; jPoint < inputObject.getNPointsOfPolygon(iPoly); jPoint++){
			(*indices)[iPoly*3 + jPoint] = inputObject.getVertexIndexOfPolygon(jPoint, iPoly);
		}
	}

	geometry->addPrimitiveSet(indices);
	geometry->setVertexArray(vertices);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setNormalArray(normals);

	this->addDrawable(geometry);
	
	std::cout << this->getBoundingBox().center().x() <<" "<< this->getBoundingBox().center().y() << " "<< this->getBoundingBox().center().z() <<std::endl;


	this->setPicked(false);
}

InputObject* GeometricVisObject::readVTK(char* xmlFilename)
{
	TixiDocumentHandle handle = -1;
	 
	tixiOpenDocument( xmlFilename, &handle );
	//tixiGetDoubleElement( handle, elementPath, &x );
	/*if(tixiGetIntegerAttribute(handle, elementPath, attributeName, &number) != SUCCESS){
		std::cerr << "fehler" << std::endl;
	}
	else 
	    std::cout << number << std::endl;*/

	clock_t start, stop;
	start = clock();


	char * text = NULL;
	tixiGetTextElement(handle, "/VTKFile/PolyData/Piece/Points/DataArray", &text);
	
	std::vector<std::string> SplitVecStrings;
	std::vector<double> SplitVecDouble;
	osg::Vec3Array *vertices = new osg::Vec3Array;

	boost::algorithm::split( SplitVecStrings, text, boost::is_any_of(" \t\n"), boost::algorithm::token_compress_on );

	//Umwandlung von Strings in Double
	for(unsigned int i = 1; i < SplitVecStrings.size()-1; ++i){
		SplitVecDouble.push_back(atof(SplitVecStrings[i].data()));
		//Nach jeweils 3 Umwandlungen als Vec3 in *vertices schreiben 
		if(i%3 == 0){
			vertices->push_back(osg::Vec3(SplitVecDouble[i-3], SplitVecDouble[i-2], SplitVecDouble[i-1]));
		}
	}

	tixiGetTextElement(handle, "/VTKFile/PolyData/Piece/Polys/DataArray[1]", &text);

	boost::algorithm::split( SplitVecStrings, text, boost::is_any_of(" \t\n"), boost::algorithm::token_compress_on );

	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES,SplitVecStrings.size()-2);
	//Umwandlung von Strings in Int
	for(unsigned int i = 1; i < SplitVecStrings.size()-1; ++i){
		(*indices)[i-1] = atoi(SplitVecStrings[i].data());
	}

	InputObject* inputObject = new InputObject(vertices, indices);

	tixiCloseDocument( handle );

	stop = clock();
	std::cout << "Time:" << (stop-start)/((double) CLOCKS_PER_SEC) << std::endl;

	return inputObject;
}

void GeometricVisObject::pick(){
	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(1));
	this->setPicked(true);
	std::cout<<this->getName()<<" was Picked"<< std::endl;
}

void GeometricVisObject::unpick(){
	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));
	this->setPicked(false);
}
