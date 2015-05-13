/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2015-04-08 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
#include "contrib/MakePatches.hxx" 
#include "contrib/MakeLoops.hxx" 

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Precision.hxx>
#include <GeomFill_FillingStyle.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Shape.hxx>

/******************************************************************************/
// test surface maker for single segment
/******************************************************************************/
TEST(guideCurvePatches, segment)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_segmentGuides.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_segmentProfiles.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    TopoDS_Shape faces = SurfMaker.Patches();
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    BRepTools::Write(SurfMaker.Patches(), "TestData/export/guideCurvePatch_segment.brep");
}

/******************************************************************************/
// test surface maker for single wing with two segments
/******************************************************************************/
TEST(guideCurvePatches, simpleWing)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_simpleWingGuides.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_simpleWingProfiles.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    TopoDS_Shape faces = SurfMaker.Patches();
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    BRepTools::Write(SurfMaker.Patches(), "TestData/export/guideCurvePatch_simpleWing.brep");
}

/******************************************************************************/
// test surface maker for nacelle (closed guide curves) with open profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelle)
{
    TopoDS_Shape guides;
    BRep_Builder b;
    ifstream in;
    in.open("TestData/guideCurvePatch_nacelleGuides.brep");
    BRepTools::Read(guides, in, b);
    in.close();
    
    TopoDS_Shape profiles;
    in.open("TestData/guideCurvePatch_nacelleProfiles.brep");
    BRepTools::Read(profiles, in, b);
    in.close();
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    TopoDS_Shape faces = SurfMaker.Patches();
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    SurfMaker.Patches();
    BRepTools::Write(SurfMaker.Patches(), "TestData/export/guideCurvePatch_nacellePatches.brep");
}


/******************************************************************************/
// test surface maker for nacelle (closed guide curves) with closed profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelleClosedProfiles)
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
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    TopoDS_Shape faces = SurfMaker.Patches();
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    SurfMaker.Patches();
    BRepTools::Write(SurfMaker.Patches(), "TestData/export/guideCurvePatch_nacelleClosedProfilesPatches.brep");
}


/******************************************************************************/
// test surface maker for nacelle with inverted (closed guide curves) with open profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelleInverted)
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
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    TopoDS_Shape faces = SurfMaker.Patches();
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    SurfMaker.Patches();
    BRepTools::Write(SurfMaker.Patches(), "TestData/export/guideCurvePatch_nacellePatchesInverted.brep");
}
