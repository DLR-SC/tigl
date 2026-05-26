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
    m_pole_matrix({}),
    m_inner_rounding_distance(inner_rounding_distance),
    m_outer_rounding_distance(outer_rounding_distance),
    m_profileCurves(m_profileCurves),
    m_segments({}),
    m_surface(nullptr){}

TIGL_EXPORT  Handle(Geom_BSplineSurface) Surface();

void CTiglRoundedSegmentSurface::Perform(){
    //initialize polematrix
    Standard_Integer m = m_profileCurves.size()+(m_profileCurves.size()-2)*2*_nb_dummies;
    Standard_Integer n = m_profileCurves[0]->NbPoles();
    m_pole_matrix = TColgp_HArray2OfPnt(1,m,1,n);
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

    int row=0; //counter for inserted rows
    for(RoundedSegment seg: m_segments){
        //fill pole matrix
        //Retrieve First Segments Curves
        for( int col = 1;col< n+1; col++){
            m_pole_matrix.SetValue(0,col,m_segments[0].get_first_profile().Value(col));
        }
        row++; //next row
        for(TColgp_HArray1OfPnt profile: m_segments[0].getDummyProfiles()){
            for( int col = 1;col< n+1; col++){
                m_pole_matrix.SetValue(row,col,profile.Value(col));
            }
            row++;
        }
        //Retrieve Last Segments Curves
        for( int col = 1;col< n+1; col++){
            m_pole_matrix.SetValue(0,col,m_segments[0].get_last_profile().Value(col));
        }
    }

}

//void CTiglRoundedSegmentSurface::calculatePoleMatrix()
//{
//    //1. loop: iterate through profiles(rows) (index starts with 0 since std::vector
//    //2. loop: iterate through profile curves poles(colums) (Geom_Curve) "profileCurves" for each edge
//    //i rows, j columns (OCC indizes start with 1, loops start with 1 when iterating through OCC datatypes)
//    //note: loop-indices are named i,j,k and refer to row,column,number of dummy-Profile for better geometric/code-orientation
//
//    //following variables refer to the values between i-th and (i+1)-th profile curve
//    //cases:
//    //2 sections: no dummy profiles to build
//    //3 sections: dummy profiles around middle section
//    //...
//    //no dummies after 1.st and before last section
//
//    double inner_rd, outer_rd;
//    gp_Pnt inner_poles_per_col, outer_poles_per_col;
//
//    int m = m_pole_matrix.NbRows();
//    int n = m_pole_matrix.NbColumns();
//
//    //iterate through given profile curves
//    for (int i=0; i< m_profileCurves.size(); i++){
//        //determine rounding distances between profiles
//        if(i==0){
//            inner_rd =0.; //no value available = no dummies after first section
//            outer_rd = m_outer_rounding_distance[i];
//        }
//        if(i>0&&!(i==m_profileCurves.size())){
//            inner_rd = m_inner_rounding_distance[i-1];
//            outer_rd = m_outer_rounding_distance[i];
//        } else{
//            inner_rd = m_inner_rounding_distance[i-1];
//            outer_rd = 0.; //no value available = no dummies before last section
//        }
//        //iterate through columns to create Dummy Profile-Poles between each pair of profiles
//        for( int j=0; j< n; j++){
//            //retrieve poles of i-th and (i+1)-th profile (thus out of bounds for outer pole in row before last row)
//            inner_poles_per_col = m_profileCurves[i]->Pole(j+1);//v0
//            if(i<m_profileCurves.size()-1){
//                outer_poles_per_col = m_profileCurves[i+1]->Pole(j+1); //v1
//            } else {
//                outer_poles_per_col = inner_poles_per_col;
//            }
//            //calculate Vector between both profile poles and normalize it
//            gp_Vec vector_in_v_direction(inner_poles_per_col, outer_poles_per_col); //v01
//            gp_Vec normalized_vector_in_v_direction(vector_in_v_direction);
//            if(inner_poles_per_col.IsEqual(outer_poles_per_col, 1e-14)){
//                throw tigl::CTiglError(std::to_string(inner_poles_per_col.Coord().X())+"x"+
//                                       std::to_string(inner_poles_per_col.Coord().Y())+"y"+
//                                       std::to_string(inner_poles_per_col.Coord().Y())+"Z"+
//                                       std::to_string(outer_poles_per_col.Coord().X())+"x"+
//                                       std::to_string(outer_poles_per_col.Coord().Y())+"y"+
//                                       std::to_string(outer_poles_per_col.Coord().Y())+"Z");
//            }
//            normalized_vector_in_v_direction.Normalize(); //|v01|
//            //create dummy_profiles, which will be ordered as follows in between the sections(profile[i]/[i+1]):
//            //(inner)-> profile[0] | ------ dummy_outer1 | dummy_outer2 | dummy_outer3 | profile[i] | dummy_inner1 | dummy_inner2 | dummy_inner3 | ----  profile[i+1] |<-(outer)
//            //add profile first
//            int current_row;
//            if(i==0){
//                current_row =1;
//            } else {
//                current_row = _nb_dummies+3+(i-1)*(1+_nb_dummies*2);
//            }
//            //insert first profile pole
//            if(current_row==1){
//                std::cerr << "Insert First Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
//                m_pole_matrix.SetValue(current_row,j+1, inner_poles_per_col); //v0 first
//                current_row+=1;
//            }
//            //calculate new poles for inner dummy profiles (from inner to outer) except 1st (i=0) section
//            if(i>0&&i<m_profileCurves.size()-1){
//                for (int k=0; k < _nb_dummies; k++){
//                    //calculate distance between k-th inner dummy-pole in v-direction and profile
//                    double inner_distance = inner_rd/_nb_dummies*(k+1);
//                    gp_Vec inner_poles_per_col_vec(inner_poles_per_col.Coord().X(),inner_poles_per_col.Coord().Y(),inner_poles_per_col.Coord().Z());
//                    //save new poles in a vector for each inner dummy profile
//                    gp_Vec inner_vec(normalized_vector_in_v_direction);
//                    inner_vec.Scale(inner_distance); //scaled normalized vector in v-direction
//                    inner_vec.Add(inner_poles_per_col_vec); //vector to new k-th pole
//                    gp_Pnt new_pole_inner(inner_vec.XYZ());
//                    std::cout << "InnerDistance:" << inner_distance << " k:" << k <<std::endl;
//                    std::cerr << "Insert inner " << k << ". Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
//                    m_pole_matrix.SetValue(current_row,j+1, new_pole_inner);
//                    current_row +=1;
//                }
//            }
//            //calculate new poles for outer dummy profiles (calculate also from inner to outer -> _nb_dummies -(k+1)/_nb_dummies)
//            //last segment has no outer dummies -> < m_profileCurves.size()-2
//            if(i<(m_profileCurves.size()-2)){
//                for (int k=0; k < _nb_dummies; k++){
//                    //calculate distance between k-th outer dummy-pole in v-direction and profile
//                    double outer_distance = outer_rd/_nb_dummies * (_nb_dummies-k);
//                    gp_Vec outer_vec(outer_poles_per_col.Coord().X(),outer_poles_per_col.Coord().Y(),outer_poles_per_col.Coord().Z());
//                    gp_Vec outer_normalized_vec(normalized_vector_in_v_direction);
//                    outer_normalized_vec.Scale(outer_distance); //scaled normalized vector in v-direction
//                    outer_vec.Subtract(outer_normalized_vec);
//                    gp_Pnt new_pole_outer(outer_vec.XYZ());
//                    std::cout << "OuterDistance:" << outer_distance << " k:" << k << std::endl;
//                    std::cerr << "Insert outer " << k << ". Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
//                    //save new poles in a vector for each outer dummy profile
//                    m_pole_matrix.SetValue(current_row,j+1, new_pole_outer);
//                    current_row+=1;
//                }
//            }
//            //insert actual profile pole
//            if(i<m_profileCurves.size()){
//                std::cerr << "Last Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
//                m_pole_matrix.SetValue(current_row, j+1, outer_poles_per_col);
//                current_row+=1;
//            }
//        }
//    }
//}








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

opencascade::handle<Geom_BSplineSurface> Surface(){

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

    //auto surface = new Geom_BSplineSurface(m_pole_matrix, m_u_knots, m_v_knots, m_u_multiplicities, m_v_multiplicities, _u_degree, _v_degree, false, false);

    //DEBUG DELETEME
   // Handle(Geom_Surface) handle_surface = surface;
   // TopoDS_Shape surf = BRepBuilderAPI_MakeFace(handle_surface, 1e-15);
   // tigl::dumpShape(surf, "makeLoftWing", "Surface",1);

   // return surface;
}


}
