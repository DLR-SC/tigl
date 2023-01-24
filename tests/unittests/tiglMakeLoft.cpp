/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-13 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "test.h"
#include "tigl.h"
#include "CTiglMakeLoft.h"
#include "CTiglError.h"
#include "CCPACSConfigurationManager.h"

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopoDS.hxx>
#include <GeomConvert.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

#include <fstream>

using std::ifstream;

TEST(makeLoft, nacelleInverted)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_nacelleGuidesInverted.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_nacelleProfiles.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    CTiglMakeLoft loftMaker;
    loftMaker.addProfiles(profiles);
    loftMaker.addGuides(guides);
    loftMaker.setMakeSolid(false);
    TopoDS_Shape loft = loftMaker.Shape();
    ASSERT_FALSE(loft.IsNull());
    ASSERT_EQ(TopAbs_SHELL, loft.ShapeType());
    BRepTools::Write(loft, "TestData/export/makeLoft_nacellePatchesInverted.brep");
}

TEST(makeLoft, nacelleClosedInverted)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_nacelleGuidesInverted.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_nacelleProfilesClosed.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    CTiglMakeLoft loftMaker;
    loftMaker.addProfiles(profiles);
    loftMaker.addGuides(guides);
    TopoDS_Shape loft = loftMaker.Shape();
    ASSERT_FALSE(loft.IsNull());
    ASSERT_EQ(TopAbs_SOLID, loft.ShapeType());
    BRepTools::Write(loft, "TestData/export/makeLoft_nacellePatchesClosedInverted.brep");
}

TEST(makeLoft, nacelleClosed)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_nacelleGuides.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_nacelleProfilesClosed.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    CTiglMakeLoft loftMaker;
    loftMaker.addProfiles(profiles);
    loftMaker.addGuides(guides);
    TopoDS_Shape loft = loftMaker.Shape();
    ASSERT_FALSE(loft.IsNull());
    ASSERT_EQ(TopAbs_SOLID, loft.ShapeType());
    BRepTools::Write(loft, "TestData/export/makeLoft_nacellePatchesClosed.brep");
}

TEST(makeLoft, bug753)
{
    /**
     * The lofter should call an exception, as the number of wires
     * is not equal
     */
    BRep_Builder b;
    CTiglMakeLoft loftMaker;
    for (size_t i = 0; i < 4; ++i) {
        std::stringstream str;
        str << "TestData/bugs/753/4_profile_" << i << ".brep";
        TopoDS_Shape wire;
        EXPECT_EQ(Standard_True, BRepTools::Read(wire, str.str().c_str(), b));
        loftMaker.addProfiles(TopoDS::Wire(wire));
    }
    loftMaker.setMakeSolid(true);
    loftMaker.setMakeSmooth(true);
    ASSERT_THROW(loftMaker.Shape(), tigl::CTiglError);
}

TEST(makeLoft, bug753_cpacs)
{
    TiglHandleWrapper handle("TestData/bugs/753/bug753.cpacs.xml", "");
    const auto &fuselage = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(handle).GetFuselage(1);
    EXPECT_NO_THROW(fuselage.GetLoft());
}

class CurveNetworkCoons: public ::testing::TestWithParam<std::string>
{
protected:
    virtual void SetUp()
    {
        // get the name of the folder with the B-spline network data
        path_profiles = "TestData/CurveNetworks/" + GetParam() + "/profiles.brep";
        path_guides = "TestData/CurveNetworks/" + GetParam() + "/guides.brep";
        path_output = "TestData/CurveNetworks/" + GetParam() + "/result_coons.brep";
    }

    void TearDown()
    {
    }

    // name of the folder with the B-spline network data
    std::string path_profiles;
    std::string path_guides;
    std::string path_output;
};

TEST_P(CurveNetworkCoons, testFromBRep)
{
    // u-directional B-spline curves
    // first read the brep-input file
    TopoDS_Shape shape_u;

    BRep_Builder builder_u;

    BRepTools::Read(shape_u, path_profiles.c_str(), builder_u);

    TopExp_Explorer Explorer;
    // get the splines in u-direction from the Edges
    std::vector<Handle(Geom_BSplineCurve)> splines_u_vector;
    for (Explorer.Init(shape_u, TopAbs_EDGE); Explorer.More(); Explorer.Next()) {
        TopoDS_Edge curve_edge = TopoDS::Edge(Explorer.Current());
        double beginning = 0;
        double end = 1;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(curve_edge, beginning, end);
        Handle(Geom_BSplineCurve) spline = GeomConvert::CurveToBSplineCurve(curve);
        
        // search if spline is already in list (can happen for closed curve networks)
        bool alreadyInList = false;
        for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_u_vector.begin(); it != splines_u_vector.end(); ++it) {
            const Handle(Geom_BSplineCurve)& curve = *it;
            if (curve->IsEqual(spline, 1e-6)) {
                alreadyInList = true;
            }
        }
        if (!alreadyInList) {
            splines_u_vector.push_back(spline);
        }
    }

    // v-directional B-spline curves
    // first read the BRep-input file
    TopoDS_Shape shape_v;

    BRep_Builder builder_v;

    BRepTools::Read(shape_v, path_guides.c_str(), builder_v);

    // now filter out the Edges
    TopTools_IndexedMapOfShape mapEdges_v;
    TopExp::MapShapes(shape_v, TopAbs_EDGE, mapEdges_v);

    // get the splines in v-direction from the Edges
    std::vector<Handle(Geom_BSplineCurve)> splines_v_vector;
    for (Explorer.Init(shape_v, TopAbs_EDGE); Explorer.More(); Explorer.Next()) {
        TopoDS_Edge curve_edge = TopoDS::Edge(Explorer.Current());
        double beginning = 0;
        double end = 1;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(curve_edge, beginning, end);
        Handle(Geom_BSplineCurve) spline = GeomConvert::CurveToBSplineCurve(curve);

        // search if spline is already in list (can happen for closed curve networks)
        bool alreadyInList = false;
        for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_v_vector.begin(); it != splines_v_vector.end(); ++it) {
            const Handle(Geom_BSplineCurve)& curve = *it;
            if (curve->IsEqual(spline, 1e-6)) {
                alreadyInList = true;
            }
        }
        if (!alreadyInList) {
            splines_v_vector.push_back(spline);
        }
    }

    CTiglMakeLoft lofter(1e-6, 1e-4);
    lofter.setMakeSolid(false);
    lofter.useGordonSurfaceAlgorithm(false);
    
    for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_u_vector.begin(); it != splines_u_vector.end(); ++it) {
        const Handle(Geom_BSplineCurve)& curve = *it;
        lofter.addProfiles(BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(curve).Edge()).Wire());
    }
    for (std::vector<Handle(Geom_BSplineCurve)>::const_iterator it = splines_v_vector.begin(); it != splines_v_vector.end(); ++it) {
        const Handle(Geom_BSplineCurve)& curve = *it;
        lofter.addGuides(BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(curve).Edge()).Wire());
    }

    lofter.Perform();
    TopoDS_Shape loft = lofter.Shape();
    ASSERT_FALSE(loft.IsNull());
    BRepTools::Write(loft, path_output.c_str());
}

INSTANTIATE_TEST_CASE_P(makeLoft, CurveNetworkCoons, ::testing::Values(
                            "nacelle",
                            "full_nacelle",
                            "wing2",
                            /* "spiralwing", TODO: Coons has a problem with this case */
                            "test_surface4_sorted",
                            "test_surface4",
                            "wing3",
                            "bellyfairing",
                            "helibody",
                            "fuselage1",
                            "fuselage2"
                            ));
