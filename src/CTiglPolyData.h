/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-09 Martin Siggel <Martin.Siggel@dlr.de>
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
*
* @file CTiglPolyData.h
* @brief Handles polygon data for export and rendering usage
*/

#ifndef VTKPOLYDATA_H
#define VTKPOLYDATA_H

#include <vector>
#include <tixi.h>
#include "CTiglPoint.h"

class ObjectImpl;

#define UNDEFINED_REAL 0.0

namespace tigl {

class CTiglPolygon{
public:
    CTiglPolygon();

    void addPoint(const CTiglPoint&);

    // returns number of points inside polygon
    unsigned int getNPoints() const;

    CTiglPoint& getPoint(unsigned int index);

    //returns reference to point object
    const CTiglPoint& getPointConst(unsigned int index) const;

    //returns reference to normal vector
    const CTiglPoint& getNormConst(unsigned int index) const;

    // sets a metadata text, can be used for vtk export
    void setMetadata(const char *);

    // returns metadata text
    const char * getMetadata() const;

    void addNormal(const CTiglPoint&);

private:
    std::string _metadata;
    std::vector<CTiglPoint> _points;
    std::vector<CTiglPoint> _normals;
};

class CTiglPolyObject{
public:
    CTiglPolyObject();
    ~CTiglPolyObject();
    
    // switch , if to store normal vectors
    void enableNormals(bool);

    bool hasNormals() const;
    
    // adds a triangle to the global polygon list. the corners are spcified with the vertex indices
    // returned by addPointNormal or getVertexIndexOfPolygon. Returns index of added Triangle
    unsigned long addTriangleByVertexIndex(unsigned long i1, unsigned long i2, unsigned long i3 );

    // adds a polygon to the object
    void addPolygon(const class CTiglPolygon&);
    
    unsigned long getNVertices() const;

    // returns the number of polygons for the current surface
    unsigned long getNPolygons() const;

    // returns the number of points of the polygon
    unsigned long getNPointsOfPolygon(unsigned long iPoly) const;

    const char * getPolyMetadata(unsigned long iPoly) const;

    void setPolyMetadata(unsigned long iPoly, const char *);

    // returns the vertex index of the ith point of a polygon
    // to retrieve the actual point, call getPoint with this index
    unsigned long getVertexIndexOfPolygon(unsigned long iPoint, unsigned long iPoly) const;
    
    // each polygon has a list of data elements, this methods sets the value
    void setPolyDataReal(unsigned long iPoly, const char * dataName, double value);
    
    double getPolyDataReal(unsigned long iPoly, const char * dataName) const;

    //  ---------- Vertex specific methods -----------------
    
    // adds a point and its normal vector to the global point list and returns its index in the list
    unsigned long addPointNormal(const class CTiglPoint& p, const class CTiglPoint& n);    
    
    // returns the actual point, specified with the vertex index
    const CTiglPoint& getVertexPoint(unsigned long iVertexIndex) const;
    
    // returns the normal vector, specified with vertex index
    const CTiglPoint& getVertexNormal(unsigned long iVertexIndex) const;
    
    // each vertex has a list of data elements, this methods sets the value
    void setVertexDataReal(unsigned long iVertexIndex, const char * dataName, double value);
    
    double getVertexDataReal(unsigned long iVertexIndex, const char * dataName) const;
    
    // returns the number if different polygon data entries 
    unsigned int getNumberOfPolyRealData() const;
    
    // retuns the  name of the ith data field (i = 0 .. getNumberPolyReadlData - 1)
    const char * getPolyDataFieldName(unsigned long iField) const;


private:
    CTiglPolyObject& operator=(const CTiglPolyObject&);

    ObjectImpl * impl;
};

class CTiglPolyData {
public:
    CTiglPolyData();
    ~CTiglPolyData();

    // returns number of object
    unsigned int getNObjects();

    // creates a new object, switches current object to the new one
    // we store the polygon data as different object
    // to allow hard edges
    CTiglPolyObject& createNewObject();

    CTiglPolyObject& currentObject();

    // changes the current surface, we count from 1 to getNObjects
    CTiglPolyObject& switchObject(unsigned int iObject);

    void writeVTK(const char * filename);
    void printVTK();

private:
    void writeVTKPiece(TixiDocumentHandle& handle, unsigned int iObject);
    void createVTK(TixiDocumentHandle& handle);

    std::vector<CTiglPolyObject*> _objects;
    std::vector<CTiglPolyObject*>::iterator itCurrentObj;
};

}

#endif // VTKPOLYDATA_H
