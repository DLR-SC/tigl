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

#include <iostream>
#include <cassert>
#include <vector>
#include <map>

#include <fstream>
#include <cfloat>

using namespace tigl;

// Comparer for gp_Pnts
struct TiglPointComparer
{
    
    // function should only return true, if lhs < rhs
    bool operator()(const CTiglPoint lhs, const CTiglPoint rhs) const
    {
        
        // if two points lie inside the epsilon environment
        // they should be the same, hence lhs is not smaller than rhs
        if(lhs.distance2(rhs) < 1e-10 )
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
};

typedef std::map<CTiglPoint, unsigned int, TiglPointComparer> PointMap;

struct PolyImpl{
    PolyImpl(int num){
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


class PolyDataImpl {
public:
    PolyDataImpl(){
        pointlist.clear();
        polys.clear();
        min_coord = DBL_MAX;
        max_coord = DBL_MIN;
    }
    void addPoint(const CTiglPoint &p, int polynum);
    void addPolygon(const CTiglPolygon&);

    void write_stream(std::ostream&);
    
    
    double min_coord;
    double max_coord;
    PointMap pointlist;
    std::vector<PolyImpl> polys;
};


CTiglPolyData::CTiglPolyData(): impl(new PolyDataImpl){
}

CTiglPolyData::~CTiglPolyData(){
    delete impl;
}

void CTiglPolyData::addPoint(const CTiglPoint &p, int id){
    impl->addPoint(p, id);
}

void CTiglPolyData::printVTK(){
    impl->write_stream(std::cout);
}

void CTiglPolyData::writeVTK(const char * filename){
    std::ofstream out(filename);
    if(!out.is_open()){
        std::cerr << "Cannot open file to write " << filename << std::endl;
        return;
    }
    impl->write_stream(out);
}

void CTiglPolyData::addPolygon(const CTiglPolygon & p){
    impl->addPolygon(p);
}

void PolyDataImpl::addPoint(const CTiglPoint& p, int polynum){
    using namespace std;
    //check if point is already in pointlist
    int index = pointlist.size();
    std::pair<PointMap::iterator,bool> ret;
    ret = pointlist.insert(std::pair<CTiglPoint, int>(p,index));
    index = ret.first->second;
#ifndef NDEBUG
    if(ret.second == false){
        double dist = ret.first->first.dist2(p);
        assert(pointlist.size() == 0 ||  dist < 1e-9);
    }
#endif
    if(polynum > (int) polys.size()){
        polys.push_back(PolyImpl(polynum));
    }
    assert(polynum == polys.size());
    polys.at(polynum-1).addPoint(index);

    double mmin, mmax;
    p.getMinMax(mmin, mmax);
    if(mmin < min_coord) min_coord = mmin;
    if(mmax > max_coord) max_coord = mmax;
}


void PolyDataImpl::addPolygon(const CTiglPolygon & poly){
    int polynum = polys.size() + 1;
    for(unsigned int i = 0 ; i < poly.getNPoints(); ++i){
        addPoint(poly.getPointConst(i), polynum);
    }
    assert(polynum = polys.size());
    polys.at(polynum-1).setMetadata(poly.getMetadata());
}


// we should switch to tixi ad some point
// to do so, we need function like tixiAddAttributeAtElementIndex as the DataArray node comes twice
void PolyDataImpl::write_stream(std::ostream& out ){
    using namespace std;
    std::vector<CTiglPoint> tmplist;
    tmplist.resize(pointlist.size());

    PointMap::iterator it = pointlist.begin();
    for(; it != pointlist.end(); ++it){
        int index = it->second;
        tmplist.at(index) = it->first;
    }

    std::vector<PolyImpl>::iterator pit = polys.begin();
    int nvert = 0;;
    for(; pit != polys.end(); ++pit){
        pit->repair();
        nvert += pit->getNVert();
    }

    out << "<?xml version=\"1.0\"?>"  << std::endl;
    out << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">" << std::endl;
    out << "  <PolyData>" << endl;
    out << "   <Piece NumberOfPoints=\"" << tmplist.size() <<  "\" NumberOfVerts=\"0\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"" << polys.size() << "\">"
        << std::endl;
    out << "      <Points>" << std::endl;
    out << "         <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\" RangeMin=\"" << min_coord << "\" RangeMax=\"" << max_coord << "\">"
        << std::endl;

    std::vector<CTiglPoint>::iterator tmpit = tmplist.begin();
    for(; tmpit != tmplist.end(); ++tmpit){
        out <<     "             ";
        out << tmpit->x << " " << tmpit->y << " " << tmpit->z << std::endl;
    }
    out << "         </DataArray>" << std::endl;
    out << "      </Points>" << std::endl;
    out << "      <Polys>" << std::endl;
    out << "         <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\"" << pointlist.size()-1 << "\">"  << std::endl;

    // write polygons
    pit = polys.begin();
    for(; pit != polys.end(); ++pit){
        out <<     "             ";
        for(int j = 0; j < pit->getNVert(); ++j ){
            out << pit->getPointIndex(j)<< " ";
        }
        out  << std::endl;
    }

    out << "         </DataArray>" << endl;
    out << "         <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\" RangeMin=\"" << polys.front().getNVert() << "\" RangeMax=\"" << nvert << "\">";
    //write offsets
    int next = 0;
    for (unsigned int i = 0; i < polys.size(); i ++)
    {
        if ((i % 10 == 0) && (i != (polys.size() - 1)))
        {
            out << endl << "            ";
        }
        int np = polys[i].getNVert();
        next += np;
        out << " " << next;
    }
    out << endl
        << "         </DataArray>" << endl;

    // write metadata
    out << "         <MetaData elements=\"uID segmentIndex eta xsi isOnTop\">" << endl;
    for (unsigned int i = 0; i < polys.size(); i ++){
        out << "             " << polys.at(i).getMetadata() << endl;
    }
    out <<  "        </MetaData>" << endl;
    out <<  "      </Polys>" << std::endl;
    out <<  "   </Piece>" << std::endl;
    out <<  "  </PolyData>" << std::endl;
    out <<  "</VTKFile>" << std::endl;
}

void PolyImpl::addPoint(int index){
    // dont add same point twice
    if(pindex.size() == 0 || pindex.back() != index){
         pindex.push_back(index);
    }
}

void PolyImpl::repair(){
    if(pindex.size() > 0 && pindex.front() == pindex.back()){
        pindex.pop_back();
    }
    if(pindex.size() < 3){
        std::cerr << "ERROR: polygon " << myid << " must contain at least 3 points!" << pindex.size()<< std::endl;
        exit(1);
    }
}

CTiglPolygon::CTiglPolygon(){
    _metadata.clear();
    _points.clear();
}

unsigned int CTiglPolygon::getNPoints() const{
    return _points.size();
}

void CTiglPolygon::addPoint(const CTiglPoint & p){
    _points.push_back(p);
}

CTiglPoint& CTiglPolygon::getPoint(unsigned int index){
    return _points.at(index);
}

const CTiglPoint& CTiglPolygon::getPointConst(unsigned int index) const{
    return _points.at(index);
}

void CTiglPolygon::setMetadata(const char * text){
    _metadata = text;
}

const char * CTiglPolygon::getMetadata() const{
    return _metadata.c_str();
}
