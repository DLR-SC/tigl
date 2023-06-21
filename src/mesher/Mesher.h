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
void set_options(int dimensions,int min, int max);
void mesh() const;
void import(TopoDS_Shape shape) const;
void refine() const;
void write(std::string myName) const;
    
private:
    Options options;
};
}
