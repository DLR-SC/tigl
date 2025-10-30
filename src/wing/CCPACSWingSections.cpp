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
* @brief  Implementation of CPACS wing sections handling routines.
*/

#include "CCPACSWingSections.h"
#include "CCPACSWingSection.h"
#include "CTiglError.h"
#include "CTiglUIDManager.h"
#include "CCPACSWingSectionElement.h"

namespace tigl
{
CCPACSWingSections::CCPACSWingSections(CCPACSWing* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSections(parent, uidMgr) {}

CCPACSWingSections::CCPACSWingSections(CCPACSEnginePylon* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingSections(parent, uidMgr) {}

CCPACSWingSection& CCPACSWingSections::CreateSection(const std::string& sectionUID, const std::string& profileUID)
{
    CTiglUIDManager& uidManager = GetUIDManager();

    CCPACSWingSection& newSection = AddSection();
    std::string newSectionUID     = uidManager.MakeUIDUnique(sectionUID);
    newSection.SetUID(newSectionUID);
    newSection.SetName(newSectionUID);
    newSection.GetTransformation().Init(uidManager.MakeUIDUnique(newSectionUID + "Tr"));

    CCPACSWingSectionElement& newElement = newSection.GetElements().AddElement();
    std::string newElementUID            = uidManager.MakeUIDUnique(newSectionUID + "Elem1");
    newElement.SetUID(newElementUID);
    newElement.SetName(newElementUID);
    newElement.GetTransformation().Init(uidManager.MakeUIDUnique(newElementUID + "Tr"));
    newElement.SetAirfoilUID(profileUID);

    return newSection;
}

} // end namespace tigl
