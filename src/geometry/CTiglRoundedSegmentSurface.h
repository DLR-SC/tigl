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
    TIGL_EXPORT void Perform();
    // Converts TopoDS_Wire → Handle(Geom_BSplineCurve)
    TIGL_EXPORT void ConvertCurves();

    // Initialize values required for 'buildLoft'
    // Determin dimensions of pole matrix
    TIGL_EXPORT void initializePoleMatrix();

    // Calculate required poles to build rounded curves at given sections
    TIGL_EXPORT void calculatePoleMatrix();

    // Deduce required knot vector and multiplicies from pole matrix properties
    //TODO: Add mathematical boundaries for required number of knots and multiplicities
    TIGL_EXPORT void calculateKnotsAndMultiplicities();
    /*
     * @brief Creates a surface with rounded sections

        Steps:  Create Matrix that contains all poles of each profile curve and
                Dummy-Profile-Curves that contain poles that are required to build the curve in v-Direction
                TODO: Add mathematical description of the rounded segment curve building process
                - i-th row represents a profile in u-direction
                - j-th columnrepresents a curve in v-direction

                Steps:  Retrieve poles from inner and outer profile
                        Calculate distance between poles, apply inner/outer rounding distance relative to number of dummy profile
                        store new poles and original poles all in one matrix
    */

    // Call required methods to initialize required values and build surface
    TIGL_EXPORT Handle(Geom_BSplineSurface) buildLoft();
    TIGL_EXPORT void Invalidate() { _hasPerformed = false; }

private:
    // Rows represent profile curves (u-direction) and dummy curves, Columns define poles of curves in v-direction
    TColgp_HArray2OfPnt m_pole_matrix;

    TColStd_Array1OfReal m_u_knots;
    TColStd_Array1OfReal m_v_knots;

    TColStd_HArray1OfInteger m_u_multiplicities;
    TColStd_HArray1OfInteger m_v_multiplicities;

    //Input datatype for constructor, to be converted in perform()-method
    const std::vector<TopoDS_Wire>& m_profileWires;

    //Storage of inner and outer rounding distance per segment
    std::vector<double> m_inner_rounding_distance;
    std::vector<double> m_outer_rounding_distance;

    // Store as B-spline curves internally (after conversion)
    std::vector<Handle(Geom_BSplineCurve)> m_profileCurves;

    bool _hasPerformed = false;
    int _maxDegree = 3;
    int _u_degree = 3;
    int _v_degree = 3;
    size_t _nb_dummies =3; //Rows per rounding distance


};

}
#endif // CTIGLROUNDEDSEGMENTSURFACE_H
