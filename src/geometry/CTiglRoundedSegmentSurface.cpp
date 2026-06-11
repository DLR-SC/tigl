#include "CTiglRoundedSegmentSurface.h"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "CTiglError.h"
#include "Debugging.h"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopoDS_Edge.hxx"
#include "tiglcommonfunctions.h"


namespace tigl{
CTiglRoundedSegmentSurface::CTiglRoundedSegmentSurface(const std::vector<Handle(Geom_BSplineCurve)> &m_profileCurves ,
                                                       std::vector<double> inner_rounding_distance,
                                                       std::vector<double> outer_rounding_distance, int u_degree, int v_degree):
    m_pole_matrix(1,m_profileCurves.size()+(m_profileCurves.size()-2)*6,1, (m_profileCurves[0]->NbPoles())),
    m_u_knots(1,m_profileCurves[0]->NbKnots()),
    m_v_knots(1, m_pole_matrix.ColLength()-v_degree+1),
    m_u_multiplicities(m_profileCurves[0]->Multiplicities()),
    m_v_multiplicities(1, m_pole_matrix.ColLength()-v_degree+1, 1),
    m_inner_rounding_distance(inner_rounding_distance),
    m_outer_rounding_distance(outer_rounding_distance),
    m_profileCurves(m_profileCurves),
    m_segments({}),
    m_surface(nullptr),
    _u_degree(u_degree),
    _v_degree(v_degree){}

TIGL_EXPORT  Handle(Geom_BSplineSurface) CTiglRoundedSegmentSurface::Surface(){

    Perform();


    //BEGIN DEBUG DELETEME
    int m = m_pole_matrix.ColLength();
    int n = m_pole_matrix.RowLength();

    for(int col=1; col<n+1; col++){
        NCollection_Array1< gp_Pnt > i_poles(1,m);
    //    std::cerr<< "Column:" << col << std::endl;
        for(int i=1; i < m+1; i++){
            i_poles.SetValue(i,(m_pole_matrix.Value(i,col)));
    //        std::cerr << "X:" << i_poles.Value(i).Coord().X() << "\t Y:" << i_poles.Value(i).Coord().Y() << "\t Z:" << i_poles.Value(i).Coord().Z() << std::endl;
        }
        auto spline = new Geom_BSplineCurve(i_poles, m_v_knots, m_v_multiplicities, _v_degree);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(spline);
        tigl::dumpShape(edge,"makeLoftWing", "Edge",col);
    }
    for(int row=1; row<m+1; row++){
        NCollection_Array1< gp_Pnt > i_poles(1,n);
  //      std::cerr<< "Row:" << row << std::endl;
        for(int i=1; i < n+1; i++){
            i_poles.SetValue(i,(m_pole_matrix.Value(row,i)));
 //           std::cerr << "X:" << i_poles.Value(i).Coord().X() << "\t Y:" << i_poles.Value(i).Coord().Y() << "\t Z:" << i_poles.Value(i).Coord().Z() << std::endl;
        }
        auto spline = new Geom_BSplineCurve(i_poles, m_u_knots, m_u_multiplicities, _u_degree);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(spline);
        tigl::dumpShape(edge,"makeLoftWing", "Row",row);
    }
    std::cerr << "Dimensions:\nPolematrix:\nrows: " << m_pole_matrix.ColLength()
              << "\ncol: " << m_pole_matrix.RowLength()
              << "\n v_knot number: " << m_v_knots.Length()
              << "\n u_knot number: " << m_u_knots.Length()
              << "\n v_mults number: " << m_v_multiplicities.Length()
              << "\n u_mults number: " << m_u_multiplicities.Length() <<std::endl;
    //END DEBUG DELETEME


    auto surface = new Geom_BSplineSurface(m_pole_matrix, m_v_knots, m_u_knots,  m_v_multiplicities, m_u_multiplicities,_v_degree, _u_degree, false, false);
    return surface;
}

void CTiglRoundedSegmentSurface::Perform(){
    if (_hasPerformed) {
        return;
    }


    std::cerr << "Dimensions:\nPolematrix:\nrows: " << m_pole_matrix.ColLength()
              << "\ncol: " << m_pole_matrix.RowLength()
              << "\n v_knot number: " << m_v_knots.Length()
              << "\n u_knot number: " << m_u_knots.Length()
              << "\n v_mults number: " << m_v_multiplicities.Length()
              << "\n u_mults number: " << m_u_multiplicities.Length() <<std::endl;

    std::cerr << "number of multiplicities" << m_u_multiplicities.Length() << std::endl;
    for(int i = 1; i<= m_u_multiplicities.Length(); i++){
            std::cerr << i << ":" << m_u_multiplicities.Value(i) << " ";
    }
    std::cerr << "" << std::endl;
    calculateKnotsAndMultiplicities();

    //initialize m_segments vector
    for(int i = 0; i < m_profileCurves.size()-1; i++){
        m_segments.push_back(RoundedSegment(m_profileCurves[i],
                                            m_profileCurves[i+1],
                                            m_inner_rounding_distance[i], //TODO access the right values per RoundedSegment
                                            m_outer_rounding_distance[i]));
    }

    //Write poles in pole matrix
    int row=1; //counter for inserted rows
    std::cerr << "col:" << m_pole_matrix.NbColumns() << " rows: " << m_pole_matrix.NbRows() << std::endl;
    for(RoundedSegment seg: m_segments){
        //fill pole matrix
        for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
            m_pole_matrix.SetValue(row,col,seg.get_first_profile().Value(col));
        }
        row++;
        std::cerr << row << std::endl; //DEBUG DELETEME
        for(TColgp_HArray1OfPnt profile: seg.getDummyProfiles()){
            for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
                m_pole_matrix.SetValue(row,col,profile.Value(col));
            }
            row++;
  //      std::cerr << row << "ROW: " << m_pole_matrix.ColLength() << std::endl; //DEBUG DELETEME
        }
        if(row==m_pole_matrix.ColLength()){
    //        std::cerr << row << "ROW: " << m_pole_matrix.ColLength() << std::endl; //DEBUG DELETEME
            for( int col = 1;col< m_profileCurves[0]->NbPoles()+1; col++){
                m_pole_matrix.SetValue(row,col,seg.get_last_profile().Value(col));
            }
        }
    }
    _hasPerformed = true;
}

void CTiglRoundedSegmentSurface::calculateKnotsAndMultiplicities(){
    //create knots in v-direction
    //auto nb_v_knots = m_pole_matrix.ColLength()+_v_degree-5;
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
    std::cerr << "CalculateKnots :number of multiplicities" << m_u_multiplicities.Length() << std::endl;
     if (m_u_multiplicities.Length()==2){
        std::cerr << "IF" << std::endl;
//        m_u_multiplicities.SetValue(1,4);
//        m_u_multiplicities.SetValue(2,4);
        _u_degree =m_profileCurves[0]->Degree();
    } else {
        m_u_multiplicities = m_profileCurves[0]->Multiplicities();
    }

    //MAYBE CHECK
    if(!(m_u_knots.Length()==m_u_multiplicities.Length())){
        throw tigl::CTiglError("u_knots and mults number mismatch:("+std::to_string(m_u_knots.Length())+","+std::to_string(m_u_multiplicities.Length())+")");
    }
    if(!(m_v_knots.Length()==m_v_multiplicities.Length())){
        throw tigl::CTiglError("v_knots and mults number mismatch:("+std::to_string(m_v_knots.Length())+","+std::to_string(m_v_multiplicities.Length())+")");
    }
    for(int i = 1; i< m_u_knots.Length()-1;i++){
        if (!(m_u_knots.Value(i) < m_u_knots.Value(i+1))){
            throw tigl::CTiglError(""+std::to_string(m_u_knots.Value(i))+"i:"+std::to_string(i)+" check knots"+std::to_string(m_u_knots.Value(i+1)));
        }
    }
    for(int i = 1; i< m_v_knots.Length()-1;i++){
        if (!(m_v_knots.Value(i) < m_v_knots.Value(i+1))){
            throw tigl::CTiglError("check knots");
        }
    }
//    if(!(m_profileCurves[0]->NbPoles()==(m_profileCurves[0]->NbKnots()+2*3-_u_degree-1))){
//        throw tigl::CTiglError("poles != knots- u_degree-1 Poles:"+std::to_string(m_profileCurves[0]->NbPoles())+"Knots:"+
//                               std::to_string(m_u_knots.Length())+"u_degree:"+std::to_string(_u_degree));
//    }
    if(!(m_pole_matrix.ColLength()==(nb_v_knots+2*3-_v_degree-1))){
        throw tigl::CTiglError("poles != knots- v_degree-1 Poles:"+std::to_string(nb_v_knots)+"Knots:"
                               +std::to_string(m_pole_matrix.ColLength())+"v_degree:"+std::to_string(_v_degree));
    }
}

}
