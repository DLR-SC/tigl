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
* @file CTiglPolyData.cpp
* @brief Handles polygon data for export and rendering usage
*/

#include "CTiglPolyData.h"
#include "CTiglPoint.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "tigl.h"
#include "tixi.h"
#include "to_string.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <set>
#include <utility>

#include <fstream>
#include <cfloat>
#include <cstdio>

using namespace tigl;

#ifdef _MSC_VER
    #define snprintf _snprintf
#endif


#define COMP_TOLERANCE 1.e-10


struct TiglPointComparer;

class PointImpl 
{
public:
    PointImpl() : 
        p_(CTiglPoint(0,0,0)),
        n_(CTiglPoint(1,0,0)) 
    {
    }

    PointImpl(const CTiglPoint& p, const CTiglPoint& n) : p_(p), n_(n) 
    {
        realDataMap.clear();
    }

    CTiglPoint& point(){ return p_; }
    CTiglPoint& normal(){ return n_; }

    const CTiglPoint& getPoint() const { return p_; }
    const CTiglPoint& getNormal()const { return n_; }
    
    void setRealData(const char * txt, double value)
    {
        realDataMap[txt] = value;
    }
    
    double getRealData(const char * txt) const 
    {
        std::map<std::string, double>::const_iterator it=realDataMap.find(txt);
        if (it != realDataMap.end()) {
            return it->second;
        }
        else {
            return UNDEFINED_REAL;
        }
    }

private:
    CTiglPoint p_;
    CTiglPoint n_;
    
    std::map<std::string, double> realDataMap;
};

typedef std::map<PointImpl, unsigned int, TiglPointComparer> PointMap;

// Comparer for gp_Pnts
struct TiglPointComparer
{
    bool isSmaller(const CTiglPoint& lhs, const CTiglPoint& rhs) const 
    {
        // if two points lie inside the epsilon environment
        // they should be the same, hence lhs is not smaller than rhs
        if (lhs.distance2(rhs) < COMP_TOLERANCE ) {
            return false;
        }


        // as lhs and rhs don't lie inside an epsilon environment
        if (lhs.x < rhs.x) {
            return true;
        }
        else if (lhs.x == rhs.x) {
            if (lhs.y < rhs.y) {
                return true;
            }
            else if (lhs.y == rhs.y) {
                return lhs.z < rhs.z;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }


    // function should only return true, if lhs < rhs
    bool operator()(const PointImpl& lhs, const PointImpl& rhs) const
    {
        const CTiglPoint& leftP  = lhs.getPoint();
        const CTiglPoint& rightP = rhs.getPoint();

        const CTiglPoint& leftN  = lhs.getNormal();
        const CTiglPoint& rightN = rhs.getNormal();

        if (isSmaller(leftP,rightP)) {
            return true;
        }
        else {
            // if points are equal, compare normals
            if (leftP.distance2(rightP) <  COMP_TOLERANCE ) {
                return isSmaller(leftN, rightN);
            }
            else {
                return false;
            }
        }
    }
};


//class to store a polygon by its point indices
struct PolyIndexList
{
    PolyIndexList(int num) : _metadata("")
    {
        myid = num;
    }

    void setMetadata(const std::string& str)
    {
        _metadata = str;
    }

    const std::string& getMetadata() const
    {
        return _metadata;
    }
    
    void addPoint(int index);
    
    void setRealData(const char * txt, double value)
    {
        realDataMap[txt] = value;
    }
    
    double getRealData(const char * txt) const 
    {
        std::map<std::string, double>::const_iterator it=realDataMap.find(txt);
        if (it != realDataMap.end()) {
            return it->second;
        }
        else {
            return UNDEFINED_REAL;
        }
    }
    
    int getNVert() const
    {
        return static_cast<int>(pindex.size());
    }

    int getPointIndex(int i) const
    {
        return pindex.at(i);
    }
    
private:
    std::vector<int> pindex;

    //each polygon should have it's own metadata
    std::string _metadata;
    
    std::map<std::string, double> realDataMap;

    int myid;
};


class ObjectImpl 
{
public:
    ObjectImpl()
    {
        pointlist.clear();
        polys.clear();
        pPoints.clear();
        has_normals = false;
        has_metadata = false;
        metaDataElements = "";
    }

    void addPointNorm(const CTiglPoint &p, const CTiglPoint& norm, long polynum);
    unsigned long addPointNorm(const CTiglPoint &p, const CTiglPoint &norm);
    
    unsigned long addTriangleByVertexIndex(unsigned long i1, unsigned long i2, unsigned long i3 );
    
    void addPolygon(const CTiglPolygon&);
    
    unsigned long getNPolygons() const;
    unsigned long getNPointsOfPolygon(unsigned long ipoly) const;

    const CTiglPoint& getVertexPoint(unsigned long iVertexIndex) const;
    const CTiglPoint& getVertexNormal(unsigned long iVertexIndex) const;

    unsigned long getVertexIndexOfPolygon(unsigned long ipoint, unsigned long ipoly) const;

    bool has_normals;
    bool has_metadata;

    // we use this container to store point data, it allows us to find
    // efficiently a specific point in the list
    PointMap pointlist;

    // this vector keeps track of the order of points, it complies with
    // the polygon indices
    std::vector<PointImpl*> pPoints;

    std::vector<PolyIndexList> polys;
    
    std::set<std::string> polyDataElems;
    std::set<std::string> vertexDataElems;
    
    std::string metaDataElements;
};



// -----------------------------------------------------------------------//

CTiglPolyData::CTiglPolyData()
{
    _objects.clear();
    // we want to have one object by default
    _objects.push_back(new CTiglPolyObject());
    itCurrentObj = _objects.begin();
}

CTiglPolyData::~CTiglPolyData()
{
    std::vector<CTiglPolyObject*>::iterator pObj_it = _objects.begin();
    for (; pObj_it != _objects.end(); ++pObj_it) {
        delete *pObj_it;
    }
}

CTiglPolyObject& CTiglPolyData::currentObject()
{
    return **itCurrentObj;
}

CTiglPolyObject& CTiglPolyData::createNewObject() 
{
    _objects.push_back(new CTiglPolyObject());

    itCurrentObj = _objects.end()-1;
    return currentObject();
}

// changes the current surface, we count from 1 to getNSurfaces
CTiglPolyObject& CTiglPolyData::switchObject(unsigned int iObject)
{
    if (iObject >= 1 && iObject <= _objects.size()) {
        itCurrentObj = _objects.begin() + iObject -1;
        return currentObject();
    }
    else {
        throw tigl::CTiglError("Invalid surface index in CTiglPolyData::switchObject!", TIGL_INDEX_ERROR);
    }
}

const CTiglPolyObject& CTiglPolyData::getObject(unsigned int iObject) const
{
    if (iObject >= 1 && iObject <= _objects.size()) {
        return **(_objects.begin() + iObject -1);
    }
    else {
        throw tigl::CTiglError("Invalid surface index in CTiglPolyData::switchObject!", TIGL_INDEX_ERROR);
    }
}

unsigned int CTiglPolyData::getNObjects() const
{
    return static_cast<unsigned int>(_objects.size());
}

unsigned long CTiglPolyData::getTotalPolygonCount() const
{
    unsigned long nPolys = 0;

    for (unsigned int i = 1; i <= getNObjects(); ++i) {
        const CTiglPolyObject& obj = getObject(i);
        nPolys += obj.getNPolygons();
    }

    return nPolys;
}

unsigned long CTiglPolyData::getTotalVertexCount() const
{
    unsigned long nVertices = 0;

    for (unsigned int i = 1; i <= getNObjects(); ++i) {
        const CTiglPolyObject& obj = getObject(i);
        nVertices += obj.getNVertices();
    }

    return nVertices;
}

// --------------------------------------------------------------------------//

void PolyIndexList::addPoint(int index)
{
    // dont add same point twice
    if (pindex.size() == 0 || pindex.back() != index) {
         pindex.push_back(index);
    }
}


//--------------------------------------------------------------------------//

CTiglPolyObject::CTiglPolyObject()
{
    impl = new ObjectImpl;
}

CTiglPolyObject::~CTiglPolyObject()
{
    delete impl;
}

unsigned long CTiglPolyObject::addPointNormal(const CTiglPoint &p, const CTiglPoint &n)
{
    return impl->addPointNorm(p,n);
}

void CTiglPolyObject::addPolygon(const CTiglPolygon & polygon)
{
    impl->addPolygon(polygon);
}

unsigned long CTiglPolyObject::addTriangleByVertexIndex(unsigned long i1, unsigned long i2, unsigned long i3)
{
    return impl->addTriangleByVertexIndex(i1, i2, i3);
}

unsigned long CTiglPolyObject::getNPointsOfPolygon(unsigned long ipoly) const 
{
    return impl->getNPointsOfPolygon(ipoly);
}

unsigned long CTiglPolyObject::getNPolygons() const
{
    return impl->getNPolygons();
}

unsigned long CTiglPolyObject::getNVertices() const
{
    return static_cast<unsigned long>(impl->pPoints.size());
}

void CTiglPolyObject::enableNormals(bool normals_enabled)
{
    impl->has_normals = normals_enabled;
}

bool CTiglPolyObject::hasNormals() const 
{
    return impl->has_normals;
}

unsigned long CTiglPolyObject::getVertexIndexOfPolygon(unsigned long iPoint, unsigned long iPoly) const 
{
    if (iPoly < getNPolygons()) {
        if (iPoint < getNPointsOfPolygon(iPoly)) {
            return impl->polys[iPoly].getPointIndex(iPoint);
        }
        else {
            throw tigl::CTiglError("Illegal Point Index at CTiglPolyObject::getVertexIndexOfPolygon", TIGL_INDEX_ERROR);
        }
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at CTiglPolyObject::getVertexIndexOfPolygon", TIGL_INDEX_ERROR);
    }
}

const CTiglPoint& CTiglPolyObject::getVertexNormal(unsigned long iVertexIndex) const 
{
    if (iVertexIndex < getNVertices()) {
        return impl->pPoints[iVertexIndex]->getNormal();
    }
    else {
        throw tigl::CTiglError("Illegal Vertex Index at CTiglPolyObject::getVertexNormal", TIGL_INDEX_ERROR);
    }
}

const CTiglPoint& CTiglPolyObject::getVertexPoint(unsigned long iVertexIndex) const 
{
    if (iVertexIndex < getNVertices()) {
        return impl->pPoints[iVertexIndex]->getPoint();
    }
    else {
        throw tigl::CTiglError("Illegal Vertex Index at CTiglPolyObject::getVertexNormal", TIGL_INDEX_ERROR);
    }
}

bool CTiglPolyObject::hasMetadata() const 
{
    return impl->has_metadata;
}

const char * CTiglPolyObject::getMetadataElements() const 
{
    return impl->metaDataElements.c_str();
}

void CTiglPolyObject::setMetadataElements(const char * elems) 
{
    impl->metaDataElements = elems;
}

const char * CTiglPolyObject::getPolyMetadata(unsigned long iPoly) const 
{
    if (iPoly < getNPolygons()) {
        return impl->polys[iPoly].getMetadata().c_str();
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at CTiglPolyObject::getPolyMetadata", TIGL_INDEX_ERROR);
    }
}

void CTiglPolyObject::setPolyMetadata(unsigned long iPoly, const char * txt)
{
    if (iPoly < getNPolygons()) {
        impl->polys[iPoly].setMetadata(txt);
        impl->has_metadata = true;
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at CTiglPolyObject::setPolyMetadata", TIGL_INDEX_ERROR);
    }
}

void CTiglPolyObject::setVertexDataReal(unsigned long iVertexIndex, const char *dataName, double value)
{
    if (iVertexIndex < getNVertices()) {
        //insert into elems
        impl->vertexDataElems.insert(dataName);
        
        impl->pPoints[iVertexIndex]->setRealData(dataName, value);
    }
    else {
        throw tigl::CTiglError("Illegal Vertex Index at CTiglPolyObject::setVertexDataReal", TIGL_INDEX_ERROR);
    }
}

double CTiglPolyObject::getVertexDataReal(unsigned long iVertexIndex, const char *dataName) const
{
    if (iVertexIndex < getNVertices()) {
        return impl->pPoints[iVertexIndex]->getRealData(dataName);
    }
    else {
        throw tigl::CTiglError("Illegal Vertex Index at CTiglPolyObject::getVertexDataReal", TIGL_INDEX_ERROR);
    }
}

void CTiglPolyObject::setPolyDataReal(unsigned long iPolyIndex, const char *dataName, double value)
{
    if (iPolyIndex < getNPolygons()) {
        //insert into elems
        impl->polyDataElems.insert(dataName);
        
        impl->polys[iPolyIndex].setRealData(dataName, value);
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at CTiglPolyObject::setPolyDataReal", TIGL_INDEX_ERROR);
    }
}

double CTiglPolyObject::getPolyDataReal(unsigned long iPolyIndex, const char *dataName) const
{
    if (iPolyIndex < getNPolygons()) {
        return impl->polys[iPolyIndex].getRealData(dataName);
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at CTiglPolyObject::getPolyDataReal", TIGL_INDEX_ERROR);
    }
}

// returns the number if different polygon data entries 
unsigned int CTiglPolyObject::getNumberOfPolyRealData() const 
{
    return static_cast<unsigned int>(impl->polyDataElems.size());
}

// retuns the  name of the ith data field (i = 0 .. getNumberPolyReadlData - 1)
const char * CTiglPolyObject::getPolyDataFieldName(unsigned long iField) const 
{
    if (iField < getNumberOfPolyRealData()) {
        std::set<std::string>::iterator it = impl->polyDataElems.begin();
        for (unsigned long i = 0; i < iField; ++i) {
            ++it;
        }
        return it->c_str();
    }
    else {
        throw tigl::CTiglError("Illegal Data Field Index at CTiglPolyObject::getPolyDataFieldName", TIGL_INDEX_ERROR);
    }
}


//---------------------------------------------------------------------------//

CTiglPolygon::CTiglPolygon()
{
    _metadata.clear();
    _points.clear();
    _normals.clear();
}

unsigned int CTiglPolygon::getNPoints() const
{
    return static_cast<unsigned int>(_points.size());
}

void CTiglPolygon::addPoint(const CTiglPoint & p)
{
    _points.push_back(p);
}

void CTiglPolygon::addNormal(const  CTiglPoint & n)
{
    _normals.push_back(n);
}

CTiglPoint& CTiglPolygon::getPoint(unsigned int index)
{
    return _points.at(index);
}

const CTiglPoint& CTiglPolygon::getPointConst(unsigned int index) const
{
    return _points.at(index);
}

const CTiglPoint& CTiglPolygon::getNormConst(unsigned int index) const
{
    return _normals.at(index);
}

void CTiglPolygon::setMetadata(const char * text)
{
    _metadata = text;
}

const char * CTiglPolygon::getMetadata() const
{
    return _metadata.c_str();
}

//--------------------------------------------------------------//

void ObjectImpl::addPointNorm(const CTiglPoint& p, const CTiglPoint& n, long polynum)
{
    unsigned long index = addPointNorm(p,n);

    // now insert point into polygon list
    if (polynum > (long) polys.size()) {
        polys.push_back(PolyIndexList(polynum));
    }
    assert(polynum == polys.size());
    polys.at(polynum-1).addPoint(index);
}

unsigned long ObjectImpl::addPointNorm(const CTiglPoint& p, const CTiglPoint& n) 
{
    //check if point is already in pointlist
    unsigned int index = static_cast<unsigned int>(pointlist.size());
    std::pair<PointMap::iterator,bool> ret;

    ret = pointlist.insert(std::pair<PointImpl, int>(PointImpl(p,n),index));
    index = ret.first->second;
/*#ifndef NDEBUG
    if(ret.second == false){
        double dist = ret.first->first.dist2(p);
        assert(pointlist.size() == 0 ||  dist < 1e-9);
    }
#endif*/
    if (ret.second == true) {
        // a new point was inserted, we have to keep track
        PointImpl * ptmp = (PointImpl*)&ret.first->first;
        pPoints.push_back(ptmp);
    }
    
    return index;
}

unsigned long ObjectImpl::addTriangleByVertexIndex(unsigned long i1, unsigned long i2, unsigned long i3 )
{
    // if we don't have a true triangle / 2 indices are the same, report error
    if (i1 == i2 || i1 == i3 || i2 == i3) {
        return ULONG_MAX;
    }

    unsigned long nPolys = static_cast<unsigned long>(polys.size());
    polys.push_back(PolyIndexList(nPolys));
    
    std::vector<PolyIndexList>::iterator itLastPoly = polys.end()-1;
    itLastPoly->addPoint(i1);
    itLastPoly->addPoint(i2);
    itLastPoly->addPoint(i3);
    
    return nPolys;
}


void ObjectImpl::addPolygon(const CTiglPolygon & poly)
{
    long polynum = static_cast<long>(polys.size()) + 1;
    if (!has_normals) {
        for (unsigned long i = 0 ; i < poly.getNPoints(); ++i) {
            CTiglPoint n(1,0,0);
            addPointNorm(poly.getPointConst(i), n, polynum);
        }
    }
    else {
        for (unsigned long i = 0 ; i < poly.getNPoints(); ++i) {
            addPointNorm(poly.getPointConst(i), poly.getNormConst(i), polynum);
        }
    }
    assert(polynum == polys.size());
    polys.at(polynum-1).setMetadata(poly.getMetadata());
}


unsigned long ObjectImpl::getNPolygons() const 
{
    return static_cast<unsigned long>(polys.size());
}

unsigned long ObjectImpl::getNPointsOfPolygon(unsigned long ipoly) const 
{
    if (ipoly < getNPolygons()) {
        return polys[ipoly].getNVert();
    }
    else {
        throw tigl::CTiglError("Illegal Polygon Index at SurfaceImpl::getNPointsOfPolygon", TIGL_INDEX_ERROR);
    }
}

