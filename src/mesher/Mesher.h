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
        int minelementsize;
        int maxelementsize;
    };

Mesher();
~Mesher();
void set_options(int dimensions,int min, int max, bool adapted_mesh);
void mesh() const;
void import(TopoDS_Shape shape) const;
void leading_trailingEdge( std::vector<double> Edge, const int SizeMin = 0.1, const int SizeMax = 1, const int DistMin = 0.5, const int DistMax = 1);
void refine() const;
void write(std::string myName) const;
    
private:
    Options options;
};
}
