/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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

#include "CCPACSFuselageConnection.h"
#include "CTiglError.h"
#include "CCPACSFuselage.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSConfiguration.h"
#include <iostream>

namespace tigl
{

CCPACSFuselageConnection::CCPACSFuselageConnection() : elementUID(NULL), segment(NULL) {}

// Constructor
CCPACSFuselageConnection::CCPACSFuselageConnection(const std::string& elementUID, CCPACSFuselageSegment* aSegment)
    : elementUID(&elementUID), segment(aSegment)
{
    // find the corresponding section to this segment
    CCPACSFuselage& fuselage = segment->GetFuselage();
    for (int i = 1; i <= fuselage.GetSectionCount(); i++) {
        CCPACSFuselageSection& section = fuselage.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID) {
                sectionUID = section.GetUID();
                sectionIndex = i;
                elementIndex = j;
            }
        }
    }
}

// Returns the section index of this connection
const std::string& CCPACSFuselageConnection::GetSectionUID() const
{
    return sectionUID;
}

// Returns the section element index of this connection
const std::string& CCPACSFuselageConnection::GetSectionElementUID() const
{
    return *elementUID;
}


// Returns the section index of this connection
int CCPACSFuselageConnection::GetSectionIndex() const
{
    return sectionIndex;
}

// Returns the section element index of this connection
int CCPACSFuselageConnection::GetSectionElementIndex() const
{
    return elementIndex;
}


// Returns the fuselage profile referenced by this connection
CCPACSFuselageProfile& CCPACSFuselageConnection::GetProfile() const
{
    CCPACSFuselage& fuselage = segment->GetFuselage();
    std::string profileUID;

    bool found = false;
    for (int i=1; i <= fuselage.GetSectionCount(); i++) {
        CCPACSFuselageSection& section = fuselage.GetSection(i);
        for (int j=1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == *elementUID ) {
                CCPACSFuselageSectionElement& element = section.GetSectionElement(j);
                profileUID = element.GetProfileIndex();
                found = true;
                break;
            }
        }
        if ( found ) {
            break;
        }
    }
    CCPACSConfiguration& config = fuselage.GetConfiguration();

    return (config.GetFuselageProfile(profileUID));
}

// Returns the positioning transformation for the referenced section
CTiglTransformation CCPACSFuselageConnection::GetPositioningTransformation() const
{
    return (segment->GetFuselage().GetPositioningTransformation(sectionUID));
}

// Returns the section matrix referenced by this connection
CTiglTransformation CCPACSFuselageConnection::GetSectionTransformation() const
{
    CCPACSFuselage& fuselage = segment->GetFuselage();
    CTiglTransformation transformation;

    for (int i = 1; i <= fuselage.GetSectionCount(); i++) {
        CCPACSFuselageSection& section = fuselage.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == *elementUID) {
                transformation = section.GetSectionTransformation();
            }
        }
    }
    return transformation;
}

// Returns the section element matrix referenced by this connection
CTiglTransformation CCPACSFuselageConnection::GetSectionElementTransformation() const
{
    CCPACSFuselage& fuselage = segment->GetFuselage();
    CTiglTransformation transformation;

    for (int i = 1; i <= fuselage.GetSectionCount(); i++) {
        CCPACSFuselageSection& section = fuselage.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == *elementUID) {
                CCPACSFuselageSectionElement& element = section.GetSectionElement(j);
                transformation = element.GetSectionElementTransformation();
            }
        }  
    }
    return transformation;
}

} // end namespace tigl
