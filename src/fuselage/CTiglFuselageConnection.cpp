/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: CCPACSFuselageConnection.cpp 2641 2017-03-30 21:08:46Z bgruber $ 
*
* Version: $Revision: 2641 $
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
* @file
* @brief  Implementation of CPACS fuselage connection handling routines.
*/

#include "CTiglFuselageConnection.h"
#include "CTiglError.h"
#include "CCPACSFuselage.h"
#include "CCPACSDuct.h"
#include "CCPACSHull.h"
#include "CCPACSHulls.h"
#include "CCPACSGenericFuelTank.h"
#include "CCPACSFuselageSections.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfiguration.h"
#include <iostream>

namespace tigl
{

CTiglFuselageConnection::CTiglFuselageConnection() : elementUID(NULL), segment(NULL) {}

// Constructor
CTiglFuselageConnection::CTiglFuselageConnection(const std::string& elementUID, CCPACSFuselageSegment* aSegment)
    : elementUID(&elementUID), segment(aSegment)
{
    // find the corresponding section to this segment
    int i = 0;
    for (auto& section: GetParentComponentSections().GetSections()) {
        ++i;
        for (int j = 1; j <= section->GetSectionElementCount(); j++) {
            if (section->GetSectionElement(j).GetUID() == elementUID) {
                sectionUID = section->GetUID();
                sectionIndex = i;
                elementIndex = j;
            }
        }
    }
}

// Returns the section index of this connection
const std::string& CTiglFuselageConnection::GetSectionUID() const
{
    return sectionUID;
}

// Returns the section element index of this connection
const std::string& CTiglFuselageConnection::GetSectionElementUID() const
{
    return *elementUID;
}


// Returns the section index of this connection
int CTiglFuselageConnection::GetSectionIndex() const
{
    return sectionIndex;
}

// Returns the section element index of this connection
int CTiglFuselageConnection::GetSectionElementIndex() const
{
    return elementIndex;
}


// Returns the fuselage profile referenced by this connection
CCPACSFuselageProfile& CTiglFuselageConnection::GetProfile()
{
    std::string profileUID;

    bool found = false;
    for (auto& section: GetParentComponentSections().GetSections()) {
        for (int j=1; j <= section->GetSectionElementCount(); j++) {
            if (section->GetSectionElement(j).GetUID() == *elementUID ) {
                CCPACSFuselageSectionElement& element = section->GetSectionElement(j);
                profileUID = element.GetProfileUID();
                found = true;
                break;
            }
        }
        if ( found ) {
            break;
        }
    }
    CCPACSConfiguration const& config = segment->GetParent()->GetConfiguration();

    return (config.GetFuselageProfile(profileUID));
}

const CCPACSFuselageProfile& CTiglFuselageConnection::GetProfile() const {
    return const_cast<CTiglFuselageConnection&>(*this).GetProfile();
}

// Returns the positioning transformation for the referenced section
boost::optional<CTiglTransformation> CTiglFuselageConnection::GetPositioningTransformation() const
{
    boost::optional<CTiglTransformation> ret;
    if (GetParentComponentPositionings()) {
        ret = GetParentComponentPositionings()->GetPositioningTransformation(sectionUID);
    }
    return ret;
}

// Returns the section matrix referenced by this connection
CTiglTransformation CTiglFuselageConnection::GetSectionTransformation() const
{
    CTiglTransformation transformation;

    for (auto& section: GetParentComponentSections().GetSections()) {
        for (int j = 1; j <= section->GetSectionElementCount(); j++) {
            if (section->GetSectionElement(j).GetUID() == *elementUID) {
                transformation = section->GetSectionTransformation();
            }
        }
    }
    return transformation;
}

// Returns the section element matrix referenced by this connection
CTiglTransformation CTiglFuselageConnection::GetSectionElementTransformation() const
{
    CTiglTransformation transformation;

    for (auto& section: GetParentComponentSections().GetSections()) {
        for (int j = 1; j <= section->GetSectionElementCount(); j++) {
            if (section->GetSectionElement(j).GetUID() == *elementUID) {
                CCPACSFuselageSectionElement& element = section->GetSectionElement(j);
                transformation = element.GetSectionElementTransformation();
            }
        }  
    }
    return transformation;
}

CCPACSFuselageSections const& CTiglFuselageConnection::GetParentComponentSections() const
{
    if (segment->GetParent()->IsParent<CCPACSFuselage>()) {
        return segment->GetParent()->GetParent<CCPACSFuselage>()->GetSections();
    }
    else if (segment->GetParent()->IsParent<CCPACSDuct>()) {
        return segment->GetParent()->GetParent<CCPACSDuct>()->GetSections();
    }
    // ToDo: add exception handling
    else if (segment->GetParent()->IsParent<CCPACSHull>()) {
        return segment->GetParent()->GetParent<CCPACSHull>()->GetSections_choice1().get();
    }
    else {
        throw CTiglError("CTiglFuselageConnection: Unknown parent for segment.");
    }
}

boost::optional<CCPACSPositionings>& CTiglFuselageConnection::GetParentComponentPositionings() const
{
    if (segment->GetParent()->IsParent<CCPACSFuselage>()) {
        return segment->GetParent()->GetParent<CCPACSFuselage>()->GetPositionings();
    }
    else if (segment->GetParent()->IsParent<CCPACSDuct>()) {
        return segment->GetParent()->GetParent<CCPACSDuct>()->GetPositionings();
    }
    else if (segment->GetParent()->IsParent<CCPACSHull>()) {
        return segment->GetParent()->GetParent<CCPACSHull>()->GetParent()->GetParent()->GetParent()->GetParent()->GetParent()->GetPositionings();
    }
    else {
        throw CTiglError("CTiglFuselageConnection: Unknown parent for segment.");
    }
}

} // end namespace tigl
