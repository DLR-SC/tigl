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
#include "tigl.h"
#include "tixi.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <utility>

#include <fstream>
#include <cfloat>
#include <cstdio>

using namespace tigl;

#ifdef _MSC_VER
    #define snprintf _snprintf
#endif


#define COMP_TOLERANCE 1.e-10

namespace {

struct TiglPointComparer;

class PointImpl {
public:
    PointImpl(){
        p_ = CTiglPoint(0,0,0);
        n_ = CTiglPoint(1,0,0);
    }

    PointImpl(const CTiglPoint& p, const CTiglPoint& n)
    : p_(p), n_(n) {}

    CTiglPoint& point(){ return p_; }
    CTiglPoint& normal(){ return n_; }

    const CTiglPoint& getPoint() const { return p_; }
    const CTiglPoint& getNormal()const { return n_; }

private:
    CTiglPoint p_;
    CTiglPoint n_;
};

typedef std::map<PointImpl, unsigned int, TiglPointComparer> PointMap;

// Comparer for gp_Pnts
struct TiglPointComparer
{
    bool isSmaller(const CTiglPoint& lhs, const CTiglPoint& rhs) const {
        // if two points lie inside the epsilon environment
        // they should be the same, hence lhs is not smaller than rhs
        if(lhs.distance2(rhs) < COMP_TOLERANCE )
            return false;


        // as lhs and rhs don't lie inside an epsilon environment
       if(lhs.x < rhs.x)
            return true;
        else if (lhs.x == rhs.x) {
            if(lhs.y < rhs.y)
                return true;
            else if(lhs.y == rhs.y)
                return lhs.z < rhs.z;
            else
                return false;
        }
        else
            return false;
    }


    // function should only return true, if lhs < rhs
    bool operator()(const PointImpl& lhs, const PointImpl& rhs) const{
        const CTiglPoint& leftP  = lhs.getPoint();
        const CTiglPoint& rightP = rhs.getPoint();

        const CTiglPoint& leftN  = lhs.getNormal();
        const CTiglPoint& rightN = rhs.getNormal();

        if(isSmaller(leftP,rightP)){
            return true;
        }
        else {
            // if points are equal, compare normals
            if(leftP.distance2(rightP) <  COMP_TOLERANCE ){
                return isSmaller(leftN, rightN);
            }
            else
                return false;
        }
    }
};


//class to store a polygon by its point indices
struct PolyIndexList{
    PolyIndexList(int num){
        myid = num;
        _metadata = "";
    }

    void setMetadata(const std::string& str){
        _metadata = str;
    }

    const std::string& getMetadata() const{
        return _metadata;
    }
    
    void addPoint(int index);
    
    void repair();
    
    int getNVert(){ return pindex.size();}
    int getPointIndex(int i) { return pindex.at(i); }
    
private:
    std::vector<int> pindex;

    //each polygon should have it's own metadata
    std::string _metadata;
    
private:
    int myid;
};


class SurfaceImpl {
public:
    SurfaceImpl(){
        pointlist.clear();
        polys.clear();
        pPoints.clear();
        min_coord = DBL_MAX;
        max_coord = DBL_MIN;
        has_normals = false;
    }
    void addPoint(const CTiglPoint &p, int polynum);
    void addPointNorm(const CTiglPoint &p, const CTiglPoint& norm, int polynum);
    void addPolygon(const CTiglPolygon&);

    void writeVTKPiece(TixiDocumentHandle handle, unsigned int iSurf);
    
    double min_coord;
    double max_coord;
    bool has_normals;

    // we use this container to store point data, it allows us to find
    // efficiently a specific point in the list
    PointMap pointlist;

    // this vector keeps track of the order of points, it complies with
    // the polygon indices
    std::vector<const PointImpl*> pPoints;

    std::vector<PolyIndexList> polys;
};

} // anonymous namespace

class PolyDataImpl{
public:
    PolyDataImpl(){
        _surfaces.clear();
        //create a default surface
        _surfaces.push_back(SurfaceImpl());

        itCurrentSurface = _surfaces.begin();
    }

    // creates a new surface, switches current surface to the new one
    // we store the polygon data as different surfaces
    // to allow hard edges
    void createNewSurface();

    // returns number of surfaces
    unsigned int getNSurfaces();

    void enableNormals(bool);

    unsigned int getNPolygons() const;

    unsigned int getNPointsOfPoly(unsigned int ipoly) const;

    const CTiglPoint& getPointOfPoly(unsigned int iPoint, unsigned int iPoly) const;

    // changes the current surface, we count from 1 to getNSurfaces
    void switchSurface(unsigned int iSurf);

    void addPoint(const CTiglPoint &p, int polynum);
    void addPolygon(const CTiglPolygon&);

    void writeVTK(TixiDocumentHandle handle);

private:
    std::vector<SurfaceImpl>::iterator itCurrentSurface;
    std::vector<SurfaceImpl> _surfaces;
};

// -----------------------------------------------------------------------//


CTiglPolyData::CTiglPolyData(): impl(new PolyDataImpl){
}

CTiglPolyData::~CTiglPolyData(){
    delete impl;
}

void CTiglPolyData::addPoint(const CTiglPoint &p, int id){
    impl->addPoint(p, id);
}

void CTiglPolyData::printVTK(){
    TixiDocumentHandle handle;
        tixiCreateDocument("VTKFile", &handle);
        impl->writeVTK(handle);

        char * text = NULL;
        tixiExportDocumentAsString(handle, &text);

        cout << text << endl;

        tixiCloseDocument(handle);
}

void CTiglPolyData::writeVTK(const char * filename){
    TixiDocumentHandle handle;
    tixiCreateDocument("VTKFile", &handle);
    impl->writeVTK(handle);

    if(tixiSaveDocument(handle, (char*) filename)!= SUCCESS){
        tixiCloseDocument(handle);
        throw CTiglError("Error writing VTK file in CTiglPolyData::writeVTK", TIGL_ERROR);
    }else
        tixiCloseDocument(handle);
}

void CTiglPolyData::addPolygon(const CTiglPolygon & p){
    impl->addPolygon(p);
}

void CTiglPolyData::createNewSurface(){
    impl->createNewSurface();
}

void CTiglPolyData::enableNormals(bool enable){
    impl->enableNormals(enable);
}

unsigned int CTiglPolyData::getNPolygons() const{
    return impl->getNPolygons();
}

// returns the number of points of the polygon
unsigned int CTiglPolyData::getNPointsOfPoly(unsigned int ipoly) const {
    return impl->getNPointsOfPoly(ipoly);
}

const CTiglPoint& CTiglPolyData::getPointOfPoly(unsigned int iPoint, unsigned int iPoly) const {
    return impl->getPointOfPoly(iPoint,  iPoly);
}

//--------------------------------------------------------------------------//

CTiglPolygon::CTiglPolygon(){
    _metadata.clear();
    _points.clear();
    _normals.clear();
}

unsigned int CTiglPolygon::getNPoints() const{
    return _points.size();
}

void CTiglPolygon::addPoint(const CTiglPoint & p){
    _points.push_back(p);
}

void CTiglPolygon::addNormal(const  CTiglPoint & n){
    _normals.push_back(n);
}

CTiglPoint& CTiglPolygon::getPoint(unsigned int index){
    return _points.at(index);
}

const CTiglPoint& CTiglPolygon::getPointConst(unsigned int index) const{
    return _points.at(index);
}

const CTiglPoint& CTiglPolygon::getNormConst(unsigned int index) const{
    return _normals.at(index);
}

void CTiglPolygon::setMetadata(const char * text){
    _metadata = text;
}

const char * CTiglPolygon::getMetadata() const{
    return _metadata.c_str();
}

// ----------------------------------------------------------------------  //

// changes the current surface, we count from 1 to getNSurfaces
void PolyDataImpl::switchSurface(unsigned int iSurf){
    if(iSurf >= 1 && iSurf <= _surfaces.size()){
        itCurrentSurface = _surfaces.begin() + iSurf - 1;
    }
    else {
        throw tigl::CTiglError("Invalid surface index in PolyDataImpl::switchSurface!", TIGL_INDEX_ERROR);
    }
}

void PolyDataImpl::enableNormals(bool enable){
    if(itCurrentSurface!=_surfaces.end())
            itCurrentSurface->has_normals = enable;
        else
            throw tigl::CTiglError("Error: current surface is NULL in PolyDataImpl::enableNormals ", TIGL_NULL_POINTER);
}

void PolyDataImpl::addPoint(const CTiglPoint &p, int polynum){
    if(itCurrentSurface!=_surfaces.end())
        itCurrentSurface->addPoint(p, polynum);
    else
        throw tigl::CTiglError("Error: current surface is NULL in PolyDataImpl::addPoint ", TIGL_NULL_POINTER);
}

void PolyDataImpl::addPolygon(const CTiglPolygon& poly){
    if(itCurrentSurface!=_surfaces.end())
        itCurrentSurface->addPolygon(poly);
    else
        throw tigl::CTiglError("Error: current surface is NULL in PolyDataImpl::addPolygon ", TIGL_NULL_POINTER);
}

void PolyDataImpl::writeVTK(TixiDocumentHandle handle){
    tixiAddTextAttribute(handle, "/VTKFile", "type", "PolyData");
    tixiAddTextAttribute(handle, "/VTKFile", "version", "0.1");
    tixiAddTextAttribute(handle, "/VTKFile", "byte_order", "LittleEndian");
    tixiAddTextAttribute(handle, "/VTKFile", "compressor", "vtkZLibDataCompressor");

    tixiCreateElement(handle, "/VTKFile", "PolyData");

    std::vector<SurfaceImpl>::iterator it = _surfaces.begin();
    unsigned int i = 1;
    for(;it != _surfaces.end(); ++it, ++i){
        it->writeVTKPiece(handle, i);
    }
}

void PolyDataImpl::createNewSurface(){
    _surfaces.push_back(SurfaceImpl());
    itCurrentSurface = _surfaces.end()-1;
}

unsigned int PolyDataImpl::getNSurfaces(){
    return _surfaces.size();
}

unsigned int PolyDataImpl::getNPolygons() const{
    return itCurrentSurface->polys.size();
}

unsigned int PolyDataImpl::getNPointsOfPoly(unsigned int ipoly) const{
    if(ipoly >= 0 && ipoly < getNPolygons())
        return itCurrentSurface->polys[ipoly].getNVert();
    else
        throw tigl::CTiglError("Illegal Polygon Index at PolyDataImpl::getNPointsOfPoly", TIGL_INDEX_ERROR);
}

const CTiglPoint& PolyDataImpl::getPointOfPoly(unsigned int ipoint, unsigned int ipoly) const {
    if(ipoly >= 0 && ipoly < getNPolygons()){
        if (ipoint >= 0 && ipoint < getNPointsOfPoly(ipoly)){
            int pointIndex = itCurrentSurface->polys[ipoly].getPointIndex(ipoint);
            return itCurrentSurface->pPoints[pointIndex]->getPoint();
        }
        else
            throw tigl::CTiglError("Illegal Point Index at PolyDataImpl::getPointOfPoly", TIGL_INDEX_ERROR);
    }
    else
        throw tigl::CTiglError("Illegal Polygon Index at PolyDataImpl::getPointOfPoly", TIGL_INDEX_ERROR);
}

//--------------------------------------------------------------//


void SurfaceImpl::addPoint(const CTiglPoint& p, int polynum){
    using namespace std;
    CTiglPoint n(1,0,0);
    //check if point is already in pointlist
    int index = pointlist.size();
    std::pair<PointMap::iterator,bool> ret;
    ret = pointlist.insert(std::pair<PointImpl, int>( PointImpl(p,n),index));
    index = ret.first->second;
#ifndef NDEBUG
    if(ret.second == false){
        double dist = ret.first->first.dist2(p);
        assert(pointlist.size() == 0 ||  dist < 1e-9);
    }
#endif
    if(ret.second == true){
        // a new point was inserted, we have to keep track
        pPoints.push_back(&(ret.first->first));
    }

    if(polynum > (int) polys.size()){
        polys.push_back(PolyIndexList(polynum));
    }

    assert(polynum == polys.size());
    polys.at(polynum-1).addPoint(index);

    double mmin, mmax;
    p.getMinMax(mmin, mmax);
    if(mmin < min_coord) min_coord = mmin;
    if(mmax > max_coord) max_coord = mmax;
}

void SurfaceImpl::addPointNorm(const CTiglPoint& p, const CTiglPoint& n, int polynum){
    using namespace std;
    //check if point is already in pointlist
    int index = pointlist.size();
    std::pair<PointMap::iterator,bool> ret;

    ret = pointlist.insert(std::pair<PointImpl, int>(PointImpl(p,n),index));
    index = ret.first->second;
#ifndef NDEBUG
    if(ret.second == false){
        double dist = ret.first->first.dist2(p);
        assert(pointlist.size() == 0 ||  dist < 1e-9);
    }
#endif
    if(ret.second == true){
        // a new point was inserted, we have to keep track
        pPoints.push_back(&(ret.first->first));
    }

    // now insert point into polygon list
    if(polynum > (int) polys.size()){
        polys.push_back(PolyIndexList(polynum));
    }
    assert(polynum == polys.size());
    polys.at(polynum-1).addPoint(index);

    double mmin, mmax;
    p.getMinMax(mmin, mmax);
    if(mmin < min_coord) min_coord = mmin;
    if(mmax > max_coord) max_coord = mmax;
}


void SurfaceImpl::addPolygon(const CTiglPolygon & poly){
    int polynum = polys.size() + 1;
    if(!has_normals){
        for(unsigned int i = 0 ; i < poly.getNPoints(); ++i){
            addPoint(poly.getPointConst(i), polynum);
        }
    }
    else {
        for(unsigned int i = 0 ; i < poly.getNPoints(); ++i){
            addPointNorm(poly.getPointConst(i), poly.getNormConst(i), polynum);
        }
    }
    assert(polynum = polys.size());
    polys.at(polynum-1).setMetadata(poly.getMetadata());
}


// writes the polygon data of a surface (in vtk they call it piece)
void SurfaceImpl::writeVTKPiece(TixiDocumentHandle handle, unsigned int iSurf){
    using namespace std;
    if(pointlist.size() <= 0){
        return;
    }

    // count number of vertices - this is not necessarily the number of points
    std::vector<PolyIndexList>::iterator pit = polys.begin();
    int nvert = 0;;
    for(; pit != polys.end(); ++pit){
        pit->repair();
        nvert += pit->getNVert();
    }

    if(nvert <= 0){
        return;
    }

    // surface specific stuff
    tixiCreateElement(handle, "/VTKFile/PolyData","Piece");

    char piecepath[512];
    snprintf(piecepath, 512, "/VTKFile/PolyData/Piece[%d]", iSurf);

    tixiAddIntegerAttribute(handle, piecepath, "NumberOfPoints", pPoints.size(), "%d");
    tixiAddIntegerAttribute(handle, piecepath, "NumberOfVerts",  0, "%d");
    tixiAddIntegerAttribute(handle, piecepath, "NumberOfLines",  0, "%d");
    tixiAddIntegerAttribute(handle, piecepath, "NumberOfStrips", 0, "%d");
    tixiAddIntegerAttribute(handle, piecepath, "NumberOfPolys", polys.size(), "%d");

    tixiCreateElement(handle, piecepath, "Points");

    //points
    {
    std::vector<const PointImpl*>::iterator tmpit = pPoints.begin();
    std::stringstream stream1;
    stream1 << std::endl <<   "         ";
    for(; tmpit != pPoints.end(); ++tmpit){
        const CTiglPoint& p = (*tmpit)->getPoint();
        stream1 << "    " <<  p.x << " " << p.y << " " << p.z << std::endl;
        stream1 << "         ";
    }
    char tmpPath[512];
    snprintf(tmpPath, 512, "%s/Points", piecepath);
    tixiAddTextElement(handle, tmpPath, "DataArray", stream1.str().c_str());
    snprintf(tmpPath, 512,  "%s/DataArray", tmpPath);
    tixiAddTextAttribute(handle, tmpPath, "type", "Float64");
    tixiAddTextAttribute(handle, tmpPath, "Name", "Points");
    tixiAddTextAttribute(handle, tmpPath, "NumberOfComponents", "3");
    tixiAddTextAttribute(handle, tmpPath, "format", "ascii");
    tixiAddDoubleAttribute(handle, tmpPath, "RangeMin", min_coord ,"%f");
    tixiAddDoubleAttribute(handle, tmpPath, "RangeMax", max_coord ,"%f");
    }

    //normals
    if(has_normals){
        tixiCreateElement(handle, piecepath, "PointData");
        char tmpPath[512];
        snprintf(tmpPath, 512, "%s/PointData", piecepath);

        tixiAddTextAttribute(handle, tmpPath, "Normals", "surf_normals");

        stringstream stream;
        stream << endl  << "        ";
        std::vector<const PointImpl*>::iterator it = pPoints.begin();
        for(; it != pPoints.end(); ++it){
             const CTiglPoint& n = (*it)->getNormal();
             stream << "    " << n.x << " " << n.y << " "  << n.z << endl;
             stream <<  "        ";
        }

        tixiAddTextElement(handle, tmpPath, "DataArray", stream.str().c_str());
        snprintf(tmpPath, 512, "%s/DataArray", tmpPath);
        tixiAddTextAttribute(handle, tmpPath, "type", "Float64");
        tixiAddTextAttribute(handle, tmpPath, "Name", "surf_normals");
        tixiAddTextAttribute(handle, tmpPath, "NumberOfComponents", "3");
        tixiAddTextAttribute(handle, tmpPath, "format", "ascii");
        tixiAddDoubleAttribute(handle, tmpPath, "RangeMin", 0 ,"%f");
        tixiAddDoubleAttribute(handle, tmpPath, "RangeMax", 1 ,"%f");
    }

    //polygons
    {
    tixiCreateElement(handle, piecepath, "Polys");
    pit = polys.begin();
    std::stringstream stream2;
    stream2 << std::endl <<   "        ";
    for(; pit != polys.end(); ++pit){
        stream2 <<     "    ";
        for(int j = 0; j < pit->getNVert(); ++j ){
            stream2 << pit->getPointIndex(j)<< " ";
        }
        stream2  << std::endl <<  "        ";;
    }

    char tmpPath[512];
    snprintf(tmpPath, 512, "%s/Polys", piecepath);
    tixiAddTextElement(handle, tmpPath, "DataArray", stream2.str().c_str());
    snprintf(tmpPath, 512, "%s/DataArray", tmpPath);
    tixiAddTextAttribute(handle,tmpPath, "type", "Int32");
    tixiAddTextAttribute(handle, tmpPath, "Name", "connectivity");
    tixiAddTextAttribute(handle, tmpPath, "format", "ascii");
    tixiAddIntegerAttribute(handle, tmpPath, "RangeMin", 0 ,"%d");
    tixiAddIntegerAttribute(handle, tmpPath, "RangeMax", (int)pointlist.size()-1,"%d");
    }

    //offset
    {
    int next = 0;
    std::stringstream stream3;
    for (unsigned int i = 0; i < polys.size(); i ++)
    {
        if ((i % 10 == 0) && (i != (polys.size() - 1)))
        {
            stream3 << endl << "            ";
        }
        int np = polys[i].getNVert();
        next += np;
        stream3 << " " << next;
    }
    stream3 << endl << "        ";
    char tmpPath[512];
    snprintf(tmpPath, 512, "%s/Polys", piecepath);
    tixiAddTextElement(handle, tmpPath, "DataArray", stream3.str().c_str());
    snprintf(tmpPath, 512, "%s/DataArray[2]", tmpPath);
    tixiAddTextAttribute(handle, tmpPath, "type", "Int32");
    tixiAddTextAttribute(handle, tmpPath, "Name", "offsets");
    tixiAddIntegerAttribute(handle, tmpPath, "RangeMin", polys.front().getNVert() ,"%d");
    tixiAddIntegerAttribute(handle, tmpPath, "RangeMax", nvert,"%d");
    }

    // write metadata
    {
    std::stringstream stream4;
    stream4 << endl  << "        ";
    for (unsigned int i = 0; i < polys.size(); i ++){
        stream4 << "    " << polys.at(i).getMetadata() << endl;
        stream4 << "        ";
    }
    char tmpPath[512];
    snprintf(tmpPath, 512, "%s/Polys", piecepath);
    tixiAddTextElement(handle, tmpPath, "MetaData", stream4.str().c_str());
    snprintf(tmpPath, 512, "%s/MetaData", tmpPath);
    tixiAddTextAttribute(handle,  tmpPath, "elements", "uID segmentIndex eta xsi isOnTop" );
    }

}

// --------------------------------------------------------------------------//

void PolyIndexList::addPoint(int index){
    // dont add same point twice
    if(pindex.size() == 0 || pindex.back() != index){
         pindex.push_back(index);
    }
}

void PolyIndexList::repair(){
    if(pindex.size() > 0 && pindex.front() == pindex.back()){
        pindex.pop_back();
    }
    if(pindex.size() < 3){
        std::cerr << "ERROR: polygon " << myid << " must contain at least 3 points!" << pindex.size()<< std::endl;
        exit(1);
    }
}


