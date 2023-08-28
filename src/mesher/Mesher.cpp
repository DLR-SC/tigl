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

//Options: dimensions: digit 1-3, min and max elementsize (0-1e+22)
void Mesher::set_options(int dimensions, int min, int max, bool adapted_mesh )
{
    options.dim = dimensions;
    options.minelementsize = min;
    options.maxelementsize = max;

if (adapted_mesh == false)
{
    gmsh::option::setNumber("Mesh.MeshSizeMin", options.minelementsize);
    gmsh::option::setNumber("Mesh.MeshSizeMax", options.maxelementsize);
}

}


// imports an TopoDS_Shape by using a pointer to void
void Mesher::import(TopoDS_Shape shape) const
{
    gmsh::vectorpair outDimTags;
    gmsh::model::occ::importShapesNativePointer((void const*)&shape, outDimTags);
    gmsh::model::occ::synchronize();

}

void Mesher::leading_trailingEdge( std::vector<double> Edge, const int SizeMin, const int SizeMax, const int DistMin, const int DistMax)
{
        gmsh::model::mesh::field::add("Distance", 1);
    gmsh::model::mesh::field::setNumbers(1, "CurvesList", Edge);
    gmsh::model::mesh::field::setNumber(1, "Sampling", 100);

    gmsh::model::mesh::field::add("Threshold", 2);
    gmsh::model::mesh::field::setNumber(2, "InField", 1);
    gmsh::model::mesh::field::setNumber(2, "SizeMin", SizeMin);
    gmsh::model::mesh::field::setNumber(2, "SizeMax", SizeMax);
    gmsh::model::mesh::field::setNumber(2, "DistMin", DistMin);
    gmsh::model::mesh::field::setNumber(2, "DistMax", DistMax);

    gmsh::model::mesh::field::add("Min", 3);
    gmsh::model::mesh::field::setNumbers(3, "FieldsList", {2});
    gmsh::model::mesh::field::setAsBackgroundMesh(3);


    gmsh::option::setNumber("Mesh.MeshSizeExtendFromBoundary", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromPoints", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", 0);
}


// meshes the previous called Shape (currently without a size option) and save it as .msh
void Mesher::mesh() const
{  
    gmsh::model::mesh::generate(options.dim);

}

// refines the mesh by splitting the Elemnts and saves it as .msh
void Mesher::refine() const
{
    gmsh::model::mesh::refine();
    
}

void Mesher::write(std::string myName) const
{

    gmsh::write(myName.c_str());

}
}
