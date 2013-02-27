#include "GeometricVisObject.h"
#include "InputObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <time.h>

#include "tixi.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <osg/Geometry>
#include <osg/Material>
#include <osgUtil/SmoothingVisitor>

int GeometricVisObject::noOfGVO = 0;

GeometricVisObject::GeometricVisObject(char* filename, char* objectName)
{

	InputObject* inputObject = readVTK(filename);
	create(inputObject);
	this->setName(objectName);

}

GeometricVisObject::GeometricVisObject(InputObject* inputObject, char* objectName)
{
	create(inputObject);
	this->setName(objectName);
}


GeometricVisObject::~GeometricVisObject(void)
{
}

void GeometricVisObject::create(InputObject* inputObject)
{	
	this->id = noOfGVO;
	noOfGVO++;
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
}

void GeometricVisObject::unpick(){
	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));
	this->setPicked(false);
}