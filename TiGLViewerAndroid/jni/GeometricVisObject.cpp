#include "GeometricVisObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

#include "tixi.h"

#include <osg/Geometry>
#include <osg/Material>
#include <osgUtil/SmoothingVisitor>

int GeometricVisObject::noOfGVO = 0;

GeometricVisObject::GeometricVisObject()
{
	setName("");
}


GeometricVisObject::~GeometricVisObject(void)
{
}


int GeometricVisObject::readTiglPolydata(tigl::CTiglPolyData& polyData)
{


	polyData.switchObject(1);
	tigl::CTiglPolyObject& inputObject = polyData.currentObject();

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

	osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec3Array* normals =  new osg::Vec3Array();



	for(unsigned int iPoly = 0; iPoly < inputObject.getNPolygons(); iPoly++){
		if(inputObject.getNPointsOfPolygon(iPoly) != 3){
			osg::notify(osg::ALWAYS) << "Error: polygon has to be a triangle!" << std::endl;
			continue;
		}
		// else
		unsigned long index0 = inputObject.getVertexIndexOfPolygon(0, iPoly);
		unsigned long index1 = inputObject.getVertexIndexOfPolygon(1, iPoly);
		unsigned long index2 = inputObject.getVertexIndexOfPolygon(2, iPoly);

		tigl::CTiglPoint vertexPoint0  = inputObject.getVertexPoint (index0);
		tigl::CTiglPoint vertexPoint1  = inputObject.getVertexPoint (index1);
		tigl::CTiglPoint vertexPoint2  = inputObject.getVertexPoint (index2);

		tigl::CTiglPoint vertexNormal0 = inputObject.getVertexNormal(index0);
		tigl::CTiglPoint vertexNormal1 = inputObject.getVertexNormal(index1);
		tigl::CTiglPoint vertexNormal2 = inputObject.getVertexNormal(index2);

		osg::Vec3f vertex0(vertexPoint0.x,  vertexPoint0.y,  vertexPoint0.z );
		osg::Vec3f vertex1(vertexPoint1.x,  vertexPoint1.y,  vertexPoint1.z );
		osg::Vec3f vertex2(vertexPoint2.x,  vertexPoint2.y,  vertexPoint2.z );

		osg::Vec3f normal0(vertexNormal0.x, vertexNormal0.y, vertexNormal0.z);
		osg::Vec3f normal1(vertexNormal1.x, vertexNormal1.y, vertexNormal1.z);
		osg::Vec3f normal2(vertexNormal2.x, vertexNormal2.y, vertexNormal2.z);

		// scale normals to unit length
		normal0.normalize();
		normal1.normalize();
		normal2.normalize();

		vertices->push_back(vertex0);
		vertices->push_back(vertex1);
		vertices->push_back(vertex2);
		normals ->push_back(normal0);
		normals ->push_back(normal1);
		normals ->push_back(normal2);

	}

	osg::DrawArrays* array = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES , 0 , vertices->size());

	geometry->addPrimitiveSet(array);
	geometry->setVertexArray(vertices);
	geometry->setNormalArray(normals);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	this->addDrawable(geometry.get());

	//this->setPicked(false);

	return 0;
}


int GeometricVisObject::readVTK(const char* xmlFilename)
{

	

	TixiDocumentHandle handle = -1;
	 
	if(tixiOpenDocument( xmlFilename, &handle ) != SUCCESS){
		osg::notify(osg::ALWAYS) << "Could not open " << xmlFilename << "!" << std::endl;
		return 1;
	}

    char * text = NULL;
    if(tixiGetTextElement(handle, "/VTKFile/PolyData/Piece[1]/Points/DataArray", &text)!=SUCCESS){
    	osg::notify(osg::ALWAYS) << "Error reading vtk file " << xmlFilename << "!" << std::endl;
    	return 2;
    }


    char * textDummy = text;
    const char *  ptr = strtok(textDummy, " \t\n");
    int iVertexCounter = 0;
    double x, y, z;

	osg::Vec3Array *vertices = new osg::Vec3Array;

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

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

    tixiGetTextElement(handle, "/VTKFile/PolyData/Piece[1]/Polys/DataArray[1]", &text);
    char * polyptr = strtok(text, " \t\n");

    while(polyptr != NULL)
    {
        indices->push_back(atoi(polyptr));
        polyptr = strtok(NULL, " \t\n");
    }


	tixiCloseDocument( handle );


	this->id = noOfGVO;
	noOfGVO++;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();


	//this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));
	geometry->setVertexArray(vertices);
	geometry->addPrimitiveSet(indices.get());


	this->addDrawable(geometry.get());

	//this->setPicked(false);

	osgUtil::SmoothingVisitor sv;
	sv.setCreaseAngle(osg::DegreesToRadians(80.0f));
	this->accept(sv);

	return 0;
}

//void GeometricVisObject::pick(){
//	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(1));
//	this->setPicked(true);
//}

//void GeometricVisObject::unpick(){
//	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(0));
//	this->setPicked(false);
//}
