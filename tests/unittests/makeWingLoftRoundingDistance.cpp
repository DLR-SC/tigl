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
        size_t nb_dummies =3;
        Standard_Integer u_degree = 3;
        Standard_Integer v_degree = 3;

        //iterate through profile curves poles (Geom_Curve) "profileCurves" for each edge

        //DELETEMEstd::vector<std::vector<gp_Pnt>> pole_matrix(profileCurves.size(), std::vector<gp_Pnt>(profileCurves[profileCurves.size()]->NbPoles()));
        //i rows, j columns (OCC indizes start with 1)
        //note: for-loop indices are named i,j,k and refer to row,column,number of dummy-Profile for better geometric/code-orientation
        Standard_Integer m = (profileCurves.size()+1)+profileCurves.size()*2*nb_dummies*(profileCurves.size()-1);
        Standard_Integer n = profileCurves[0]->NbPoles()+1;
        //check if NbPoles is same in all profileCurves
        for(int i=0; n<profileCurves.size()-1; i++){
            if(!(profileCurves[i]->NbPoles()==profileCurves[i+1]->NbPoles())){
                throw tigl::CTiglError("ProfileCurves: Numbers of poles don't match");
            }
        }
        TColgp_HArray2OfPnt pole_matrix(1,m,1,n);
        //iterate through profiles(rows)
        gp_Pnt inner_originalCurve_pnt, outer_originalCurve_pnt;
        for (int i=0; i< profileCurves.size(); i++){
            //iterate through columns to create Dummy Profiles to each pair of profiles
            for( int j=0; j< profileCurves[i]->NbPoles(); j++){
                //retrieve a pole to each profile
                if(i<profileCurves.size()-2){
                    inner_originalCurve_pnt = profileCurves[i]->Pole(j+1);
                    outer_originalCurve_pnt = profileCurves[i+1]->Pole(j+1);
                }
                //calculate Vector between both profile poles and normalize it
                //distance between both points is length of this vector
                gp_Vec vector_in_v_direction(inner_originalCurve_pnt, outer_originalCurve_pnt);
                gp_Vec normalized_vector_in_v_direction(vector_in_v_direction);
                if(inner_originalCurve_pnt.IsEqual(outer_originalCurve_pnt, 0e-35)){
                    throw tigl::CTiglError(std::to_string(inner_originalCurve_pnt.Coord().X())+"x"+
                                           std::to_string(inner_originalCurve_pnt.Coord().Y())+"y"+
                                           std::to_string(inner_originalCurve_pnt.Coord().Y())+"Z"+
                                           std::to_string(outer_originalCurve_pnt.Coord().X())+"x"+
                                           std::to_string(outer_originalCurve_pnt.Coord().Y())+"y"+
                                           std::to_string(outer_originalCurve_pnt.Coord().Y())+"Z");
                }
                normalized_vector_in_v_direction.Normalize();
                //create dummy_profiles, which will be ordered as follows in between the sections(profile[i]/[i+1]):
                //(inner)-> profile[i] | dummy_inner1 dummy_inner2 dummy_inner3 | dummy_outer1 dummy_outer2 dummy_outer3 profile[i+1] | <-(outer)
                //add profile first
                int current_row = i+i*2*nb_dummies*(profileCurves.size()-1)+1;
                pole_matrix.SetValue(current_row,j+1, inner_originalCurve_pnt);
                //calculate new poles for inner dummy profiles (from inner to outer)
                for (int k=0; k < nb_dummies; k++){
                    //calculate distance between k-th inner dummy-pole in v-direction and profile
                    double inner_distance = inner_rd* (k+1)/nb_dummies;
                    //save new poles in a vector for each inner dummy profile
                    gp_Vec inner_vec(normalized_vector_in_v_direction);
                    inner_vec.Scale(inner_distance);
                    gp_Pnt new_pole_inner(inner_vec.XYZ());
                    current_row +=1;
                    pole_matrix.SetValue(current_row,j+1, new_pole_inner);
                }
                //calculate new poles for outer dummy profiles (calculate also from inner to outer -> nb_dummies -(k+1)/nb_dummies)
                for (int k=0; k < nb_dummies; k++){
                    //calculate distance between k-th outer dummy-pole in v-direction and profile
                    double outer_distance = outer_rd* (nb_dummies-(k+1))/nb_dummies;
                    gp_Vec outer_vec(vector_in_v_direction);
                    gp_Vec outer_normalized_vec(normalized_vector_in_v_direction);
                    outer_normalized_vec.Scale(outer_distance);
                    outer_vec.Subtract(outer_normalized_vec);
                    gp_Pnt new_pole_outer(outer_vec.XYZ());
                    //save new poles in a vector for each outer dummy profile
                    current_row+=1;
                    pole_matrix.SetValue(current_row,j+1, new_pole_outer);
                }
                //add profile curve from outer section only if it is last section
                if (i==(profileCurves.size())){
                    current_row+= 1;
                    pole_matrix.SetValue(current_row, j+1, outer_originalCurve_pnt);
                }
            }
        }

        //TODO CHECK: is this information given anywhere later, so these lines can be deleted again?
        //create knots in u-direction
        size_t num_u_knots = profileCurves[0]->NbKnots();
        TColStd_Array1OfReal u_knots(1, num_u_knots);
        auto u_curve_length = (pole_matrix.Value(1,1).Distance(pole_matrix.Value(1,num_u_knots)));
        if (u_curve_length< 0e-15){
            throw tigl::CTiglError("Curve Length Zero");
        }
        for(int i=1; i < num_u_knots+1; i++){
            u_knots.SetValue(i,profileCurves[0]->Knot(i));
        }
        //define multiplicities in u-direction
        TColStd_HArray1OfInteger u_multiplicities(1,num_u_knots);
        for(int i=1; i < num_u_knots; i++){
            u_multiplicities[i] = 1;
        }
        //END TODO CHECK

        //create knots in v-direction
        //create knot-matrix in v-direction: 
        //Assume that knots are locateted where poles are (which is by implementation: intersections between Profiles/Dummy-Profiles and Curves in v-direction)
        //DELETEMEstd::vector<std::vector<double>> v_knots(pole_matrix.size(), std::vector<double>(pole_matrix[0].size()));
        TColStd_Array1OfReal v_knots(1, pole_matrix.ColLength());
        //iterate through pole_matrix columns and calculate distances between pole-vectors
        //since the knots are represented by a value relative to total value of curve_length = 1
        //divide these distances by total length of the curve in v-direction to create a parameter for the knot vector
        size_t num_v_knots = pole_matrix.ColLength();

        auto v_curve_length = (pole_matrix.Value(1,1).Distance(pole_matrix.Value(1,num_v_knots)));
        for(int j=1; j < num_v_knots+1; j++){
            v_knots.SetValue(j,(pole_matrix.Value(1,1).Distance(pole_matrix.Value(1,j))/v_curve_length));
        }


        TColStd_Array1OfInteger v_multiplicities(1,num_v_knots);
        //define multiplicities for all the curves in v-direction
        v_multiplicities[1] = v_degree +1;
        for(int j=1; j < num_v_knots-1; j++){
            v_multiplicities[j] = 1; //assume 1 for a start, to be extended for modification-options
        }
        v_multiplicities[num_v_knots] = v_degree+1;
         
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


