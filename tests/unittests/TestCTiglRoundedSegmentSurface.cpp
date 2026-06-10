#include "CCPACSWingSection.h"
#include "CTiglRoundedSegmentSurface.h"
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


TEST(TestCTiglRoundedSegmentSurface, make_minimal_Shape)
{
    std::vector<Handle(Geom_BSplineCurve)> profileCurves;
    //Create Profile Curves Vector
    TColgp_HArray1OfPnt u_poles(1,7);
    u_poles.SetValue(1,gp_Pnt(1., 0., 0.));
    u_poles.SetValue(2,gp_Pnt(0.66, 0., -0.015));
    u_poles.SetValue(3,gp_Pnt(0., 0., -0.1));
    u_poles.SetValue(4,gp_Pnt(0., 0., 0.));
    u_poles.SetValue(5,gp_Pnt(0., 0., 0.1));
    u_poles.SetValue(6,gp_Pnt(0.33, 0., 0.05));
    u_poles.SetValue(7,gp_Pnt(1., 0., 0.));

    TColgp_HArray1OfPnt u1_poles(1,7);
    u1_poles.SetValue(1,gp_Pnt(1., 0., 0.));
    u1_poles.SetValue(2,gp_Pnt(0.66, 0., -0.015));
    u1_poles.SetValue(3,gp_Pnt(0., 0., -0.1));
    u1_poles.SetValue(4,gp_Pnt(0., 0., 0.));
    u1_poles.SetValue(5,gp_Pnt(0., 0., 0.1));
    u1_poles.SetValue(6,gp_Pnt(0.33, 0., 0.05));
    u1_poles.SetValue(7,gp_Pnt(1., 0., 0.));

    TColgp_HArray1OfPnt u2_poles(1,7);
    u2_poles.SetValue(1,gp_Pnt(1., 0., 0.));
    u2_poles.SetValue(2,gp_Pnt(0.66, 0., -0.015));
    u2_poles.SetValue(3,gp_Pnt(0., 0., -0.1));
    u2_poles.SetValue(4,gp_Pnt(0., 0., 0.));
    u2_poles.SetValue(5,gp_Pnt(0., 0., 0.1));
    u2_poles.SetValue(6,gp_Pnt(0.33, 0., 0.05));
    u2_poles.SetValue(7,gp_Pnt(1., 0., 0.));


    TColStd_Array1OfReal u_knots(1,3);
    u_knots.SetValue(1,-1.);
    u_knots.SetValue(2,0.);
    u_knots.SetValue(3,1.);

    TColStd_Array1OfInteger u_mults(1,3);
    u_mults.SetValue(1,4);
    u_mults.SetValue(2,3);
    u_mults.SetValue(3,4);

    int u_degree = 3;

    //Create three profiles for the loft
    auto profile_root = new Geom_BSplineCurve(u_poles, u_knots, u_mults, u_degree);
    profileCurves.push_back(profile_root);

    Handle(Geom_BSplineCurve) profile_mid = new Geom_BSplineCurve(u1_poles, u_knots, u_mults, u_degree);
    //Scale by (0.66,1.,0.66) Translate by (0.2,0.5,0.)
    for(int i= 1; i< profile_root->NbPoles() + 1;i++){
        double X = profile_mid->Poles().Value(i).Coord().X()*0.66+0.2;
        double Y = profile_mid->Poles().Value(i).Coord().Y()*1.+0.5;
        double Z = profile_mid->Poles().Value(i).Coord().Z()*0.66+0.;
        gp_Pnt transformed_pole(X,Y,Z);
        profile_mid->SetPole(i,transformed_pole);
    }

    profileCurves.push_back(profile_mid);

    Handle(Geom_BSplineCurve) profile_tip = new Geom_BSplineCurve(u2_poles, u_knots, u_mults, u_degree);
    //Scale by (0.2,1.,0.1) Translate by (1.,2.,0.)
    for(int i= 1; i< profile_root->NbPoles() + 1;i++){
        double X = profile_tip->Poles().Value(i).Coord().X()*0.2+1.;
        double Y = profile_tip->Poles().Value(i).Coord().Y()*1.+2.;
        double Z = profile_tip->Poles().Value(i).Coord().Z()*0.1+0.;
        gp_Pnt transformed_pole(X,Y,Z);
        profile_tip->SetPole(i,transformed_pole);
    }

    profileCurves.push_back(profile_tip);

    for(int i=0; i<profileCurves.size(); i++){
        BRepBuilderAPI_MakeEdge edge(profileCurves[i]);
        tigl::dumpShape(edge ,"ProfleCurves","curve",i);
    }

    std::vector<double> ird;
    std::vector<double> ord;
    ird.push_back(0.);
    ord.push_back(0.);
    for(int i=1; i< profileCurves.size()-1; i++){
        ird.push_back(0.25);
        ord.push_back(0.25);
    }
    ird.push_back(0.);
    ord.push_back(0.);

    tigl::CTiglRoundedSegmentSurface lofter(profileCurves, ird, ord);
    auto surface = lofter.Surface();
   // DEBUG DELETEME
    Handle(Geom_Surface) handle_surface = surface;
    TopoDS_Shape surf = BRepBuilderAPI_MakeFace(handle_surface, 1e-15);
    tigl::dumpShape(surf, "makeLoftWing", "Surface",1);
}

TEST(TestCTiglRoundedSegmentSurface, openCpacsRoundedSegments){

        const char* filename = "TestData/simpletest-roundedSegmentSurface.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        TixiDocumentHandle tiglHandle = -1;
        TiglCPACSConfigurationHandle tixiHandle = -1;


        tiglHandle = -1;
        tixiHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);

        tigl::CCPACSConfigurationManager& manager = tigl::CCPACSConfigurationManager::GetInstance();
        tigl::CCPACSConfiguration& config         = manager.GetConfiguration(tiglHandle);
        auto wing = config.GetWing(1).GetLoftWithCutouts();
        tigl::dumpShape(wing, "makeLoftWing", "Surface",2);
}


