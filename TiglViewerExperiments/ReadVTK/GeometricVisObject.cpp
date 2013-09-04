#include "GeometricVisObject.h"
#include "InputObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

#include "tixi.h"

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


    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));
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



	
	std::vector<std::string> SplitVecStrings;
	std::vector<double> SplitVecDouble;
	osg::Vec3Array *vertices = new osg::Vec3Array;

    char * text = NULL;
    tixiGetTextElement(handle, "/VTKFile/PolyData/Piece/Points/DataArray", &text);



    char * textDummy = text;
    const char *  ptr = strtok(textDummy, " \t\n");
    int iVertexCounter = 0;
    double x, y, z;
    int iCoordinateCount = 0;


    while(ptr != NULL){
        float val = atof(ptr);
        switch(iCoordinateCount){
        case 0:
            x = val;
            iCoordinateCount++;
            break;
        case 1:
            y = val;
            iCoordinateCount++;
            break;
        case 2:
            z = val;
            //add to vertexlist
            vertices->push_back(osg::Vec3(x,y,z));

            iCoordinateCount = 0;
            iVertexCounter++;
            break;
        default:
            break;
        }

        ptr = strtok(NULL, " \t\n");
    }

    osg::ref_ptr<osg::DrawElementsUInt> indices2 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    tixiGetTextElement(handle, "/VTKFile/PolyData/Piece/Polys/DataArray[1]", &text);
    char * polyptr = strtok(text, " \t\n");

    while(polyptr != NULL)
    {
        indices2->push_back(atoi(polyptr));
        polyptr = strtok(NULL, " \t\n");
    }

	

	InputObject* inputObject = new InputObject(vertices, indices2);

	tixiCloseDocument( handle );

	stop = clock();
	std::cout << "Time:" << (stop-start)/((double) CLOCKS_PER_SEC) << std::endl;

	return inputObject;
}

void GeometricVisObject::pick(){
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(SELECTED));
	this->setPicked(true);
}

void GeometricVisObject::unpick(){
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));
	this->setPicked(false);
}
