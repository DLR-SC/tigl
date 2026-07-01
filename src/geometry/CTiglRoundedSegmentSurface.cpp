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
CTiglRoundedSegmentSurface::CTiglRoundedSegmentSurface(const std::vector<Handle(Geom_Curve)> &profileCurves ,
                                                       const std::vector<double> &inner_rounding_distance,
                                                       const std::vector<double> &outer_rounding_distance):
    m_inner_rounding_distance(inner_rounding_distance),
    m_outer_rounding_distance(outer_rounding_distance),
    m_segments({}),
    m_surface(nullptr)
    {

    m_profileCurves.reserve(profileCurves.size());
    for (Handle(Geom_Curve) curve : profileCurves) {
        m_profileCurves.push_back(GeomConvert::CurveToBSplineCurve(curve));
    }

    CTiglBSplineAlgorithms::matchDegree(m_profileCurves);

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

    //initialize knots in u-direction(copy from originial 1. profile curve fits all)
    m_u_knots = m_profileCurves[0]->Knots();
    if (m_u_multiplicities.Length()==2){
        _u_degree =m_profileCurves[0]->Degree();
    } else {
        m_u_multiplicities = m_profileCurves[0]->Multiplicities();
    }

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
