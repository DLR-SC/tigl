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

#include "CTiglStandardizer.h"

void tigl::CTiglStandardizer::StandardizeFuselage(tigl::CCPACSFuselage& fuselage)
{
    CTiglPoint noseCenter                 = fuselage.GetNoseCenter();
    std::vector<std::string> elementsUIDS = fuselage.GetOrderedConnectedElement();
    if (elementsUIDS.size() < 2) {
        CTiglError("CTiglStandardizer: Impossible to standardize a fuselage with less than 2 valid elements!");
    }

    std::map<std::string, CTiglPoint> elementsCenters;
    CTiglFuselageSectionElement* cElementTemp = nullptr;
    for (int i = 0; i < elementsUIDS.size(); i++) {
        cElementTemp                        = fuselage.fuselageHelper->GetCTiglElementOfFuselage(elementsUIDS.at(i));
        elementsCenters[elementsUIDS.at(i)] = cElementTemp->GetCenter(GLOBAL_COORDINATE_SYSTEM);
    }

    // Remove all the positioning
    CCPACSPositionings& positionings                   = fuselage.GetPositionings(CreateIfNotExistsTag());
    std::vector<unique_ptr<CCPACSPositioning>>& refPos = positionings.GetPositionings();
    while (refPos.size() != 0) {
        positionings.RemovePositioning(*(refPos.front()));
    }

    // create the std positioning structure
    std::string fromUID = "";
    std::string toUID;
    CTiglPoint dummyDelta(1.11, 0, 0);
    for (int i = 0; i < elementsUIDS.size(); i++) {
        toUID = fuselage.fuselageHelper->GetCTiglElementOfFuselage(elementsUIDS.at(i))->GetSectionUID();
        positionings.CreatePositioning(fromUID, toUID, dummyDelta);
        fromUID = toUID;
    }

    // Set the transformation of the fuselage such that the nose center translation is performed by the fuselage transforamtion
    CCPACSTransformation& fuselageTransformation = fuselage.GetTransformation();
    fuselageTransformation.setTranslation(noseCenter);
    fuselage.Invalidate();

    // Reset the center of each section (positioning will be set in SetCenter function)
    cElementTemp = nullptr;
    for (int i = 0; i < elementsUIDS.size(); i++) {
        cElementTemp = fuselage.fuselageHelper->GetCTiglElementOfFuselage(elementsUIDS.at(i));
        cElementTemp->SetCenter(elementsCenters.at(elementsUIDS.at(i)));
    }
}
