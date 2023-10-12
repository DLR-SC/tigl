#include "gmsh.h"
#include "Mesher.h"
#include <string>

namespace tigl {

Mesher::Mesher() { gmsh::initialize(); }
Mesher::~Mesher(){ gmsh::finalize(); }

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

void Mesher::set_minDistance(double min_dist)
{
    options.mindistance = min_dist;
}

void Mesher::set_maxDistance(double max_dist)
{
    options.maxdistance = max_dist;
}

Mesher::FieldOptions& Mesher::get_field_options(std::string const& field_name)
{
    return options.field_options[field_name];
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


void Mesher::mesh_fields() const
{
    gmsh::model::mesh::field::add("Min", num + 1 );
    gmsh::model::mesh::field::setNumbers(num + 1, "FieldsList", Fields);
    gmsh::model::mesh::field::setAsBackgroundMesh(num + 1);

    gmsh::option::setNumber("Mesh.MeshSizeExtendFromBoundary", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromPoints", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", 0);

   
    
    gmsh::model::mesh::generate(options.dim);
}


// is used to refine the mesh at given curves (can be extended to points or planes)
// anderer Weg wäre meshsize (dann allerdings Vektor mit allen Punkten und der zugehörigen Mesh Size)
void Mesher::refineEdge( std::vector<double> Edge, std::string field_name)
{
    
    Fields.push_back(num + 1);

    get_field_options(field_name).field_number = Fields.back() - 1;

    num = num + 2;

    gmsh::model::mesh::field::add("Distance", get_field_options(field_name).field_number);
    gmsh::model::mesh::field::setNumbers(get_field_options(field_name).field_number, "CurvesList", Edge);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number, "Sampling", 100);

    gmsh::model::mesh::field::add("Threshold", get_field_options(field_name).field_number + 1);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "InField", get_field_options(field_name).field_number);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "SizeMin", get_field_options(field_name).min_element_size);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "SizeMax", options.maxelementsize);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "DistMin", options.mindistance);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "DistMax", options.maxdistance);

    gmsh::model::occ::synchronize();
    
}

void Mesher::refineSurface( std::vector<double> Surface, std::string field_name)
{
    
    Fields.push_back(num + 1);

    get_field_options(field_name).field_number = Fields.back() - 1;

    num = num + 2;

    gmsh::model::mesh::field::add("Distance", get_field_options(field_name).field_number);
    gmsh::model::mesh::field::setNumbers(get_field_options(field_name).field_number, "SurfacesList", Surface);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number, "Sampling", 100);

    gmsh::model::mesh::field::add("Threshold", get_field_options(field_name).field_number + 1);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "InField", get_field_options(field_name).field_number);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "SizeMin", get_field_options(field_name).min_element_size);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "SizeMax", options.maxelementsize);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "DistMin", options.mindistance);
    gmsh::model::mesh::field::setNumber(get_field_options(field_name).field_number + 1, "DistMax", options.maxdistance);

    gmsh::model::occ::synchronize();
    
}


// writes the mesh in the given format 
void Mesher::write(std::string myName) const
{

    gmsh::write(myName.c_str());
    // maybe automatically export a brep too 

}
}
