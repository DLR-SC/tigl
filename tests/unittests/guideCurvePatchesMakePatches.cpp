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
#include "tigl_config.h"
#include "contrib/MakePatches.hxx" 

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Precision.hxx>
#include <GeomFill_FillingStyle.hxx>

#ifdef HAVE_OCE_COONS_PATCHED
#define COONS_FILLING_STYLE GeomFill_CoonsC2Style
#else
#define COONS_FILLING_STYLE GeomFill_CoonsStyle
#endif

/******************************************************************************/
// test surface maker for single segment
/******************************************************************************/
TEST(guideCurvePatches, segment)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/guideCurvePatch_segmentGuides.brep", b);
    BRepTools::Read(profiles, "TestData/guideCurvePatch_segmentProfiles.brep", b);

    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/guideCurvePatch_segment.brep");
}

/******************************************************************************/
// test surface maker for single wing with two segments
/******************************************************************************/
TEST(guideCurvePatches, simpleWing)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/guideCurvePatch_simpleWingGuides.brep", b);
    BRepTools::Read(profiles, "TestData/guideCurvePatch_simpleWingProfiles.brep", b);

    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/guideCurvePatch_simpleWing.brep");
}

/******************************************************************************/
// test surface maker for nacelle (closed guide curves) with open profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelle)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/guideCurvePatch_nacelleGuides.brep", b);
    BRepTools::Read(profiles, "TestData/guideCurvePatch_nacelleProfiles.brep", b);

    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/guideCurvePatch_nacellePatches.brep");
}


/******************************************************************************/
// test surface maker for nacelle (closed guide curves) with closed profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelleClosedProfiles)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/guideCurvePatch_nacelleGuides.brep", b);
    BRepTools::Read(profiles, "TestData/guideCurvePatch_nacelleProfilesClosed.brep", b);
    
    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/guideCurvePatch_nacelleClosedProfilesPatches.brep");
}


/******************************************************************************/
// test surface maker for nacelle with inverted (closed guide curves) with open profiles
/******************************************************************************/
TEST(guideCurvePatches, nacelleInverted)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/guideCurvePatch_nacelleGuidesInverted.brep", b);
    BRepTools::Read(profiles, "TestData/guideCurvePatch_nacelleProfiles.brep", b);

    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    SurfMaker.Perform(tolConf, tolPara, style, Standard_True);
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/guideCurvePatch_nacellePatchesInverted.brep");
}

/******************************************************************************/
// test surface maker for heli fuselage
/******************************************************************************/
TEST(guideCurvePatches, bugHeliFuselage)
{
    TopoDS_Shape guides, profiles;
    BRep_Builder b;
    BRepTools::Read(guides, "TestData/bugs/145/Fuselage_guides.brep", b);
    BRepTools::Read(profiles, "TestData/bugs/145/Fuselage_profiles.brep", b);

    MakePatches SurfMaker(guides, profiles);
    Standard_Real tolConf = Precision::Confusion();
    Standard_Real tolPara = Precision::Confusion();
    GeomFill_FillingStyle style = COONS_FILLING_STYLE;
    ASSERT_NO_THROW(SurfMaker.Perform(tolConf, tolPara, style, Standard_True));
    ASSERT_EQ(SurfMaker.GetStatus(), 0);
    TopoDS_Shape faces = SurfMaker.Patches();
    BRepTools::Write(faces, "TestData/export/bug_145_helipatch.brep");
}
