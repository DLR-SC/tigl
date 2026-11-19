#include "CTiglRoundedSegmentSurface.h"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "CTiglError.h"
#include "Debugging.h"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopoDS_Edge.hxx"
#include "tiglcommonfunctions.h"
#include <GeomConvert.hxx>
#include "CTiglBSplineAlgorithms.h"


namespace tigl{

namespace {

// below this value, a rounding distance is treated as zero (no dummy profiles are inserted)
const double roundingDistanceTolerance = 1e-6;

std::string SegmentInfo(size_t iSeg)
{
    return "segment " + std::to_string(iSeg + 1);
}

} // namespace

void CTiglRoundedSegmentSurface::ValidateRoundingDistances(size_t nProfiles,
                                                           const std::vector<double>& inner,
                                                           const std::vector<double>& outer)
{
    if (nProfiles < 3) {
        throw CTiglError("CTiglRoundedSegmentSurface: At least 3 profile curves (2 segments) are required, got "
                         + std::to_string(nProfiles) + ".");
    }
    const size_t nSegments = nProfiles - 1;
    if (inner.size() != nSegments || outer.size() != nSegments) {
        throw CTiglError("CTiglRoundedSegmentSurface: Expected " + std::to_string(nSegments)
                         + " inner and outer rounding distances (one per segment), got "
                         + std::to_string(inner.size()) + " inner and " + std::to_string(outer.size()) + " outer.");
    }

    for (size_t iSeg = 0; iSeg < nSegments; ++iSeg) {
        for (double d : {inner[iSeg], outer[iSeg]}) {
            if (d < 0. || d > 1.) {
                throw CTiglError("CTiglRoundedSegmentSurface: Rounding distances of " + SegmentInfo(iSeg)
                                 + " must be within [0, 1] (relative to the segment), got " + std::to_string(d) + ".");
            }
        }
        // if both distances sum up to more than one, the dummy profiles of the segment overlap
        // and the surface folds back onto itself
        if (inner[iSeg] + outer[iSeg] > 1. + roundingDistanceTolerance) {
            throw CTiglError("CTiglRoundedSegmentSurface: The inner and outer rounding distance of " + SegmentInfo(iSeg)
                             + " sum up to " + std::to_string(inner[iSeg] + outer[iSeg])
                             + ", which is more than 1. This would result in a self-intersecting surface.");
        }
    }

    // the ends of the surface are not rounded
    if (inner.front() > roundingDistanceTolerance || outer.back() > roundingDistanceTolerance) {
        throw CTiglError("CTiglRoundedSegmentSurface: The inner rounding distance of the first segment and the outer "
                         "rounding distance of the last segment must be zero.");
    }

    // all inner sections must be rounded on both sides
    for (size_t iSeg = 0; iSeg + 1 < nSegments; ++iSeg) {
        if (outer[iSeg] <= roundingDistanceTolerance || inner[iSeg + 1] <= roundingDistanceTolerance) {
            throw CTiglError("CTiglRoundedSegmentSurface: The outer rounding distance of " + SegmentInfo(iSeg)
                             + " and the inner rounding distance of " + SegmentInfo(iSeg + 1)
                             + " must be greater than zero.");
        }
    }
}

CTiglRoundedSegmentSurface::CTiglRoundedSegmentSurface(const std::vector<Handle(Geom_Curve)> &profileCurves ,
                                                       const std::vector<double> &inner_rounding_distance,
                                                       const std::vector<double> &outer_rounding_distance):
    m_inner_rounding_distance(inner_rounding_distance),
    m_outer_rounding_distance(outer_rounding_distance),
    m_segments({}),
    m_surface(nullptr)
    {

    ValidateRoundingDistances(profileCurves.size(), inner_rounding_distance, outer_rounding_distance);

    m_profileCurves.reserve(profileCurves.size());
    for (Handle(Geom_Curve) curve : profileCurves) {
        m_profileCurves.push_back(GeomConvert::CurveToBSplineCurve(curve));
    }

    // make the profile curves compatible, i.e. same degree, parameter range and knot vector
    // (required, as the profile poles are used directly as rows of the surface pole matrix)
    CTiglBSplineAlgorithms::matchDegree(m_profileCurves);
    m_profileCurves = CTiglBSplineAlgorithms::createCommonKnotsVectorCurve(m_profileCurves, 1e-14);

    m_pole_matrix = TColgp_Array2OfPnt(1,m_profileCurves.size()+(m_profileCurves.size()-2)*6,1, (m_profileCurves[0]->NbPoles()));
    m_u_knots = TColStd_Array1OfReal(1,m_profileCurves[0]->NbKnots());
    m_v_knots = TColStd_Array1OfReal(1, m_pole_matrix.ColLength()-_v_degree+1);
    m_u_multiplicities = m_profileCurves[0]->Multiplicities();
    m_v_multiplicities= TColStd_HArray1OfInteger(1, m_pole_matrix.ColLength()-_v_degree+1, 1);
}

TIGL_EXPORT  Handle(Geom_BSplineSurface) CTiglRoundedSegmentSurface::Surface(){

    Perform();

    auto surface = new Geom_BSplineSurface(m_pole_matrix, m_v_knots, m_u_knots,  m_v_multiplicities, m_u_multiplicities,_v_degree, _u_degree, false, false);
    return surface;
}

void CTiglRoundedSegmentSurface::Perform(){
    if (_hasPerformed) {
        return;
    }

    //create knots in v-direction
    auto nb_v_knots = m_pole_matrix.ColLength()-_v_degree+1;
    std::vector<double> tmp_v = LinspaceWithBreaks(0., 1., nb_v_knots,{});
    for(int i=1;i<m_v_knots.Length()+1; i++){
        m_v_knots.SetValue(i,tmp_v[i-1]);
    }

    //create multiplicities in v-direction
    m_v_multiplicities.SetValue(1,4);
    m_v_multiplicities.SetValue((nb_v_knots),4);

    //initialize knots in u-direction (all profile curves share a common knot vector)
    m_u_knots = m_profileCurves[0]->Knots();
    m_u_multiplicities = m_profileCurves[0]->Multiplicities();
    _u_degree = m_profileCurves[0]->Degree();

    //initialize m_segments vector
    for(int i = 0; i < m_profileCurves.size()-1; i++){
        m_segments.push_back(RoundedSegment(m_profileCurves[i],
                                            m_profileCurves[i+1],
                                            m_inner_rounding_distance[i],
                                            m_outer_rounding_distance[i]));
    }

    //Write poles in pole matrix
    int row=1; //counter for inserted rows
    for(RoundedSegment seg: m_segments){
        //fill pole matrix
        for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
            m_pole_matrix.SetValue(row,col,seg.get_first_profile().Value(col));
        }
        row++;
        for(TColgp_HArray1OfPnt profile: seg.getDummyProfiles()){
            for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
                m_pole_matrix.SetValue(row,col,profile.Value(col));
            }
            row++;
        }
        if(row==m_pole_matrix.ColLength()){

            for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
                m_pole_matrix.SetValue(row,col,seg.get_last_profile().Value(col));
            }
        }
    }
    //Check if number of rows matches expected total number of profile curves
    if (row<m_profileCurves.size()+(m_profileCurves.size()-2)*6){
        throw tigl::CTiglError("CTiglRoundedSegmentSurface: Check Rounding Distances, only inner and outer segment can have  rounding distance = 0 ");
    }
    _hasPerformed = true;
}

}
