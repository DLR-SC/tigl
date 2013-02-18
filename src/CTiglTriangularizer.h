#ifndef CTIGLTRIANGULARIZER_H
#define CTIGLTRIANGULARIZER_H

#include "CTiglPolyData.h"

class TopoDS_Shape;

namespace tigl {

class CTiglTriangularizer : public CTiglPolyData
{
public:
    CTiglTriangularizer();
    CTiglTriangularizer(TopoDS_Shape&, bool useMultipleObjects = false);
    CTiglTriangularizer(class CTiglAbstractPhysicalComponent&);
    
    void useMultipleObjects(bool);
    
private:
    int triangularizeShape(const TopoDS_Shape &);
    int computeVTKMetaData(class CCPACSWing&);
    
    bool _useMultipleObjects;
};

}

#endif // CTIGLTRIANGULARIZER_H
