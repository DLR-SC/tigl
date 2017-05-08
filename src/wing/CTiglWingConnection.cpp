/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: CCPACSWingConnection.cpp 2698 2017-04-26 14:40:01Z bgruber $ 
*
* Version: $Revision: 2698 $
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
* @brief  Implementation of CPACS wing connection handling routines.
*/

#include <iostream>

#include "CTiglWingConnection.h"
#include "CTiglError.h"
#include "CCPACSWing.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSegment.h"
#include "CCPACSConfiguration.h"

namespace tigl
{
CTiglWingConnection::CTiglWingConnection() {}

    // Constructor
CTiglWingConnection::CTiglWingConnection(const std::string& elementUID, CCPACSWingSegment* aSegment)
    : elementUID(elementUID), segment(aSegment)
{
}

// Returns the section UID of this connection
const std::string& CTiglWingConnection::GetSectionUID() const
{
    resolve();
    return *m_resolved.sectionUidPtr;
}

// Returns the section element UID of this connection
const std::string& CTiglWingConnection::GetSectionElementUID() const
{
    return elementUID;
}

// Returns the section index of this connection
int CTiglWingConnection::GetSectionIndex() const
{
    resolve();
    return m_resolved.sectionIndex;
}


// Returns the section element index of this connection
int CTiglWingConnection::GetSectionElementIndex() const
{
    resolve();
    return m_resolved.elementIndex;
}

// Returns the wing profile referenced by this connection
CCPACSWingProfile& CTiglWingConnection::GetProfile() const
{
    resolve();
    return segment->GetWing().GetConfiguration().GetWingProfile(*m_resolved.profileUIDPtr);
}

// Returns the positioning transformation (segment transformation) for the referenced section
CTiglTransformation CTiglWingConnection::GetPositioningTransformation() const
{
    return segment->GetWing().GetPositioningTransformation(GetSectionUID());
}

// Returns the section matrix referenced by this connection
CTiglTransformation CTiglWingConnection::GetSectionTransformation() const
{
    CCPACSWing& wing = segment->GetWing();
    CTiglTransformation transformation;

    for (int i = 1; i <= wing.GetSectionCount(); i++) {
        const CCPACSWingSection& section = wing.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID) {
                transformation = section.GetSectionTransformation();
            }
        }
    }
    return transformation;
}

// Returns the section element matrix referenced by this connection
CTiglTransformation CTiglWingConnection::GetSectionElementTransformation() const
{
    CCPACSWing& wing = segment->GetWing();
    CTiglTransformation transformation;

    for (int i = 1; i <= wing.GetSectionCount(); i++) {
        const CCPACSWingSection& section = wing.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID) {
                const CCPACSWingSectionElement& element = section.GetSectionElement(j);
                transformation = element.GetSectionElementTransformation();
            }
        }
    }
    return transformation;
}


void CTiglWingConnection::resolve() const {
    CCPACSWing& wing = segment->GetWing();
    for (int i = 1; i <= wing.GetSectionCount(); i++) {
        const CCPACSWingSection& section = wing.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            const CCPACSWingSectionElement& element = section.GetSectionElement(j);
            if (element.GetUID() == elementUID) {
                m_resolved.sectionUidPtr = &section.GetUID();
                m_resolved.sectionIndex = i;
                m_resolved.elementIndex = j;
                m_resolved.profileUIDPtr = &element.GetProfileUID();
                return;
            }
        }
    }
    throw CTiglError("Could not resolve element UID");
}
} // end namespace tigl
