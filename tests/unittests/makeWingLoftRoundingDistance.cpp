#include "test.h"
#include "tigl.h"
#include "tixi.h"

#include "CCPACSConfigurationManager.h"

#include "CTiglMakeLoft.h"
#include "CTiglError.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWing.h"

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopoDS.hxx>
#include <GeomConvert.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include "CCPACSCurvePointListXYZ.h"
#include "CCPACSCurveParamPointMap.h"

TEST(WingLoftRoundingDistance, makeShape)
{
    TiglHandleWrapper tiglHandle("TestData/simpletest.cpacs.xml", "");

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    //get wing from file
    tigl::CCPACSWing& wing = config.GetWing("Wing");
    int numberOfProfiles = wing.getSectionCount();

    //access profile curves via wingSectionElement and fill temporary storage for profiles as TopoDS_Shape 
    std::vector<TopoDS_Wire> profiles;

    for(int i=0; i<numberOfProfiles; i++){
       profiles.push_back(wing.GetSectionElement(i).GetCTiglSectionElement().GetWire());
    }

    // check number of edges are all same
    std::vector<unsigned int> edgeCountPerProfile;
    for (const auto& profile : profiles) {
        edgeCountPerProfile.push_back(GetNumberOfEdges(profile));
    }
    if (!AllSame(edgeCountPerProfile.begin(), edgeCountPerProfile.end())) {
        throw tigl::CTiglError("Number of edges not equal in CTiglMakeLoft");
    }

    TopoDS_Shell faces;
    BRep_Builder builder;
    builder.MakeShell(faces);

    // get number of edges per profile wire
    // --> should be the same for all profiles
    TopTools_IndexedMapOfShape firstProfileMap;
    TopExp::MapShapes(profiles[0], TopAbs_EDGE, firstProfileMap);
    int const nEdgesPerProfile = firstProfileMap.Extent();

    // skin the surface edge by adge
    // CAUTION: Here it is assumed that the edges are ordered
    // in the same way along each profile (e.g. lower edge,
    // upper edge, trailing edge for a wing)
    for ( int iE = 1; iE <= nEdgesPerProfile; ++iE ) {

        // get the curves per edge
        std::vector<Handle(Geom_Curve)> profileCurves;
        profileCurves.reserve(profiles.size());
        for (unsigned iP=0; iP<profiles.size(); ++iP ) {

            TopTools_IndexedMapOfShape profileMap;
            TopExp::MapShapes(profiles[iP], TopAbs_EDGE, profileMap);
            assert( profileMap.Extent() >= iE );

            TopoDS_Edge edge = TopoDS::Edge(profileMap(iE));
            profileCurves.push_back(GetBSplineCurve(edge));
        }
        
        //skin the curves ('profileCurves'): new code for curvesToSurface 
        // //(input are curves, make 1. Geom_BSplineSurface -> build the Shape from this somehow ->output is TopoDS_Shape)
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
        double innerRd =0.2; 
        double outerRd =0.3;
        //iterate through profile curves poles (Geom_Curve) "profileCurves" for each edge
        std::vector<std::vector<gp_Pnt>> pole_matrix(i,std::vector<gp_pnt>(j,nullptr)); //i rows, j columns
        for (int i=0; i< profileCurves.size(); i++){
            for( int j=0; j< profileCurves[i].size(); j++){
                //retrieve a pole
                gp_Pnt inner_originalCurve_pnt = profileCurves[i].Pole(j);
                gp_Pnt outer_originalCurve_pnt = profileCurves[i+1].Pole(j);
                //calculate Vector between both profile poles and normalize it
                //distance between both points is length of this vector
                gp_Vec(inner_originalCurve pnt, outer_originalCurve_pnt) vector_in_v_direction;
                gp_Vec normalized_vector_in_v_direction(normalized_vector_in_v_direction); 
                normalized_vector_in_v_direction.Normalize();
                //create dummy_profiles, which will be ordered as follows in between the sections(profile[i]/[i+1]): 
                //(inner)-> profile[i] | dummy_inner1 dummy_inner2 dummy_inner3 | dummy_outer1 dummy_outer2 dummy_outer3 profile[i+1] | <-(outer)
                //add first
                int current_row = i+2*nb_dummies*(profileCurves.size()-1);
                pole_matrix[current_row][j]= inner_originalCurve_pnt;
                //calculate new poles for inner dummy profiles (from inner to outer)
                for (int k=0; k < nb_dummies; k++){
                    //calculate distance between k-th inner dummy-pole in v-direction and profile
                    double inner_distance = inner_rd* (k+1)/nb_dummies;
                    gp_Vec inner_vec = normalized_vector_in_v_direction.Scale(inner_distance);
                    gp_Pnt new_pole_inner(inner_vec.XYZ());
                    //save new poles in a vector for each inner dummy profile
                    current_row +=k;
                    pole_matrix[current_row][j] = new_pole_inner;
                }
                //calculate new poles for outer dummy profiles (calculate also from inner to outer -> nb_dummies -(k+1)/nb_dummies)
                for (int k=0; k < nb_dummies; k++){
                    //calculate distance between k-th outer dummy-pole in v-direction and profile
                    double outer_distance = outer_rd* (nb_dummies-(k+1))/nb_dummies;
                    gp_Vec outer_vec = vector_in_v_direction.Substract(normalized_vector_in_v_direction.Scale(outer_distance));
                    gp_Pnt new_pole_outer(outer_vec.XYZ());
                    //save new poles in a vector for each outer dummy profile
                    current_row+= nb_dummies+k;
                    pole_matrix[current_row][j] = new_pole_outer;
                }
                //add profile curve from outer section only if it is last section
                if(i==(profileCurves.size()-1)){
                    current_row+= 1;
                    pole_matrix[current_row][j] = outer_originalCurve_pnt;
                }
            }
        }

        //create knots in v-direction
        //create knot-matrix in v-direction: 
        //Assume that knots are locateted where poles are (which is by implementation: intersections between Profiles/Dummy-Profiles and Curves in v-direction)
        std::vector<std::vector<double>> v_knots(pole_matrix.size(),pole_matrix[0].size());
        //iterate through pole_matrix columns and calculate distances between pole-vectors
        //since the knots are represented by a value relative to total value of curve_length = 1
        //divide these distances by total length of the curve in v-direction to create a parameter for the knot vector
        num_v_knots = pole_matrix[0].size();

        for(int j=0; j < num_v_knots; j++){
            auto curve_length = fabs(pole_matrix[num_v_knots][j] - pole_matrix[0][j]); //CHECK:included math.h?
            for(int i= o; i < pole_matrix.size(); i++){
                v_knots[i][j] = pole_matrix[i][j]-pole_matrix[0][j]/curve_length;
            }
        }

        std::vector<unsigned> multiplicities(num_v_knots);
        //define multiplicities for all the curves in v-direction
        for(int i=0; i < num_v_knots; i++){
            //first knot degree+1 times (degree=3?)
            //+ k*2 knots
        }
         
        //interpolate curves in v-direction? TO SPECIFY: how to create Geom B-Spline Surface from poles
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

}