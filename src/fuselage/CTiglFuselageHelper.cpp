/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglFuselageHelper.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSegments.h"
#include "CCPACSFuselageSections.h"
#include "ListFunctions.h"
#include "CCPACSFuselageSections.h"
#include "CCPACSFuselageSection.h"

tigl::CTiglFuselageHelper::CTiglFuselageHelper(tigl::CCPACSFuselage* associatedFuselage)
{
    SetFuselage(associatedFuselage);
}

void tigl::CTiglFuselageHelper::SetFuselage(CCPACSFuselage* associatedFuselage)
{
    fuselage = associatedFuselage;
    if (fuselage) {
        elementUIDs      = fuselage->GetSegments().GetElementUIDsInOrder();
        cTiglElementsMap = BuildCTiglElementsMap();
    }
    else {
        elementUIDs.clear();
        cTiglElementsMap.clear();
    }
}

tigl::CTiglFuselageHelper::CTiglFuselageHelper()
{
    fuselage = nullptr;
}

std::map<std::string, tigl::CTiglFuselageSectionElement*> tigl::CTiglFuselageHelper::BuildCTiglElementsMap()
{
    std::map<std::string, CTiglFuselageSectionElement*> map;
    for (int s = 0; s < fuselage->GetSections().GetSectionCount(); s++) {
        for (int e = 1; e <= fuselage->GetSection(s + 1).GetSectionElementCount(); e++) {
            CCPACSFuselageSectionElement& tempElement = fuselage->GetSection(s + 1).GetSectionElement(e);
            map[tempElement.GetUID()]                 = tempElement.GetCTiglSectionElement();
        }
    }
    return map;
}

bool tigl::CTiglFuselageHelper::HasShape() const
{
    if (elementUIDs.size() >= 2) {
        return true;
    }
    return false;
}

std::string tigl::CTiglFuselageHelper::GetNoseUID() const
{

    if (HasShape()) {
        return elementUIDs[0];
    }
    else {
        LOG(WARNING) << "tigl::CTiglFuselageHelper::GetNoseUID: The fuselage does not seem to include any elements,"
	                " therefore, an empty uid is returned!";
        return "";
    }
}

std::string tigl::CTiglFuselageHelper::GetTailUID() const
{

    if (HasShape()) {
        return elementUIDs[elementUIDs.size() - 1];
    }
    else {
        LOG(WARNING) << "tigl::CTiglFuselageHelper::GetTailUID: The fuselage does not seem to include any elements,"
	                " therefore, an empty uid is returned!";
        return "";
    }
}

tigl::CTiglFuselageSectionElement* tigl::CTiglFuselageHelper::GetCTiglElementOfFuselage(std::string elementUID) const
{
    CTiglFuselageSectionElement* cElement = nullptr;

    try {
        return cTiglElementsMap.at(elementUID);
    }
    catch (const std::out_of_range&) {
        LOG(ERROR) << "CTiglFuselageHelper::GetCTiglElementOfFuselage: The given element UID:  " + elementUID +
                          " seems not to be present in this fuselage.";
        return nullptr;
    }
}
