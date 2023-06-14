#pragma once
#include "gmsh.h"
#include <string>
#include "TopoDS_Shape.hxx"

namespace tigl {


// Mesher is a singleton, that means there can ever be only one instance
class Mesher 
{
public: 
    // delete copy constructor and copy-assignment operator:
      Mesher(Mesher const&) = delete;
      Mesher& operator=(Mesher const&) = delete;

      // delete move constructor and move-assignment operator:
      Mesher(Mesher&&) = delete;
      Mesher& operator=(Mesher&&) = delete;

      // factory function creates a static instance of Mesher
      // A Meyer's singleton comes with the following factory function:
      static Mesher& get_instance();

    struct Options
    {
        int dim;
        int minelementsize;
        int maxelementsize;
    };

Mesher();
~Mesher();
void set_options(int dimensions,int min, int max);
void mesh();
void import(TopoDS_Shape shape);
void refine();
void write(std::string myName);
    
private:
    Options options;
};
}
