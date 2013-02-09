#include "CTiglPolyData.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <map>

#include <fstream>
#include <cfloat>

// Comparer for gp_Pnts
struct gp_PntEquals
{
    
    // function should only return true, if lhs < rhs
    bool operator()(const PolyPoint lhs, const PolyPoint rhs) const
    {
        
        // if two points lie inside the epsilon environment
        // they should be the same, hence lhs is not smaller than rhs
        if(lhs.dist2(rhs) < 1e-10 )
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

typedef std::map<PolyPoint, unsigned int, gp_PntEquals> PointMap;

struct PolyImpl{
    PolyImpl(int num){ myid = num; }
    
    void addPoint(int index);
    
    void repair();
    
    int getNVert(){ return pindex.size();}
    int getPointIndex(int i) { return pindex.at(i); }
    
private:
    std::vector<int> pindex;
    
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
    void addPoint(const PolyPoint& p, int polynum);
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

void CTiglPolyData::addPoint(const PolyPoint &p, int num){
    impl->addPoint(p, num);
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

void PolyDataImpl::addPoint(const PolyPoint& p, int polynum){
    using namespace std;
    //check if point is already in pointlist
    int index = pointlist.size();
    std::pair<PointMap::iterator,bool> ret;
    ret = pointlist.insert(std::pair<PolyPoint, int>(p,index));
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

// we should switch to tixi ad some point
// to do so, we need function like tixiAddAttributeAtElementIndex as the DataArray node comes twice
void PolyDataImpl::write_stream(std::ostream& out ){
    using namespace std;
    std::vector<PolyPoint> tmplist;
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

    std::vector<PolyPoint>::iterator tmpit = tmplist.begin();
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
            out << endl << "             ";
        }
        int np = polys[i].getNVert();
        next += np;
        out << " " << next;
    }
    out << endl << "         </DataArray>" << endl;

    out <<         "      </Polys>" << std::endl;
    out <<         "   </Piece>" << std::endl;
    out <<         "  </PolyData>" << std::endl;
    out <<         "</VTKFile>" << std::endl;
}

void PolyPoint::print()const{
    std::cout << "(" << x << "," << y << "," << z << ")\n";
}

void PolyPoint::getMinMax(double & min, double & max) const {
    min = x;
    if(y < min) min = y;
    if(z < min) min = z;
    max = x;
    if(y > max) max = y;
    if(z > max) max = z;
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
