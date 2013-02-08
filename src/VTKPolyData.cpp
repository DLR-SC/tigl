#include "vtkpolydata.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <map>

#include <fstream>

void VTKPolyData::addPoint(const Point& p, int polynum){
    using namespace std;
    //check if point is already in pointlist
    int index = pointlist.size();
    std::pair<PointMap::iterator,bool> ret;
    ret = pointlist.insert(std::pair<Point, int>(p,index));
    index = ret.first->second;
#ifndef NDEBUG
    if(ret.second == false){
        double dist = ret.first->first.dist2(p);
        assert(pointlist.size() == 0 ||  dist < 1e-9);
    }
#endif
    if(polynum > (int) polys.size()){
        polys.push_back(Polygon(polynum));
    }
    assert(polynum == polys.size());
    polys.at(polynum-1).addPoint(index);

    double mmin, mmax;
    p.getMinMax(mmin, mmax);
    if(mmin < min_coord) min_coord = mmin;
    if(mmax > max_coord) max_coord = mmax;
}

void VTKPolyData::write(const char * filename){
    std::ofstream out(filename);
    if(!out.is_open()){
        std::cerr << "Cannot open file to write " << filename << std::endl;
        return;
    }
    write_stream(out);
}

void VTKPolyData::print(){
    write_stream(std::cout);
}

// we should switch to tixi ad some point
// to do so, we need function like tixiAddAttributeAtElementIndex as the DataArray node comes twice
void VTKPolyData::write_stream(std::ostream& out ){
    using namespace std;
    std::vector<Point> tmplist;
    tmplist.resize(pointlist.size());

    PointMap::iterator it = pointlist.begin();
    for(; it != pointlist.end(); ++it){
        int index = it->second;
        tmplist.at(index) = it->first;
    }

    std::vector<Polygon>::iterator pit = polys.begin();
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

    std::vector<Point>::iterator tmpit = tmplist.begin();
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

void Point::print()const{
    std::cout << "(" << x << "," << y << "," << z << ")\n";
}

void Point::getMinMax(double & min, double & max) const {
    min = x;
    if(y < min) min = y;
    if(z < min) min = z;
    max = x;
    if(y > max) max = y;
    if(z > max) max = z;
}

void Polygon::addPoint(int index){
    // dont add same point twice
    if(pindex.size() == 0 || pindex.back() != index){
         pindex.push_back(index);
    }
}

void Polygon::repair(){
    if(pindex.size() > 0 && pindex.front() == pindex.back()){
        pindex.pop_back();
    }
    if(pindex.size() < 3){
        std::cerr << "ERROR: polygon " << myid << " must contain at least 3 points!" << pindex.size()<< std::endl;
        exit(1);
    }
}
