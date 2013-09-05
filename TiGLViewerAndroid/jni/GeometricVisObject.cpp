#include "GeometricVisObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

#include "tixi.h"
#include "ITiglGeometricComponent.h"
#include "CTiglTriangularizer.h"
#include <TopoDS_Shape.hxx>

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


int GeometricVisObject::fromShape(TopoDS_Shape& loft, double deflection)
{
	tigl::CTiglTriangularizer polyData(loft, deflection);

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

		for(int vindex = 0; vindex < 3; vindex++){
			unsigned long index = inputObject.getVertexIndexOfPolygon(vindex, iPoly);

			tigl::CTiglPoint vertexPoint  = inputObject.getVertexPoint (index);
			tigl::CTiglPoint vertexNormal = inputObject.getVertexNormal(index);

			osg::Vec3f vertex(vertexPoint.x,  vertexPoint.y,  vertexPoint.z );
			osg::Vec3f normal(vertexNormal.x, vertexNormal.y, vertexNormal.z);

			// scale normals to unit length
			normal.normalize();

			vertices->push_back(vertex);
			normals ->push_back(normal);
		}

	}

	osg::DrawArrays* array = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES , 0 , vertices->size());

	geometry->addPrimitiveSet(array);
	geometry->setVertexArray(vertices);
	geometry->setNormalArray(normals);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	this->addDrawable(geometry.get());

	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));

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

	this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));
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
