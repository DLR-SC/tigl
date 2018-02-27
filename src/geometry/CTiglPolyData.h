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
#include "tigl_internal.h"
#include <tixi.h>
#include "CTiglPoint.h"

class ObjectImpl;

#define UNDEFINED_REAL 0.0

namespace tigl 
{

class CTiglPolygon
{
public:
    TIGL_EXPORT CTiglPolygon();

    TIGL_EXPORT void addPoint(const CTiglPoint&);

    // returns number of points inside polygon
    TIGL_EXPORT unsigned int getNPoints() const;

    TIGL_EXPORT CTiglPoint& getPoint(unsigned int index);

    //returns reference to point object
    TIGL_EXPORT const CTiglPoint& getPointConst(unsigned int index) const;

    //returns reference to normal vector
    TIGL_EXPORT const CTiglPoint& getNormConst(unsigned int index) const;

    // sets a metadata text, can be used for vtk export
    TIGL_EXPORT void setMetadata(const char *);

    // returns metadata text
    TIGL_EXPORT const char * getMetadata() const;

    TIGL_EXPORT void addNormal(const CTiglPoint&);

private:
    std::string _metadata;
    std::vector<CTiglPoint> _points;
    std::vector<CTiglPoint> _normals;
};

class CTiglPolyObject
{
public:
    TIGL_EXPORT CTiglPolyObject();
    TIGL_EXPORT ~CTiglPolyObject();
    
    // switch , if to store normal vectors
    TIGL_EXPORT void enableNormals(bool);

    TIGL_EXPORT bool hasNormals() const;
    
    // adds a triangle to the global polygon list. the corners are spcified with the vertex indices
    // returned by addPointNormal or getVertexIndexOfPolygon. Returns index of added Triangle
    TIGL_EXPORT unsigned long addTriangleByVertexIndex(unsigned long i1, unsigned long i2, unsigned long i3 );

    // adds a polygon to the object
    TIGL_EXPORT void addPolygon(const class CTiglPolygon&);
    
    TIGL_EXPORT unsigned long getNVertices() const;

    // returns the number of polygons for the current surface
    TIGL_EXPORT unsigned long getNPolygons() const;

    // returns the number of points of the polygon
    TIGL_EXPORT unsigned long getNPointsOfPolygon(unsigned long iPoly) const;

    // states, if some metadata were set for this object
    TIGL_EXPORT bool hasMetadata() const;
    
    TIGL_EXPORT const char * getPolyMetadata(unsigned long iPoly) const;

    TIGL_EXPORT void setPolyMetadata(unsigned long iPoly, const char *);
    
    // sets a description of the elements stored in the metadata
    TIGL_EXPORT void setMetadataElements(const char *);
    
    TIGL_EXPORT const char * getMetadataElements() const;

    // returns the vertex index of the ith point of a polygon
    // to retrieve the actual point, call getPoint with this index
    TIGL_EXPORT unsigned long getVertexIndexOfPolygon(unsigned long iPoint, unsigned long iPoly) const;
    
    // each polygon has a list of data elements, this methods sets the value
    TIGL_EXPORT void setPolyDataReal(unsigned long iPoly, const char * dataName, double value);
    
    TIGL_EXPORT double getPolyDataReal(unsigned long iPoly, const char * dataName) const;

    //  ---------- Vertex specific methods -----------------
    
    // adds a point and its normal vector to the global point list and returns its index in the list
    TIGL_EXPORT unsigned long addPointNormal(const class CTiglPoint& p, const class CTiglPoint& n);
    
    // returns the actual point, specified with the vertex index
    TIGL_EXPORT const CTiglPoint& getVertexPoint(unsigned long iVertexIndex) const;
    
    // returns the normal vector, specified with vertex index
    TIGL_EXPORT const CTiglPoint& getVertexNormal(unsigned long iVertexIndex) const;
    
    // each vertex has a list of data elements, this methods sets the value
    TIGL_EXPORT void setVertexDataReal(unsigned long iVertexIndex, const char * dataName, double value);
    
    TIGL_EXPORT double getVertexDataReal(unsigned long iVertexIndex, const char * dataName) const;
    
    // returns the number if different polygon data entries 
    TIGL_EXPORT unsigned int getNumberOfPolyRealData() const;
    
    // retuns the  name of the ith data field (i = 0 .. getNumberPolyReadlData - 1)
    TIGL_EXPORT const char * getPolyDataFieldName(unsigned long iField) const;


private:
    CTiglPolyObject& operator=(const CTiglPolyObject&);

    ObjectImpl * impl;
};

class CTiglPolyData 
{
public:
    TIGL_EXPORT CTiglPolyData();
    TIGL_EXPORT ~CTiglPolyData();

    // returns number of object
    TIGL_EXPORT unsigned int getNObjects() const;

    // returns the total number of polygons, including all objects
    TIGL_EXPORT unsigned long getTotalPolygonCount() const;
    // returns the total number of vertices, including all objects
    TIGL_EXPORT unsigned long getTotalVertexCount() const;

    // creates a new object, switches current object to the new one
    // we store the polygon data as different object
    // to allow hard edges
    TIGL_EXPORT CTiglPolyObject& createNewObject();

    TIGL_EXPORT CTiglPolyObject& currentObject();

    // changes the current surface, we count from 1 to getNObjects
    TIGL_EXPORT CTiglPolyObject& switchObject(unsigned int iObject);
    
    TIGL_EXPORT const CTiglPolyObject& getObject(unsigned int iObject) const;

private:

    std::vector<CTiglPolyObject*> _objects;
    std::vector<CTiglPolyObject*>::iterator itCurrentObj;
};

} // end namespace tigl

#endif // VTKPOLYDATA_H
