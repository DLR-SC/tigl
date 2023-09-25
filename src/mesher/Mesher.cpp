#include "gmsh.h"
#include "Mesher.h"
#include <string>

namespace tigl {

Mesher::Mesher() { gmsh::initialize(); }
Mesher::~Mesher(){ gmsh::finalize(); }

static Mesher& get_instance()
{
        static Mesher m;
        return m;
}

// Options: 
void Mesher::set_dimension(int dimension)
{
    options.dim = dimension;
}

void Mesher::set_minMeshSize(double min)
{
    options.minelementsize = min;
}

void Mesher::set_maxMeshSize(double max)
{
    options.maxelementsize = max;
}

// imports an TopoDS_Shape by using a pointer to void
void Mesher::import(TopoDS_Shape shape) const
{
    gmsh::vectorpair outDimTags;
    gmsh::model::occ::importShapesNativePointer((void const*)&shape, outDimTags);
    gmsh::model::occ::synchronize();

}


// meshes the previous called Shape (currently without a size option) and save it as .msh
void Mesher::mesh() const
{  
    gmsh::option::setNumber("Mesh.MeshSizeMin", options.minelementsize);
    gmsh::option::setNumber("Mesh.MeshSizeMax", options.maxelementsize);
    gmsh::model::mesh::generate(options.dim);

}

// refines the mesh by splitting the Elemnts and saves it as .msh
void Mesher::refine() const
{
    gmsh::model::mesh::refine();
    
}

// writes the mesh in the given format 
void Mesher::write(std::string myName) const
{

    gmsh::write(myName.c_str());
    // maybe automatically export a brep too 

}
}
