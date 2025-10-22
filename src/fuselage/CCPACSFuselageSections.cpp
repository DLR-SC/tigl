/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @file 
* @brief  Implementation of CPACS fuselage sections handling routines.
*/

#include "CCPACSFuselageSections.h"
#include "CCPACSFuselageSection.h"
#include "CTiglError.h"
#include "CTiglUIDManager.h"

namespace tigl
{
CCPACSFuselageSections::CCPACSFuselageSections(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSections(parent, uidMgr) {}

CCPACSFuselageSections::CCPACSFuselageSections(CCPACSDuct* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSections(parent, uidMgr) {}

CCPACSFuselageSections::CCPACSFuselageSections(CCPACSVessel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselageSections(parent, uidMgr) {}


CCPACSFuselageSection&  CCPACSFuselageSections::CreateSection(const std::string& sectionUID, const std::string& profileUID)
{
    CTiglUIDManager& uidManager = GetUIDManager();

    CCPACSFuselageSection& newSection = AddSection();
    std::string newSectionUID = uidManager.MakeUIDUnique( sectionUID );
    newSection.SetUID(newSectionUID);
    newSection.SetName(newSectionUID);
    newSection.GetTransformation().Init(uidManager.MakeUIDUnique(newSectionUID + "Tr"));

    tigl::CCPACSFuselageSectionElement& newElement = newSection.GetElements().AddElement();
    std::string newElementUID = uidManager.MakeUIDUnique(newSectionUID + "Elem1");
    newElement.SetUID(newElementUID);
    newElement.SetName(newElementUID);
    newElement.GetTransformation().Init(uidManager.MakeUIDUnique(newElementUID + "Tr"));
    newElement.SetProfileUID(profileUID);

    return newSection;

}

} // end namespace tigl
