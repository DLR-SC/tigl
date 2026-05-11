#include "CCPACSWingSection.h"
#include "Debugging.h"
#include "test.h"
#include "tigl.h"
#include "tiglcommonfunctions.h"
#include "tixi.h"

#include "CCPACSConfigurationManager.h"

#include "CTiglMakeLoft.h"
#include "CTiglError.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopoDS.hxx>
#include <GeomConvert.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include "CCPACSCurvePointListXYZ.h"
#include "CCPACSCurveParamPointMap.h"

TEST(WingLoftRoundingDistance, makeShape)
{

    //BEGIN_LoadInformation This code is only for testing purposes
    // loaded information will be replaced by tigl-information that is accessible during runtime
    TiglHandleWrapper tiglHandle("TestData/simpletest.cpacs.xml", "");

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    //get wing from file
    tigl::CCPACSWing& wing = config.GetWing("Wing");
    int numberOfProfiles = wing.GetSectionCount();

    //access profile curves via wingSectionElement and fill temporary storage for profiles as TopoDS_Shape 
    std::vector<TopoDS_Wire> profiles;

    for(int i=1; i<numberOfProfiles+1; i++){
        auto &section = wing.GetSection(i).GetSectionElement(1);
        profiles.push_back(section.GetCTiglSectionElement()->GetWire());
    }
    //END_LoadInformation

    //BEGIN_CopyCode from CTiglMakeLoft
    // check number of edges are all same
    std::vector<unsigned int> edgeCountPerProfile;
    for (const auto& profile : profiles) {
        edgeCountPerProfile.push_back(GetNumberOfEdges(profile));
    }
    if (!AllSame(edgeCountPerProfile.begin(), edgeCountPerProfile.end())) {
        throw tigl::CTiglError("Number of edges not equal in CTiglMakeLoft");
    }

    // get number of edges per profile wire
    // --> should be the same for all profiles
    TopTools_IndexedMapOfShape firstProfileMap;
    TopExp::MapShapes(profiles[0], TopAbs_EDGE, firstProfileMap);
    int const nEdgesPerProfile = firstProfileMap.Extent();

    // skin the surface edge by edge
    // CAUTION: Here it is assumed that the edges are ordered
    // in the same way along each profile (e.g. lower edge,
    // upper edge, trailing edge for a wing)
    for ( int iE = 1; iE <= nEdgesPerProfile; ++iE ) {

        std::vector<Handle(Geom_BSplineCurve)> profileCurves;
        // get the curves per edge
        profileCurves.reserve(profiles.size());
        for (unsigned iP=0; iP<profiles.size(); ++iP ) {

            TopTools_IndexedMapOfShape profileMap;
            TopExp::MapShapes(profiles[iP], TopAbs_EDGE, profileMap);
            assert( profileMap.Extent() >= iE );

            TopoDS_Edge edge = TopoDS::Edge(profileMap(iE));
            tigl::dumpShape(edge,"makeLoftWing","edge",iP); //DELETEME
            profileCurves.push_back(GetBSplineCurve(edge));
        }

    //END Copy_code
        
        //skin the curves ('profileCurves'): new code for curvesToSurface 
        // //(input are curves, make Geom_BSplineSurface -> build the TopoDS_Shape from this (-> output is TopoDS_Shape))
        /*
        New code for skinning a wing surface:
        Goal: Create a surface, implement rounded sections

        Steps:  Create k Dummy-Profile(s-Curves maybe just the poles?) within innerRounding distance
                Create Matrix that contains all poles of each profile curve and all Dummy-Profile-Curves
                - i-th row represents a profile in u-direction
                - j-th columnrepresents a curve in v-direction
                
                Steps:  Retrieve poles from inner and outer profile
                        Calculate distance between poles, apply inner/outer rounding distance relative to number of dummy profile
                        store new poles and original poles all in one matrix
        */
        
        ////determine distance between both vectors
        //Standard_Real distance_inner_outer = originalCurve_inner.Distance(originalCurve_outer);
        //determine new location for pole relative to inner profile and inner rounding distance 
        //datatype for poles and 'support vectors' is  gp_Pnt storage is 'TColgp_Array1OfPnt &', needs to be const?
        //datatype for knots is 'double/Standard_Real', stored in  'TColStd_Array1OfReal &', needs to be const?
        //dummy values, will be input params later
        double inner_rd =0.2;
        double outer_rd =0.3;
        size_t nb_dummies =3; //Rows per rounding distance
        Standard_Integer u_degree = 3;
        Standard_Integer v_degree = 3;

        //iterate through profile curves poles (Geom_Curve) "profileCurves" for each edge

        //i rows, j columns (OCC indizes start with 1, loops start with one when iterating through OCC datatypes)
        //note: loop-indices are named i,j,k and refer to row,column,number of dummy-Profile for better geometric/code-orientation
        Standard_Integer m = profileCurves.size()+(profileCurves.size()-2)*2*nb_dummies;
        Standard_Integer n = profileCurves[0]->NbPoles();
        //check if NbPoles is same in all profileCurves
        for(int i=0; i<profileCurves.size()-1; i++){
            if(!(profileCurves[i]->NbPoles()==profileCurves[i+1]->NbPoles())){
                throw tigl::CTiglError("ProfileCurves: Numbers of poles don't match");
            }
        }
        TColgp_HArray2OfPnt pole_matrix(1,m,1,n);
        //iterate through profiles(rows)
        gp_Pnt inner_originalCurve_pnt, outer_originalCurve_pnt;
        for (int i=0; i< profileCurves.size(); i++){
            //iterate through columns to create Dummy Profiles to each pair of profiles
            for( int j=0; j< n; j++){
                //retrieve a pole to each profile
                if(i<profileCurves.size()-1){
                    inner_originalCurve_pnt = profileCurves[i]->Pole(j+1);//v0
                    outer_originalCurve_pnt = profileCurves[i+1]->Pole(j+1); //v1
                }
                //calculate Vector between both profile poles and normalize it
                //distance between both points is length of this vector
                gp_Vec vector_in_v_direction(inner_originalCurve_pnt, outer_originalCurve_pnt); //v01
                gp_Vec normalized_vector_in_v_direction(vector_in_v_direction);
                if(inner_originalCurve_pnt.IsEqual(outer_originalCurve_pnt, 1e-14)){
                    throw tigl::CTiglError(std::to_string(inner_originalCurve_pnt.Coord().X())+"x"+
                                           std::to_string(inner_originalCurve_pnt.Coord().Y())+"y"+
                                           std::to_string(inner_originalCurve_pnt.Coord().Y())+"Z"+
                                           std::to_string(outer_originalCurve_pnt.Coord().X())+"x"+
                                           std::to_string(outer_originalCurve_pnt.Coord().Y())+"y"+
                                           std::to_string(outer_originalCurve_pnt.Coord().Y())+"Z");
                }
                normalized_vector_in_v_direction.Normalize(); //|v01|
                //create dummy_profiles, which will be ordered as follows in between the sections(profile[i]/[i+1]):
                //(inner)-> profile[0] | ------ dummy_outer1 | dummy_outer2 | dummy_outer3 | profile[i] | dummy_inner1 | dummy_inner2 | dummy_inner3 | ----  profile[i+1] |<-(outer)
                //add profile first
                int current_row;
                if(i==0){
                    current_row =1;
                } else {
                    current_row = nb_dummies+3+(i-1)*(1+nb_dummies*2);
                }
                //insert first profile pole
                if(current_row==1){
                    std::cerr << "Insert First Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
                    pole_matrix.SetValue(current_row,j+1, inner_originalCurve_pnt); //v0 first
                    current_row+=1;
                }
                //calculate new poles for inner dummy profiles (from inner to outer) except 1st (i=0) section
                if(i>0&&i<profiles.size()-1){
                    for (int k=0; k < nb_dummies; k++){
                        //calculate distance between k-th inner dummy-pole in v-direction and profile
                        double inner_distance = inner_rd/nb_dummies*(k+1);
                        gp_Vec inner_originalCurve_vec(inner_originalCurve_pnt.Coord().X(),inner_originalCurve_pnt.Coord().Y(),inner_originalCurve_pnt.Coord().Z());
                        //save new poles in a vector for each inner dummy profile
                        gp_Vec inner_vec(normalized_vector_in_v_direction);
                        inner_vec.Scale(inner_distance); //scaled normalized vector in v-direction
                        inner_vec.Add(inner_originalCurve_vec); //vector to new k-th pole
                        gp_Pnt new_pole_inner(inner_vec.XYZ());
                        std::cerr << "Insert inner " << k << ". Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
                        pole_matrix.SetValue(current_row,j+1, new_pole_inner);
                        current_row +=1;
                    }
                }
                //calculate new poles for outer dummy profiles (calculate also from inner to outer -> nb_dummies -(k+1)/nb_dummies)
                //except last section
                if(i<(profileCurves.size()-2)){
                    for (int k=0; k < nb_dummies; k++){
                        //calculate distance between k-th outer dummy-pole in v-direction and profile
                        double outer_distance = outer_rd/nb_dummies * (nb_dummies-(k+1));
                        gp_Vec outer_vec(outer_originalCurve_pnt.Coord().X(),outer_originalCurve_pnt.Coord().Y(),outer_originalCurve_pnt.Coord().Z());
                        gp_Vec outer_normalized_vec(normalized_vector_in_v_direction);
                        outer_normalized_vec.Scale(outer_distance); //scaled normalized vector in v-direction
                        outer_vec.Subtract(outer_normalized_vec);
                        gp_Pnt new_pole_outer(outer_vec.XYZ());
                        std::cerr << "Insert outer " << k << ". Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
                        //save new poles in a vector for each outer dummy profile
                        pole_matrix.SetValue(current_row,j+1, new_pole_outer);
                        current_row+=1;
                    }
                }
                //insert actual profile pole
                if(i<profileCurves.size()-1){
                    std::cerr << "Last Profile pole: i:" << i << " j:" << j << " current_row:" << current_row << std::endl;
                    pole_matrix.SetValue(current_row, j+1, outer_originalCurve_pnt);
                    current_row+=1;
                }
            }
        }

        std::cerr << "Rows: " << pole_matrix.ColLength() << "m: " << m << "Columns: " << pole_matrix.RowLength() << "n: " << n << std::endl;

        //TODO CHECK: is this information given anywhere later, so these lines can be deleted again?
        //create knots in u-direction
        auto nb_u_knots= profileCurves[0]->NbPoles()+u_degree-5;
        TColStd_Array1OfReal u_knots(1,nb_u_knots);
        std::vector<double> tmp = LinspaceWithBreaks(0., 1., nb_u_knots,{});
        for(int j=1;j<u_knots.Length()+1; j++){
            u_knots.SetValue(j,tmp[j-1]);
        }

        //define multiplicities in u-direction
        TColStd_HArray1OfInteger u_multiplicities(1,nb_u_knots,1);
        u_multiplicities.SetValue(1,4);
        u_multiplicities.SetValue((nb_u_knots),4);
        //END TODO CHECK

        //create knots in v-direction
        //create knot-matrix in v-direction: 
        //Assume that knots are locateted where poles are (which is by implementation: intersections between Profiles/Dummy-Profiles and Curves in v-direction)
        //DELETEMEstd::vector<std::vector<double>> v_knots(pole_matrix.size(), std::vector<double>(pole_matrix[0].size()));
        auto nb_v_knots = pole_matrix.ColLength()+v_degree-5;
        TColStd_Array1OfReal v_knots(1, nb_v_knots);
        //iterate through pole_matrix columns and calculate distances between pole-vectors
        //since the knots are represented by a value relative to total value of curve_length = 1
        //divide these distances by total length of the curve in v-direction to create a parameter for the knot vector
        std::vector<double> tmp_v = LinspaceWithBreaks(0., 1., nb_v_knots,{});
        for(int i=1;i<v_knots.Length()+1; i++){
            v_knots.SetValue(i,tmp_v[i-1]);
        }

        TColStd_HArray1OfInteger v_multiplicities(1, nb_v_knots, 1);
        v_multiplicities.SetValue(1,4);
        v_multiplicities.SetValue((nb_v_knots),4);
         
        //create Geom B-Spline Surface from poles, knots and multiplicities, degree

        //Poles : TColpg_array2OfPnt & (pole_matrix)
        //UKnots : TColStd_array1OfReal & (u_params?)
        //VKnots : TColStd_array1OfReal & (v_kntos = v_params?)
        //UMults : TColStd_array1OfInteger &
        //VMults : TColStd_array1OfInteger (multiplicities)
        //Udegree :
        //Vdegree : v_degree
        /**
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
        if(!(u_knots.Length()==u_multiplicities.Length())){
            throw tigl::CTiglError("u_knots and mults number mismatch:("+std::to_string(u_knots.Length())+","+std::to_string(u_multiplicities.Length())+")");
        }
        if(!(v_knots.Length()==v_multiplicities.Length())){
            throw tigl::CTiglError("v_knots and mults number mismatch:("+std::to_string(v_knots.Length())+","+std::to_string(v_multiplicities.Length())+")");
        }
        for(int i = 1; i< u_knots.Length()-1;i++){
            if (!(u_knots.Value(i) < u_knots.Value(i+1))){
                throw tigl::CTiglError(""+std::to_string(u_knots.Value(i))+"i:"+std::to_string(i)+" check knots"+std::to_string(u_knots.Value(i+1)));
            }
        }
        for(int i = 1; i< v_knots.Length()-1;i++){
            if (!(v_knots.Value(i) < v_knots.Value(i+1))){
                throw tigl::CTiglError("check knots");
            }
        }
        if(!(profileCurves[0]->NbPoles()==(nb_u_knots+2*3-u_degree-1))){
            throw tigl::CTiglError("poles != knots- u_degree-1 Poles:"+std::to_string(profileCurves[0]->NbPoles())+"Knots:"+
                                        std::to_string(profileCurves[0]->NbKnots())+"u_degree:"+std::to_string(u_degree));
        }
        if(!(pole_matrix.ColLength()==(nb_v_knots+2*3-v_degree-1))){
                throw tigl::CTiglError("poles != knots- v_degree-1 Poles:"+std::to_string(nb_v_knots)+"Knots:"
                                        +std::to_string(pole_matrix.ColLength())+"v_degree:"+std::to_string(v_degree));
        }

        for(int col=1; col<n+1; col++){
            NCollection_Array1< gp_Pnt > i_poles(1,m);
            for(int i=1; i < m+1; i++){
                i_poles.SetValue(i,(pole_matrix.Value(i,col)));
                std::cerr << "X:" << i_poles.Value(i).Coord().X() << " Y:" << i_poles.Value(i).Coord().Y() << " Z:" << i_poles.Value(i).Coord().Z() << std::endl;
            }
            auto spline = new Geom_BSplineCurve(i_poles, v_knots, v_multiplicities, v_degree);
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(spline);
            tigl::dumpShape(edge,"makeLoftWing", "Edge",col);
        }

        for(int i=0; i<v_knots.Size(); i++){
            std::cerr <<"knot[" <<(i+1) <<"]:" << v_knots[i+1] << std::endl;
        }
        for(int i=0; i<v_multiplicities.Size(); i++){
            std::cerr <<"mult[" <<(i+1) <<"]:" << v_multiplicities[i+1] << std::endl;
        }
        auto surface = new Geom_BSplineSurface(pole_matrix, u_knots, v_knots, u_multiplicities, v_multiplicities, u_degree, v_degree);
        Handle(Geom_Surface) handle_surface = surface;
        TopoDS_Shape surf = BRepBuilderAPI_MakeFace(handle_surface, 1e-15);
        tigl::dumpShape(surf, "", "Surface",1);
    }
        
        // // knots for each dummy profile: uparams ? 
        // CTiglMakeLoft: private: std::vector<Standard_Real> uparams, vparams;
        // how define vparams ? 
        // // find wing length, find section length, place knot at inner_distance and outer_distance
        // // create one curve from section[0] to section[n] and length 'wing_length'=1, n being the last section
        // // calculate distances ->'param_i_k' (i-th profile, k-th dummyProfile) for knots relative to 'wing_length'
        // // each overlapping with dummy-pole position
        // // equidistant, 3 for 3 equidistant dummy-profiles, mulitiplicity at profile?

        // number of poles = number of knots - degree - 1
        // first and last knot are repeated degree + 1 times
    }


