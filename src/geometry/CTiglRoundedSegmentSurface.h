#ifndef CTIGLROUNDEDSEGMENTSURFACE_H
#define CTIGLROUNDEDSEGMENTSURFACE_H

#include "Geom_BSplineCurve.hxx"
#include "Geom_BSplineSurface.hxx"
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

    // Rows represent profile curves (u-direction) and dummy curves, Columns define poles of curves in v-direction
    TColgp_HArray2OfPnt pole_matrix;

    TColStd_Array1OfReal u_knots;
    TColStd_Array1OfReal v_knots;

    TColStd_HArray1OfInteger u_multiplicities;
    TColStd_HArray1OfInteger v_multiplicities;

    //Input datatype for constructor, to be converted in perform()-method
    const std::vector<TopoDS_Wire>& m_profileWires;

    //Storage of inner and outer rounding distance per segment
    std::vector<double> inner_rounding_distance;
    std::vector<double> outer_rounding_distance;

    // Store as B-spline curves internally (after conversion)
    std::vector<Handle(Geom_BSplineCurve)> m_profileCurves;

    Geom_BSplineSurface m_surface;

    bool _hasPerformed = false;
    int _maxDegree = 3;


};

}
#endif // CTIGLROUNDEDSEGMENTSURFACE_H
