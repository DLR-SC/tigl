/*
 * Copyright (C) 2007-2025 German Aerospace Center (DLR/SC)
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

/**
 * @file testSiblingComponentFuse.cpp
 * @brief Regression test: fusing must resolve mutual intersections between
 * sibling components, not just parent-child pairs (see GitHub issue #939)
 *
 * Issue #939: IGES/STEP fused export only considered parent-child relationships
 * in the CPACS tree, missing the fusing of sibling components (e.g., VTP and
 * HTP, both children of the fuselage, that also intersect one another and the
 * fuselage at the same location). Fusing them one pair at a time (fuselage+wing,
 * fuselage+HTP, fuselage+VTP, ...) instead of all at once cannot correctly
 * resolve such a mutual/triple intersection region.
 *
 * The test CPACS file (sibling_fuse_test.xml) contains:
 *   - Fuselage (SimpleFuselage, radius 0.5 in Y and Z)
 *   - Main wing (Wing, symmetric, spans y=[0,2])
 *   - HTP (TestHTP, x-z-plane symmetric, parent=SimpleFuselage)
 *   - VTP (TestVTP, rotated 90 deg around X, parent=SimpleFuselage)
 *
 * TestHTP and TestVTP overlap each other near the tail, and both also
 * intersect the fuselage in the same region, so all three components must be
 * fused together consistently in one pass.
 */

#include "test.h"

#include "tigl.h"

#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CTiglFusePlane.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepTools.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

namespace {

gp_Pnt GetFaceCenter(const TopoDS_Face& f)
{
    double u0, u1, v0, v1;
    BRepTools::UVBounds(f, u0, u1, v0, v1);
    BRepAdaptor_Surface adaptor(f);
    return adaptor.Value(0.5 * (u0 + u1), 0.5 * (v0 + v1));
}

} // namespace

TEST(SiblingComponentFuse, pairwiseSiblingFusing)
{
    TiglHandleWrapper handle("TestData/sibling_fuse_test.xml", "Cpacs2Test");

    tigl::CCPACSConfigurationManager& mgr = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration& config = mgr.GetConfiguration(handle);

    auto fuser = config.AircraftFusingAlgo();
    ASSERT_TRUE(fuser != nullptr);

    PNamedShape fused = fuser->FusedPlane();
    ASSERT_TRUE(fused != nullptr);
    ASSERT_FALSE(fused->Shape().IsNull());

    TopoDS_Shape shape = fused->Shape();

    BRepCheck_Analyzer analyzer(shape);
    ASSERT_TRUE(analyzer.IsValid(shape))
        << "Fused shape must be topologically valid (no self-intersections)";

    TopoDS_Solid solid = TopoDS::Solid(shape);
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);

    for (int i = 1; i <= faceMap.Extent(); ++i) {
        TopoDS_Face face = TopoDS::Face(faceMap(i));
        gp_Pnt center = GetFaceCenter(face);

        BRepClass3d_SolidClassifier classifier(solid);
        classifier.Perform(center, Precision::Confusion());

        EXPECT_NE(classifier.State(), TopAbs_IN)
            << "Face " << i << " has centroid inside the solid — indicates an "
            << "internal face from an unfused sibling intersection (VTP-HTP or "
            << "Wing-HTP overlap was not trimmed)";
    }
}
