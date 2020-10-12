/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#include "CTiglPointsToBSplineInterpolation.h"
#include "CTiglCurvesToSurface.h"
#include "tiglcommonfunctions.h"
#include "CTiglTransformation.h"

TEST(TiglTransformation, transformSurface)
{
    auto pnts1 = OccArray({
        gp_Pnt(0., 0., 0.),
        gp_Pnt(1., 0, 0.),
    });

    auto pnts2 = OccArray({
        gp_Pnt(0., 1., 0.),
        gp_Pnt(1., 1.0, 0.),
    });

    auto c1   = tigl::CTiglPointsToBSplineInterpolation(pnts1).Curve();
    auto c2   = tigl::CTiglPointsToBSplineInterpolation(pnts2).Curve();
    auto surf = tigl::CTiglCurvesToSurface({c1, c2}).Surface();

    EXPECT_EQ(0.0, surf->Value(0., 0.).Distance(gp_Pnt(0., 0., 0.)));
    EXPECT_EQ(0.0, surf->Value(1., 1.).Distance(gp_Pnt(1., 1., 0.)));

    tigl::CTiglTransformation transl;
    transl.AddTranslation(0, 0, 1.);

    auto surfTrans = transl.Transform(surf);

    EXPECT_NEAR(0.0, surfTrans->Value(0., 0.).Distance(gp_Pnt(0., 0., 1.)), 1e-12);
    EXPECT_NEAR(0.0, surfTrans->Value(1., 1.).Distance(gp_Pnt(1., 1., 1.)), 1e-12);

    tigl::CTiglTransformation rot;
    rot.AddRotationZ(90.);

    auto surfRot = rot.Transform(surf);
    EXPECT_NEAR(0.0, surfRot->Value(0., 0.).Distance(gp_Pnt(0., 0., 0.)), 1e-12);
    EXPECT_NEAR(0.0, surfRot->Value(1., 1.).Distance(gp_Pnt(-1., 1., 0.)), 1e-12);


    tigl::CTiglTransformation scale;
    scale.AddScaling(2, 0.5, 1.);

    auto surfScale = scale.Transform(surf);
    EXPECT_NEAR(0.0, surfScale->Value(0., 0.).Distance(gp_Pnt(0., 0., 0.)), 1e-12);
    EXPECT_NEAR(0.0, surfScale->Value(1., 1.).Distance(gp_Pnt(2., 0.5, 0.)), 1e-12);
}
