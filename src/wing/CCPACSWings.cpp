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
* @brief  Implementation of CPACS wings handling routines.
*/

#include "CCPACSWings.h"

#include "CTiglError.h"
#include "CCPACSConfiguration.h"
#include "CCPACSAircraftModel.h"

#include "CCPACSWingSection.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSWingSegment.h"

#include "CTiglStandardizer.h"

namespace tigl
{

// Invalidates internal state
void CCPACSWings::Invalidate(const boost::optional<std::string>& source) const
{
    for (int i = 1; i <= GetWingCount(); i++) {
        GetWing(i).Invalidate(source);
    }
}

CCPACSWings::CCPACSWings(CCPACSRotorcraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWings(parent, uidMgr) {}

CCPACSWings::CCPACSWings(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWings(parent, uidMgr) {}


bool CCPACSWings::HasWing(const std::string & uid) const
{
    for (std::size_t i = 0; i < m_wings.size(); i++)
        if (m_wings[i]->GetUID() == uid)
            return true;
    return false;
}

CCPACSWing& CCPACSWings::CreateWing(const std::string& wingUID, int numberOfSection, const std::string& airfoilUID)
{

    CTiglUIDManager& uidManager = GetUIDManager();

    // check if the wing uid is valid
    bool uidIsAlreadyPresent = uidManager.IsUIDRegistered(wingUID);
    if (uidIsAlreadyPresent) {
        throw CTiglError("Impossible to create a wing with the uid \"" + wingUID +
                         "\". This uid is already present in the file. Choose another uid.");
    }

    // check if the profile uid is valid
    try {
        uidManager.ResolveObject<CCPACSWingProfile>(airfoilUID);
    }
    catch (const CTiglError&) {
        throw CTiglError("Impossible to create the wing with the profile uid \"" +
                          airfoilUID +
                         "\". This uid seems not to be present or to not reference a profile. Make sure to use a "
                         "wing profile UID  (aka airfoil) is present in the file.");
    }

    CCPACSWing& wing = AddWing();
    wing.SetUID(wingUID);
    wing.SetName(wingUID);
    wing.GetTransformation().Init(uidManager.MakeUIDUnique(wingUID + "Transformation"));
    boost::optional<std::string> description("Wing generated from the function CCPACSWings::CreateWing");
    wing.SetDescription(description);

    // Create section and element
    std::string tempSectionUID;
    std::string tempElementUID;
    CTiglTransformation tempElementTransformation;
    tempElementTransformation.SetIdentity();
    for (int i = 1; i <= numberOfSection; i++) {

        CCPACSWingSection& section = wing.GetSections().AddSection();
        tempSectionUID                   = uidManager.MakeUIDUnique(wingUID + "Sec" + std::to_string(i));
        section.SetUID(tempSectionUID);
        section.SetName(tempSectionUID);
        section.GetTransformation().Init(uidManager.MakeUIDUnique(tempSectionUID + "Tr"));

        CCPACSWingSectionElement& element = section.GetElements().AddElement();
        tempElementUID                          = uidManager.MakeUIDUnique(tempSectionUID + "Elem1");
        element.SetUID(tempElementUID);
        element.SetName(tempElementUID);
        element.GetTransformation().Init(uidManager.MakeUIDUnique(tempElementUID + "Tr"));
        element.GetTransformation().setTransformationMatrix(tempElementTransformation);
        element.SetAirfoilUID(airfoilUID);

        // increment the position of the element
        tempElementTransformation.AddTranslation(0, 1, 0);
    }

    // Create segment
    for (size_t i = 1; i < wing.GetSections().GetSectionCount(); i++) {
        CCPACSWingSectionElement& fromElement = wing.GetSection(i).GetSectionElement(1);
        CCPACSWingSectionElement& toElement   = wing.GetSection(i + 1).GetSectionElement(1);
        CCPACSWingSegment& segment            = wing.GetSegments().AddSegment();
        std::string segmentUID                    = uidManager.MakeUIDUnique(wingUID + "Seg" + std::to_string(i));
        segment.SetUID(segmentUID);
        segment.SetName(segmentUID);
        segment.SetFromElementUID(fromElement.GetUID());
        segment.SetToElementUID(toElement.GetUID());
    }

    CTiglStandardizer::StandardizeWing(wing);
    return wing;
}

} // end namespace tigl
