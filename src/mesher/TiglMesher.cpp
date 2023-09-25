#include "TiglMesher.h"

namespace tigl {

TiglMesher::TiglMesher() {}
TiglMesher::~TiglMesher(){}

Mesher m;

void TiglMesher::set_dimension(int dimension)
{
    m.set_dimension(dimension);
}

void TiglMesher::set_minMeshSize1(double min)
{
    m.set_minMeshSize( min);
}

void TiglMesher::set_minMeshSize2(double min2)
{
    m.options.minelementsize2 = min2;
}

void TiglMesher::set_minMeshSizeSurface(double min3)
{
    m.options.minelementsize3 = min3;
}

void TiglMesher::set_maxMeshSize(double max)
{
    m.set_maxMeshSize( max);
}

void TiglMesher::set_minDistance(double DistMin)
{
    m.options.mindistance = DistMin;
}

void TiglMesher::set_maxDistance(double DistMax)
{
    m.options.maxdistance = DistMax;
}

void TiglMesher::import(TopoDS_Shape shape) const
{
    m.import(shape);
}

void TiglMesher::mesh() const
{
    gmsh::model::mesh::field::add("Min", 7);
    gmsh::model::mesh::field::setNumbers(7, "FieldsList", {2,4,6});
    gmsh::model::mesh::field::setAsBackgroundMesh(7);

    gmsh::option::setNumber("Mesh.MeshSizeExtendFromBoundary", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromPoints", 0);
    gmsh::option::setNumber("Mesh.MeshSizeFromCurvature", 0);
    gmsh::model::mesh::generate(m.options.dim);
}


// is used to refine the mesh at given curves (can be extended to points or planes)
// anderer Weg wäre meshsize (dann allerdings Vektor mit allen Punkten und der zugehörigen Mesh Size)
void TiglMesher::refineEdge1( std::vector<double> Edge)
{
    gmsh::model::mesh::field::add("Distance", 1);
    gmsh::model::mesh::field::setNumbers(1, "CurvesList", Edge);
    gmsh::model::mesh::field::setNumber(1, "Sampling", 100);

    gmsh::model::mesh::field::add("Threshold", 2);
    gmsh::model::mesh::field::setNumber(2, "InField", 1);
    gmsh::model::mesh::field::setNumber(2, "SizeMin", m.options.minelementsize);
    gmsh::model::mesh::field::setNumber(2, "SizeMax", m.options.maxelementsize);
    gmsh::model::mesh::field::setNumber(2, "DistMin", m.options.mindistance);
    gmsh::model::mesh::field::setNumber(2, "DistMax", m.options.maxdistance);

    gmsh::model::occ::synchronize();
}
void TiglMesher::refineEdge2( std::vector<double> Edge2)
{
    gmsh::model::mesh::field::add("Distance", 3);
    gmsh::model::mesh::field::setNumbers(3, "CurvesList", Edge2);
    gmsh::model::mesh::field::setNumber(3, "Sampling", 100);

    gmsh::model::mesh::field::add("Threshold", 4);
    gmsh::model::mesh::field::setNumber(4, "InField", 3);
    gmsh::model::mesh::field::setNumber(4, "SizeMin", m.options.minelementsize2);
    gmsh::model::mesh::field::setNumber(4, "SizeMax", m.options.maxelementsize);
    gmsh::model::mesh::field::setNumber(4, "DistMin", m.options.mindistance);
    gmsh::model::mesh::field::setNumber(4, "DistMax", m.options.maxdistance);

    gmsh::model::occ::synchronize();
} 
void TiglMesher::refineSurface( std::vector<double> Surface)
{
    // different refinement for Surface 7
    gmsh::model::mesh::field::add("Distance", 5);
    gmsh::model::mesh::field::setNumbers(5, "SurfacesList", {7});
    gmsh::model::mesh::field::setNumber(5, "Sampling", 50);

    gmsh::model::mesh::field::add("Threshold", 6);
    gmsh::model::mesh::field::setNumber(6, "InField", 5);
    gmsh::model::mesh::field::setNumber(6, "SizeMin", m.options.minelementsize3);
    gmsh::model::mesh::field::setNumber(6, "SizeMax", m.options.maxelementsize);
    gmsh::model::mesh::field::setNumber(6, "DistMin", m.options.mindistance);
    gmsh::model::mesh::field::setNumber(6, "DistMax", m.options.maxdistance);

    gmsh::model::occ::synchronize();
    
}
}
