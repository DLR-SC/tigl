#include "Mesher.h"

namespace tigl {

class TiglMesher
{
public:

TiglMesher();
~TiglMesher();

void set_dimension( int dimension);
void set_minMeshSize1( double min);
void set_minMeshSize2( double min2);
void set_minMeshSizeSurface(double min3);
void set_maxMeshSize ( double max);
void set_minDistance (double DistMin);
void set_maxDistance ( double DistMax);
void refineEdge1( std::vector<double> Edge);
void refineEdge2( std::vector<double> Edge2);
void refineSurface( std::vector<double> Surface);
void mesh() const;
void import(TopoDS_Shape shape) const;

int dimension;
double min;
double min2;
double min3;
double max;
double DistMin;
double DistMax;
};
}