/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2018-12-04 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "test.h" // Brings in the GTest framework
#include "CCPACSTransformation.h"

TEST(tiglTransformation, setTransformationMatrix)
{
    double scale[3] = {2., 4., 8.};
    double rot[3]   = {50., 70., 10.};
    double trans[3] = {1., 2., 3.};

    // create CPACS-conform tigl-transformation (i.e. scaling -> euler-xyz-Rotation -> translation)
    tigl::CTiglTransformation tiglTrafo;
    tiglTrafo.AddScaling(scale[0], scale[1], scale[2]);
    tiglTrafo.AddRotationX(rot[0]);
    tiglTrafo.AddRotationY(rot[1]);
    tiglTrafo.AddRotationZ(rot[2]);
    tiglTrafo.AddTranslation(trans[0], trans[1], trans[2]);

    tigl::CCPACSTransformation cpacsTrafo(NULL);
    cpacsTrafo.setTransformationMatrix(tiglTrafo);

    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetX(), scale[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetY(), scale[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetScaling()->GetZ(), scale[2], 1e-8);

    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetX(), rot[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetY(), rot[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetRotation()->GetZ(), rot[2], 1e-8);

    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetX(), trans[0], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetY(), trans[1], 1e-8);
    EXPECT_NEAR(*cpacsTrafo.GetTranslation()->GetZ(), trans[2], 1e-8);
}
