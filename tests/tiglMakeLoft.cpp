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

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

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
    BRepTools::Write(loft, "TestData/export/makeLoft_nacellePatchesClosed.brep");
}
