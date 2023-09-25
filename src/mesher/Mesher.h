#pragma once
#include "gmsh.h"
#include <string>
#include "TopoDS_Shape.hxx"

namespace tigl {


// Mesher is a singleton, that means there can ever be only one instance
class Mesher 
{
public: 
       struct Options
    {
        int dim;
        double minelementsize;
        double minelementsize2;
        double minelementsize3;
        double maxelementsize;
        double mindistance;
        double maxdistance;
    };

Mesher();
~Mesher();
void set_dimension(int dimension);
void set_minMeshSize(double min);
void set_maxMeshSize(double max);
void mesh() const;
void import(TopoDS_Shape shape) const;

void refine() const;
void write(std::string myName) const;
    

    Options options;
};
}