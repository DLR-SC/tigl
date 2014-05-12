/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "GeometricVisObject.h"
#include "MaterialTemplate.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

#include "tixi.h"
#include "ITiglGeometricComponent.h"
#include "CTiglTriangularizer.h"
#include "CHotsoseMeshReader.h"
#include <TopoDS_Shape.hxx>

#include <osg/Geometry>
#include <osg/Material>
#include <osg/LightModel>
#include <osgUtil/SmoothingVisitor>

#include <map>

int GeometricVisObject::noOfGVO = 0;

template<class T>
osg::ref_ptr<osg::Geode> SplitGeometry(osg::Vec3Array * vertexArray, T* indices)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    int iIndexDiff = 15600;

    for (int iStartIndex = 0; iStartIndex < indices->size(); iStartIndex += iIndexDiff) {

        std::map<int, int> indexMap;
        std::map<int, int>::iterator it;

        int iStopIndex = iStartIndex + iIndexDiff < indices->size() ? iStartIndex + iIndexDiff : indices->size();

        // create map
        osg::ref_ptr<osg::DrawElementsUShort> indicesShort = new osg::DrawElementsUShort(indices->getMode(), iStopIndex - iStartIndex);
        for (int iIndex = iStartIndex; iIndex < iStopIndex; ++iIndex) {
            int vertexIndexOld = indices->at(iIndex);

            it = indexMap.find(vertexIndexOld);
            if (it == indexMap.end()) {
                int nextIndex = indexMap.size();
                indexMap[vertexIndexOld] = nextIndex;
                indicesShort->at(iIndex - iStartIndex) = nextIndex;
            }
            else {
                indicesShort->at(iIndex - iStartIndex) = it->second;
            }
        }

        // create new vertex array
        osg::ref_ptr<osg::Vec3Array> verticesShort = new osg::Vec3Array(indexMap.size());
        for (it = indexMap.begin(); it != indexMap.end(); ++it) {
            int newIndex = it->second;
            int oldOndex = it->first;
            verticesShort->at(newIndex) = vertexArray->at(oldOndex);
        }
        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
        geometry->setVertexArray(verticesShort);
        geometry->addPrimitiveSet(indicesShort);

        geode->addDrawable(geometry.get());
    }

    return geode;
}

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
    osg::Vec3Array* normals = new osg::Vec3Array();

    for (unsigned int iPoly = 0; iPoly < inputObject.getNPolygons(); iPoly++) {
        if (inputObject.getNPointsOfPolygon(iPoly) != 3) {
            osg::notify(osg::ALWAYS) << "Error: polygon has to be a triangle!" << std::endl;
            continue;
        }
        // else

        for (int vindex = 0; vindex < 3; vindex++) {
            unsigned long index = inputObject.getVertexIndexOfPolygon(vindex, iPoly);

            tigl::CTiglPoint vertexPoint = inputObject.getVertexPoint(index);
            tigl::CTiglPoint vertexNormal = inputObject.getVertexNormal(index);

            osg::Vec3f vertex(vertexPoint.x, vertexPoint.y, vertexPoint.z);
            osg::Vec3f normal(vertexNormal.x, vertexNormal.y, vertexNormal.z);

            // scale normals to unit length
            normal.normalize();

            vertices->push_back(vertex);
            normals->push_back(normal);
        }
    }

    osg::DrawArrays* array = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size());

    geometry->addPrimitiveSet(array);
    geometry->setVertexArray(vertices);
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry.get());
    geode->setCullingActive(false);
    geode->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    osg::ref_ptr<osg::LightModel> pLightModel = new osg::LightModel();
    pLightModel->setTwoSided( true );
    geode->getOrCreateStateSet()->setAttributeAndModes(pLightModel.get(), osg::StateAttribute::ON);

    this->addChild(geode);

    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));

    this->setPicked(false);

    return 0;
}

int GeometricVisObject::readHotsoseMesh(const char* filename)
{
    CHotsoseMeshReader reader;
    tigl::CTiglPolyData polyData;
    if (reader.readFromFile(filename, polyData) != TIGL_SUCCESS) {
        osg::notify(osg::ALWAYS) << "Could not open " << filename << "!" << std::endl;
        return 1;
    }

    polyData.switchObject(1);
    tigl::CTiglPolyObject& inputObject = polyData.currentObject();

    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS);

    for (unsigned int iPoly = 0; iPoly < inputObject.getNPolygons(); iPoly++) {
        if (inputObject.getNPointsOfPolygon(iPoly) != 4) {
            osg::notify(osg::ALWAYS) << "Error: polygon has to be a quad!" << std::endl;
            continue;
        }

        for (int vindex = 3; vindex >= 0; vindex--) {
            unsigned long index = inputObject.getVertexIndexOfPolygon(vindex, iPoly);
            indices->push_back(index);
        }
    }

    for (int iVertex = 0; iVertex < inputObject.getNVertices(); ++iVertex) {
        const tigl::CTiglPoint& p = inputObject.getVertexPoint(iVertex);
        vertices->push_back(osg::Vec3(p.x, p.y, p.z));
    }

    osg::ref_ptr<osg::Geode> geode = SplitGeometry<osg::DrawElementsUInt>(vertices, indices);

    osgUtil::SmoothingVisitor sv;
    sv.setCreaseAngle(osg::DegreesToRadians(80.0f));
    geode->accept(sv);

    geode->setCullingActive(false);
    geode->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    osg::ref_ptr<osg::LightModel> pLightModel = new osg::LightModel();
    pLightModel->setTwoSided( true );
    this->getOrCreateStateSet()->setAttributeAndModes(pLightModel.get(), osg::StateAttribute::ON);

    this->addChild(geode);
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));
    this->setPicked(false);

}

int GeometricVisObject::readVTK(const char* xmlFilename)
{
    TixiDocumentHandle handle = -1;

    if (tixiOpenDocument(xmlFilename, &handle) != SUCCESS) {
        osg::notify(osg::ALWAYS) << "Could not open " << xmlFilename << "!" << std::endl;
        return 1;
    }

    char * text = NULL;
    if (tixiGetTextElement(handle, "/VTKFile/PolyData/Piece[1]/Points/DataArray", &text) != SUCCESS) {
        osg::notify(osg::ALWAYS) << "Error reading vtk file " << xmlFilename << "!" << std::endl;
        return 2;
    }

    char * textDummy = text;
    const char * ptr = strtok(textDummy, " \t\n");
    int iVertexCounter = 0;
    double x, y, z;

    osg::Vec3Array *vertices = new osg::Vec3Array;

    int iCoordinateCount = 0;
    while (ptr != NULL) {
        float val = atof(ptr);
        switch (iCoordinateCount) {
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
            vertices->push_back(osg::Vec3(x, y, z));

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

    while (polyptr != NULL) {
        indices->push_back(atoi(polyptr));
        polyptr = strtok(NULL, " \t\n");
    }

    tixiCloseDocument(handle);

    this->id = noOfGVO;
    noOfGVO++;

    osg::ref_ptr<osg::Geode> geode = SplitGeometry<osg::DrawElementsUInt>(vertices, indices);

    osgUtil::SmoothingVisitor sv;
    sv.setCreaseAngle(osg::DegreesToRadians(80.0f));
    geode->accept(sv);

    this->addChild(geode);
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));

    this->setPicked(false);

    return 0;
}

void GeometricVisObject::pick()
{
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(SELECTED));
    this->setPicked(true);
}

void GeometricVisObject::unpick()
{
    this->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(UNSELECTED));
    this->setPicked(false);
}
