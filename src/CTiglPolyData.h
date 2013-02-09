#ifndef VTKPOLYDATA_H
#define VTKPOLYDATA_H

#include <ostream>

class PolyPoint{
public:
    PolyPoint(){
        x = 0.; y = 0.; z = 0.;
    }
    
    PolyPoint(double X,double Y,double Z){
        x = X; y = Y; z = Z;
    }
    
    // returns sqare distance to another point
    double dist2(const PolyPoint& p) const {
        return (x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) + (z-p.z)*(z-p.z);
    }
    
    void print()const;
    
    void getMinMax(double & min, double & max) const;
    
    double x,y,z;
};


class PolyData{
public:
    PolyData();
    ~PolyData();
    
    void addPoint(const PolyPoint& p, int polynum);
    
    void writeVTK(const char * filename);
    void printVTK();
    
private:
    class PolyDataImpl * impl;
};

#endif // VTKPOLYDATA_H
