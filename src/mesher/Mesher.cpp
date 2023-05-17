#include "gmsh.h"
#include "Mesher.h"
#include <string>


Mesher::Mesher() { gmsh::initialize(); }
Mesher::~Mesher(){ gmsh::finalize(); }

//Options: dimensions: digit 1-3, myName: "filename.format"
void Mesher::set_options(int dimensions, std::string myName)
{
   
    options.setDimension(dimensions);
    //options.setSize(mySize);
    options.setName(myName);

}

void Mesher::Options::setDimension(int dimension)
{
    dim = dimension;
}

int Mesher::Options::getDimension()
{
    return dim;
}

//void Mesher::Options::setSize(int mySize)
//{
//    size = mySize;
//}

//int Mesher::Options::getSize()
//{
//    return size;
//}

void Mesher::Options::setName(std::string myName)
{
    fileName = myName;
}

std::string Mesher::Options::getName()
{
    return fileName;
}

// imports an TopoDS_Shape by using a pointer to void
void Mesher::import(TopoDS_Shape shape)
{
    gmsh::vectorpair outDimTags;
    gmsh::model::occ::importShapesNativePointer((void const*)&shape, outDimTags);
    gmsh::model::occ::synchronize();

}

// meshes the previous called Shape (currently without a size option) and save it as .msh
void Mesher::mesh()
{
//    std::vector<std::pair<int, int> > out;
//    gmsh::model::mesh::setSize(out, options.getSize());

    gmsh::model::mesh::generate(options.getDimension());

    gmsh::write(options.getName().c_str());
}

// refines the mesh by splitting the Elemnts and saves it as .msh
void Mesher::refine()
{
    gmsh::model::mesh::refine();
    
    gmsh::write(options.getName().c_str());
}


