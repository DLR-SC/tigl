#include "CTiglRoundedSegmentSurface.h"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "CTiglError.h"
#include "Debugging.h"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopoDS_Edge.hxx"
#include "tiglcommonfunctions.h"


namespace tigl{
CTiglRoundedSegmentSurface::CTiglRoundedSegmentSurface(const  std::vector<Handle(Geom_BSplineCurve)> &m_profileCurves ,
                                                       double inner_rounding_distance,
                                                       double outer_rounding_distance):
    m_pole_matrix(1,m_profileCurves.size()+(m_profileCurves.size()-2)*6,1, (m_profileCurves[0]->NbPoles())),
    m_inner_rounding_distance(inner_rounding_distance),
    m_outer_rounding_distance(outer_rounding_distance),
    m_profileCurves(m_profileCurves),
    m_segments({}),
    m_surface(nullptr){}

TIGL_EXPORT  Handle(Geom_BSplineSurface) CTiglRoundedSegmentSurface::Surface(){
    /** TODO IS ALL OF THAT COVERED?
         * OCC Geom_BSplineSurface():
         * Creates a non-rational b-spline surface (weights default value is 1.).
         * The following conditions must be verified. 0 < UDegree <= MaxDegree.
         * UKnots.Length() == UMults.Length() >= 2 UKnots(i) < UKnots(i+1) (Knots are increasing) 1 <= UMults(i) <= UDegree
         * On a non uperiodic surface the first and last umultiplicities may be UDegree+1
         * (this is even recommended if you want the curve to start and finish on the first and last pole).
         * On a uperiodic surface the first and the last umultiplicities must be the same. on non-uperiodic surfaces Poles.
         * ColLength() == Sum(UMults(i)) - UDegree - 1 >= 2 on uperiodic surfaces Poles.ColLength() == Sum(UMults(i))
         * except the first or last The previous conditions for U holds also for V, with the RowLength of the poles.
         */
    Perform();
    auto surface = new Geom_BSplineSurface(m_pole_matrix, m_u_knots, m_v_knots, m_u_multiplicities, m_v_multiplicities, _u_degree, _v_degree, false, false);

    return surface;
}

void CTiglRoundedSegmentSurface::Perform(){
    if (_hasPerformed) {
        return;
    }
    calculateKnotsAndMultiplicities();
//    //initialize polematrix
//    Standard_Integer m = m_profileCurves.size()+(m_profileCurves.size()-2)*2*_nb_dummies;
    Standard_Integer n = m_profileCurves[0]->NbPoles();
//    m_pole_matrix = TColgp_HArray2OfPnt(1,m,1,n);
    //initialize m_segments vector
    for(Handle(Geom_BSplineCurve) curve : m_profileCurves){
        m_segments.push_back(RoundedSegment(m_profileCurves[0],
                                            m_profileCurves[m_profileCurves.size()-1],
                                            m_inner_rounding_distance, //TODO access the right values per RoundedSegment
                                            m_outer_rounding_distance));
    }
    //update each element in m_segments vector and create all dummy profiles
    //outer segments have only inner|outer dummy curves
    m_segments[0].insert_outer_rows(_nb_dummies);
    m_segments[m_segments.size()-1].insert_inner_rows(_nb_dummies);
    //inner segments have both, inner and outer dummy curves
    for(int i = 1; i< m_segments.size()-1; i++){
        m_segments[i].insert_inner_rows(_nb_dummies);
        m_segments[i].insert_outer_rows(_nb_dummies);
    }

    int row=1; //counter for inserted rows
    std::cerr << "col:" << m_pole_matrix.NbColumns() << " rows: " << m_pole_matrix.NbRows() << std::endl;
    for(RoundedSegment seg: m_segments){
        //fill pole matrix
        for( int col = 1;col< n+1; col++){
            m_pole_matrix.SetValue(row,col,seg.get_first_profile().Value(col));
        }
        row++;
        std::cerr << row << std::endl;
        for(TColgp_HArray1OfPnt profile: seg.getDummyProfiles()){
            for( int col = 1;col< n+1; col++){
                m_pole_matrix.SetValue(row,col,profile.Value(col));
            }
            row++;
        std::cerr << row << std::endl;
        }
 //       if(row==m_pole_matrix.RowLength()){
 //           for( int col = 1;col< n+1; col++){
 //               m_pole_matrix.SetValue(row,col,seg.get_last_profile().Value(col));
 //           }
 //           row++;
 //           std::cerr << row << std::endl;
 //       }
    }
    _hasPerformed = true;
}

void CTiglRoundedSegmentSurface::calculateKnotsAndMultiplicities(){
    //create knots in v-direction
    auto nb_v_knots = m_pole_matrix.ColLength()+_v_degree-5;
    TColStd_Array1OfReal v_knots(1, nb_v_knots);
    std::vector<double> tmp_v = LinspaceWithBreaks(0., 1., nb_v_knots,{});
    for(int i=1;i<v_knots.Length()+1; i++){
        v_knots.SetValue(i,tmp_v[i-1]);
    }
    //create multiplicities in v-direction
    TColStd_HArray1OfInteger v_multiplicities(1, nb_v_knots, 1);
    v_multiplicities.SetValue(1,4);
    v_multiplicities.SetValue((nb_v_knots),4);

    //MAYBE CHECK
    //if(!(u_knots.Length()==u_multiplicities.Length())){
    //    throw tigl::CTiglError("u_knots and mults number mismatch:("+std::to_string(u_knots.Length())+","+std::to_string(u_multiplicities.Length())+")");
    //}
    //if(!(v_knots.Length()==v_multiplicities.Length())){
    //    throw tigl::CTiglError("v_knots and mults number mismatch:("+std::to_string(v_knots.Length())+","+std::to_string(v_multiplicities.Length())+")");
    //}
    //for(int i = 1; i< u_knots.Length()-1;i++){
    //    if (!(u_knots.Value(i) < u_knots.Value(i+1))){
    //        throw tigl::CTiglError(""+std::to_string(u_knots.Value(i))+"i:"+std::to_string(i)+" check knots"+std::to_string(u_knots.Value(i+1)));
    //    }
    //}
    //for(int i = 1; i< v_knots.Length()-1;i++){
    //    if (!(v_knots.Value(i) < v_knots.Value(i+1))){
    //        throw tigl::CTiglError("check knots");
    //    }
    //}
    //if(!(m_profileCurves[0]->NbPoles()==(nb_u_knots+2*3-u_degree-1))){
    //    throw tigl::CTiglError("poles != knots- u_degree-1 Poles:"+std::to_string(m_profileCurves[0]->NbPoles())+"Knots:"+
    //                           std::to_string(m_profileCurves[0]->NbKnots())+"u_degree:"+std::to_string(u_degree));
    //}
    //if(!(pole_matrix.ColLength()==(nb_v_knots+2*3-v_degree-1))){
    //    throw tigl::CTiglError("poles != knots- v_degree-1 Poles:"+std::to_string(nb_v_knots)+"Knots:"
    //                           +std::to_string(pole_matrix.ColLength())+"v_degree:"+std::to_string(v_degree));
    //
}

}
