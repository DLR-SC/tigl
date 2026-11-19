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
#include <BRepCheck_Analyzer.hxx>
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
#include "CCPACSDucts.h"
#include "CCPACSDuct.h"
#include "CCPACSFuselage.h"
#include "CNamedShape.h"

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>


TEST(TestCTiglRoundedSegmentSurface, make_minimal_Shape)
{
    std::vector<Handle(Geom_Curve)> profileCurves;
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

TEST(TestCTiglRoundedSegmentSurface, openCpacsTwoRoundedSegments){

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
        ASSERT_TRUE(BRepCheck_Analyzer(wing).IsValid());
}


TEST(TestCTiglRoundedSegmentSurface, openCpacsThreeRoundedSegments){

    const char* filename = "TestData/simpletest-roundedSegmentSurface2.cpacs.xml";
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
    ASSERT_TRUE(BRepCheck_Analyzer(wing).IsValid());
}


namespace
{

double Volume(const TopoDS_Shape& shape)
{
    GProp_GProps props;
    BRepGProp::VolumeProperties(shape, props);
    return props.Mass();
}

// opens a CPACS file, optionally modifies the document before the configuration is created
class RoundedSegmentsConfig
{
public:
    explicit RoundedSegmentsConfig(const char* filename, const std::function<void(TixiDocumentHandle)>& modify = {})
    {
        EXPECT_EQ(SUCCESS, tixiOpenDocument(filename, &tixiHandle));
        if (modify) {
            modify(tixiHandle);
        }
        EXPECT_EQ(TIGL_SUCCESS, tiglOpenCPACSConfiguration(tixiHandle, "", &tiglHandle));
    }

    ~RoundedSegmentsConfig()
    {
        tiglCloseCPACSConfiguration(tiglHandle);
        tixiCloseDocument(tixiHandle);
    }

    tigl::CCPACSConfiguration& Config()
    {
        return tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(tiglHandle);
    }

    TixiDocumentHandle tixiHandle = -1;
    TiglCPACSConfigurationHandle tiglHandle = -1;
};

const char* roundedFuselageFile = "TestData/simpletest-roundedFuselage.cpacs.xml";
const char* roundedFuselagePath = "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]";

} // namespace

TEST(TestCTiglRoundedSegmentSurface, roundedFuselage)
{
    RoundedSegmentsConfig rounded(roundedFuselageFile);
    RoundedSegmentsConfig notRounded(roundedFuselageFile, [](TixiDocumentHandle tixi) {
        ASSERT_EQ(SUCCESS, tixiAddTextAttribute(tixi, roundedFuselagePath, "roundedSegments", "false"));
    });

    ASSERT_TRUE(*rounded.Config().GetFuselage(1).GetRoundedSegments());
    ASSERT_FALSE(*notRounded.Config().GetFuselage(1).GetRoundedSegments());

    TopoDS_Shape roundedLoft = rounded.Config().GetFuselage(1).GetLoft()->Shape();
    TopoDS_Shape defaultLoft = notRounded.Config().GetFuselage(1).GetLoft()->Shape();
    ASSERT_TRUE(BRepCheck_Analyzer(roundedLoft).IsValid());
    ASSERT_TRUE(BRepCheck_Analyzer(defaultLoft).IsValid());

    // the rounding must actually change the geometry
    double roundedVolume = Volume(roundedLoft);
    double defaultVolume = Volume(defaultLoft);
    EXPECT_GT(roundedVolume, 0.);
    EXPECT_GT(std::fabs(roundedVolume - defaultVolume) / defaultVolume, 1e-3);
}

TEST(TestCTiglRoundedSegmentSurface, roundedFuselageFaceTraits)
{
    RoundedSegmentsConfig rounded(roundedFuselageFile);
    PNamedShape loft = rounded.Config().GetFuselage(1).GetLoft();

    std::vector<std::string> names;
    for (unsigned int iFace = 0; iFace < loft->GetFaceCount(); ++iFace) {
        names.push_back(loft->GetFaceTraits(iFace).Name());
        EXPECT_FALSE(names.back().empty()) << "face " << iFace;
    }
    EXPECT_NE(std::find(names.begin(), names.end(), "Front"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "Rear"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), loft->Name()), names.end());
}

TEST(TestCTiglRoundedSegmentSurface, roundedFuselageMissingDistanceThrows)
{
    RoundedSegmentsConfig rounded(roundedFuselageFile, [](TixiDocumentHandle tixi) {
        ASSERT_EQ(SUCCESS, tixiRemoveElement(tixi, (std::string(roundedFuselagePath) +
                                                    "/segments/segment[2]/innerRoundingDistance").c_str()));
    });
    try {
        rounded.Config().GetFuselage(1).GetLoft();
        FAIL() << "Expected a CTiglError due to the missing innerRoundingDistance";
    }
    catch (const tigl::CTiglError& err) {
        EXPECT_NE(std::string(err.what()).find("innerRoundingDistance"), std::string::npos) << err.what();
        EXPECT_NE(std::string(err.what()).find("segmentD150_Fuselage_1Segment3ID"), std::string::npos) << err.what();
    }
}

TEST(TestCTiglRoundedSegmentSurface, roundedFuselageWithGuidesAndDifferentProfiles)
{
    // This fuselage uses guide curves and a different profile in every section.
    // If roundedSegments is set, the rounded algorithm must be used and the guides are ignored.
    const char* path = "/cpacs/vehicles/aircraft/model/fuselages/fuselage[1]";
    auto makeRounded = [path](TixiDocumentHandle tixi) {
        ASSERT_EQ(SUCCESS, tixiAddTextAttribute(tixi, path, "roundedSegments", "true"));
        ASSERT_EQ(SUCCESS, tixiAddDoubleElement(tixi, (std::string(path) + "/segments/segment[1]").c_str(),
                                                "outerRoundingDistance", 0.2, "%g"));
        ASSERT_EQ(SUCCESS, tixiAddDoubleElement(tixi, (std::string(path) + "/segments/segment[2]").c_str(),
                                                "innerRoundingDistance", 0.2, "%g"));
    };

    RoundedSegmentsConfig rounded("TestData/simple_test_guide_curves.xml", makeRounded);
    RoundedSegmentsConfig withGuides("TestData/simple_test_guide_curves.xml");

    TopoDS_Shape roundedLoft = rounded.Config().GetFuselage(1).GetLoft()->Shape();
    TopoDS_Shape guidesLoft  = withGuides.Config().GetFuselage(1).GetLoft()->Shape();
    ASSERT_TRUE(BRepCheck_Analyzer(roundedLoft).IsValid());
    ASSERT_TRUE(BRepCheck_Analyzer(guidesLoft).IsValid());
    EXPECT_GT(std::fabs(Volume(roundedLoft) - Volume(guidesLoft)) / Volume(guidesLoft), 1e-3);
}

TEST(TestCTiglRoundedSegmentSurface, makeLoftIgnoresGuidesIfRounded)
{
    // three non-rational profile curves of different size
    auto makeProfile = [](double x, double scale) {
        TColgp_Array1OfPnt poles(1, 7);
        poles.SetValue(1, gp_Pnt(x, 1.0 * scale, 0.));
        poles.SetValue(2, gp_Pnt(x, 0.66 * scale, -0.3 * scale));
        poles.SetValue(3, gp_Pnt(x, 0., -0.5 * scale));
        poles.SetValue(4, gp_Pnt(x, -0.5 * scale, 0.));
        poles.SetValue(5, gp_Pnt(x, 0., 0.5 * scale));
        poles.SetValue(6, gp_Pnt(x, 0.66 * scale, 0.3 * scale));
        poles.SetValue(7, gp_Pnt(x, 1.0 * scale, 0.));
        TColStd_Array1OfReal knots(1, 5);
        TColStd_Array1OfInteger mults(1, 5);
        for (int i = 1; i <= 5; ++i) {
            knots.SetValue(i, (i - 1) / 4.);
            mults.SetValue(i, (i == 1 || i == 5) ? 4 : 1);
        }
        Handle(Geom_BSplineCurve) curve = new Geom_BSplineCurve(poles, knots, mults, 3);
        return BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(curve).Edge()).Wire();
    };

    auto makeLofter = [&](CTiglMakeLoft& lofter) {
        lofter.addProfiles(makeProfile(0., 1.));
        lofter.addProfiles(makeProfile(1., 1.5));
        lofter.addProfiles(makeProfile(2., 1.));
        lofter.setRoundedSegments(true);
        lofter.setInnerRoundingDistance({0., 0.25});
        lofter.setOuterRoundingDistance({0.25, 0.});
    };

    CTiglMakeLoft roundedLofter;
    makeLofter(roundedLofter);

    CTiglMakeLoft roundedWithGuidesLofter;
    makeLofter(roundedWithGuidesLofter);
    roundedWithGuidesLofter.addGuides(
        BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Pnt(0., 1., 0.), gp_Pnt(2., 1., 0.)).Edge()).Wire());

    double expected = Volume(roundedLofter.Shape());
    EXPECT_GT(expected, 0.);
    EXPECT_NEAR(expected, Volume(roundedWithGuidesLofter.Shape()), 1e-10);
}

TEST(TestCTiglRoundedSegmentSurface, roundedDuct)
{
    const char* path = "/cpacs/vehicles/aircraft/model/ducts/duct[1]";
    auto makeRounded = [path](TixiDocumentHandle tixi) {
        ASSERT_EQ(SUCCESS, tixiAddTextAttribute(tixi, path, "roundedSegments", "true"));
        ASSERT_EQ(SUCCESS, tixiAddDoubleElement(tixi, (std::string(path) + "/segments/segment[1]").c_str(),
                                                "outerRoundingDistance", 0.25, "%g"));
        ASSERT_EQ(SUCCESS, tixiAddDoubleElement(tixi, (std::string(path) + "/segments/segment[2]").c_str(),
                                                "innerRoundingDistance", 0.25, "%g"));
    };
    RoundedSegmentsConfig rounded("TestData/simpletest_ducts.xml", makeRounded);

    const tigl::CCPACSDuct& duct = rounded.Config().GetDucts()->GetDuct("SimpleDuct");
    ASSERT_TRUE(*duct.GetRoundedSegments());
    PNamedShape loft = duct.GetLoft();
    ASSERT_TRUE(BRepCheck_Analyzer(loft->Shape()).IsValid());
    EXPECT_GT(Volume(loft->Shape()), 0.);
    for (unsigned int iFace = 0; iFace < loft->GetFaceCount(); ++iFace) {
        EXPECT_FALSE(loft->GetFaceTraits(iFace).Name().empty()) << "face " << iFace;
    }
}
