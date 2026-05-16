#ifndef CTIGLROUNDEDSEGMENTSURFACE_H
#define CTIGLROUNDEDSEGMENTSURFACE_H

#include "Geom_BSplineCurve.hxx"
#include "TopoDS_Wire.hxx"
#include "tigl_internal.h"
#include <vector>

namespace tigl {

class CTiglRoundedSegmentSurface
{
public:
    TIGL_EXPORT CTiglRoundedSegmentSurface(const std::vector<TopoDS_Wire>&, double inner_rounding_distance, double outer_rounding_distance);

    TIGL_EXPORT TopoDS_Shape& Shape();

private:
    // Converts TopoDS_Wire → Handle(Geom_BSplineCurve)
    TIGL_EXPORT void Perform();
    TIGL_EXPORT void Invalidate() { _hasPerformed = false; }
    TIGL_EXPORT void ConvertCurves();
    TIGL_EXPORT void buildLoft();


    const std::vector<TopoDS_Wire>& profileWires;
    // Store as B-spline curves internally (after conversion)
    std::vector<Handle(Geom_BSplineCurve)> m_profileCurves;


};

}
#endif // CTIGLROUNDEDSEGMENTSURFACE_H
