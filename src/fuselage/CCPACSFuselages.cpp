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
* @brief  Implementation of CPACS fuselages handling routines.
*/

#include "CCPACSFuselages.h"
#include "CCPACSFuselage.h"
#include "CCPACSAircraftModel.h"
#include "CCPACSFuselageProfiles.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglStandardizer.h"

namespace tigl
{
CCPACSFuselages::CCPACSFuselages(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselages(parent, uidMgr) {}

CCPACSFuselages::CCPACSFuselages(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSFuselages(parent, uidMgr) {}

// Invalidates internal state
void CCPACSFuselages::Invalidate()
{
    for (int i = 1; i <= GetFuselageCount(); i++) {
        GetFuselage(i).Invalidate();
    }
}

// Read CPACS fuselages element
void CCPACSFuselages::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath)
{
    generated::CPACSFuselages::ReadCPACS(tixiHandle, xpath);
}

// Write CPACS fuselage elements
void CCPACSFuselages::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& xpath) const
{
    generated::CPACSFuselages::WriteCPACS(tixiHandle, xpath);
}

// Returns the total count of fuselages in a configuration
int CCPACSFuselages::GetFuselageCount() const
{
    return static_cast<int>(m_fuselages.size());
}

// Returns the fuselage for a given index.
CCPACSFuselage& CCPACSFuselages::GetFuselage(int index) const
{
    index--;
    if (index < 0 || index >= GetFuselageCount()) {
        throw CTiglError("Invalid index in CCPACSFuselages::GetFuselage", TIGL_INDEX_ERROR);
    }
    return *m_fuselages[index];
}

// Returns the fuselage for a given UID.
CCPACSFuselage& CCPACSFuselages::GetFuselage(const std::string& UID) const
{
    return *m_fuselages[GetFuselageIndex(UID) - 1];
}

// Returns the fuselage index for a given UID.
int CCPACSFuselages::GetFuselageIndex(const std::string& UID) const
{
    for (int i=0; i < GetFuselageCount(); i++) {
        const std::string tmpUID(m_fuselages[i]->GetUID());
        if (tmpUID == UID) {
            return i+1;
        }
    }

    // UID not there
    throw CTiglError("Invalid UID in CCPACSFuselages::GetFuselageIndex", TIGL_UID_ERROR);
}

CCPACSFuselage& CCPACSFuselages::CreateFuselage(const std::string& fuselageUID, int numberOfSection,
                                                const std::string& profileUID)
{

    CTiglUIDManager& uidManager = GetUIDManager();

    // check if the fuselage uid is valid
    // We only make this check for the fuselage uid the other uid will be create and be unique by construction
    bool uidIsAlreadyPresent = uidManager.IsUIDRegistered(fuselageUID);
    if (uidIsAlreadyPresent) {
        throw CTiglError("Impossible to create a fuselage with the uid \"" +
                         fuselageUID + "\". This uid is already present in the file. Choose another uid.");
    }

    // check if the profile uid is valid
    try {
        uidManager.ResolveObject<CCPACSFuselageProfile>(profileUID);
    }
    catch (const CTiglError& e) {
        throw CTiglError("Impossible to create the fuselage with the profile uid \"" +
                         profileUID +
                         "\". This uid seems not to be present or to not reference a profile. Make sure to use a "
                         "fuselage profile UID present in the file.");
    }

    CCPACSFuselage& fuselage = AddFuselage();
    fuselage.SetUID(fuselageUID);
    fuselage.SetName(fuselageUID);
    fuselage.GetTransformation().Init(uidManager.MakeUIDUnique(fuselageUID + "Transformation"));
    boost::optional<std::string> description("Fuselage generated from the function CCPACSFuselages::CreateFuselage");
    fuselage.SetDescription(description);

    // Create section and element
    std::string tempSectionUID;
    std::string tempElementUID;
    CTiglTransformation tempElementTransformation;
    tempElementTransformation.SetIdentity();
    for (int i = 1; i <= numberOfSection; i++) {

        tigl::CCPACSFuselageSection& section = fuselage.GetSections().AddSection();
        tempSectionUID                       = uidManager.MakeUIDUnique(fuselageUID + "Sec" + std::to_string(i));
        section.SetUID(tempSectionUID);
        section.SetName(tempSectionUID);
        section.GetTransformation().Init(uidManager.MakeUIDUnique(tempSectionUID + "Tr"));

        tigl::CCPACSFuselageSectionElement& element = section.GetElements().AddElement();
        tempElementUID                              = uidManager.MakeUIDUnique(tempSectionUID + "Elem1");
        element.SetUID(tempElementUID);
        element.SetName(tempElementUID);
        element.GetTransformation().Init(uidManager.MakeUIDUnique(tempElementUID + "Tr"));
        element.GetTransformation().setTransformationMatrix(tempElementTransformation);
        element.SetProfileUID(profileUID);

        // increment the position of the element
        tempElementTransformation.AddTranslation(1, 0, 0);
    }

    // Create segment
    for (int i = 1; i < fuselage.GetSections().GetSectionCount(); i++) {
        CCPACSFuselageSectionElement& fromElement = fuselage.GetSection(i).GetSectionElement(1);
        CCPACSFuselageSectionElement& toElement   = fuselage.GetSection(i + 1).GetSectionElement(1);
        CCPACSFuselageSegment& segment            = fuselage.GetSegments().AddSegment();
        std::string segmentUID                    = uidManager.MakeUIDUnique(fuselageUID + "Seg" + std::to_string(i));
        segment.SetUID(segmentUID);
        segment.SetName(segmentUID);
        segment.SetFromElementUID(fromElement.GetUID());
        segment.SetToElementUID(toElement.GetUID());
    }
    // so that if we change the standard, creating the fuselage will automatically follow the new standard
    CTiglStandardizer::StandardizeFuselage(fuselage);
    return fuselage;
}

} // end namespace tigl
