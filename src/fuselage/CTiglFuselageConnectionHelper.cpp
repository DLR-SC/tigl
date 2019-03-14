/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglFuselageConnectionHelper.h"
#include "CTiglError.h"

tigl::CTiglTransformation tigl::CTiglFuselageConnectionHelper::GetTransformToPlaceConnectionOriginByTranslationAt(
    const tigl::CTiglFuselageConnection& connection, const tigl::CTiglPoint& wantedOriginP)
{

    /* We search a new E' such that:
         * w = F*P*S*E'*0  (At start the origin is at 0,0,0)
         * S^-1 * P^-1 * F ^-1 w = E'0
         * A w = E'0
         * w' = E' * 0
         * w' = T' *R' * S' * 0 ( we decompose the E' into its scaling, rotation and translation, remark the S has not the same meaning as above)
         * w' = T' * 0 ( Because scaling and rotation has no effect on 0)
         * so we can deduce the wanted E'
         * E' = T' * R * S where S and R are the original scaling and rotation of the original E.
         *
         */
    // Do positioning transformations

    CTiglTransformation elementT = connection.GetSectionElementTransformation();
    CTiglTransformation sectionT = connection.GetSectionTransformation();
    CTiglTransformation positioningT = connection.GetPositioningTransformation();
    CTiglTransformation fuselageT = connection.GetFuselageTransformation();

    CTiglTransformation a = sectionT.Inverted() * positioningT.Inverted() * fuselageT.Inverted();
    CTiglPoint wp         = a * wantedOriginP;

    CTiglTransformation ep = elementT;

    ep.SetValue(0, 3, 0);
    ep.SetValue(1, 3, 0);
    ep.SetValue(2, 3, 0);
    ep.SetValue(3, 3, 1);

    ep.AddTranslation(wp.x, wp.y, wp.z);

    // check if it is correct
    CTiglPoint o(0, 0, 0);
    CTiglPoint check = fuselageT * positioningT * sectionT * ep * o;

    if (check.distance2(wantedOriginP) > 0.001) {
        throw CTiglError("CCPACSFuselage::GetTransformToPlaceElementByTranslationAt: Something go wrong!");
    }
    return ep;
}
