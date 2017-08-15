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
* @brief  Implementation of CPACS wing connection handling routines.
*/

#include <iostream>

#include "CCPACSWingConnection.h"
#include "CTiglError.h"
#include "CCPACSWing.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSegment.h"
#include "CCPACSConfiguration.h"
#include "TixiSaveExt.h"

namespace tigl
{

// Constructor
CCPACSWingConnection::CCPACSWingConnection(CCPACSWingSegment* aSegment)
    : segment(aSegment)
{
    Cleanup();
}

// Destructor
CCPACSWingConnection::~CCPACSWingConnection(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSWingConnection::Cleanup(void)
{
    sectionUID = "";
    elementUID = "";
    sectionIndex = -1;
    elementIndex = -1;
}

// Returns the section UID of this connection
const std::string& CCPACSWingConnection::GetSectionUID(void) const
{
    return sectionUID;
}

// Returns the section element UID of this connection
const std::string& CCPACSWingConnection::GetSectionElementUID(void) const
{
    return elementUID;
}

// Returns the section index of this connection
int CCPACSWingConnection::GetSectionIndex(void) const
{
    return sectionIndex;
}

// Returns the section element index of this connection
int CCPACSWingConnection::GetSectionElementIndex(void) const
{
    return elementIndex;
}

// Returns the wing profile referenced by this connection
CCPACSWingProfile& CCPACSWingConnection::GetProfile(void) const
{
    CCPACSWing& wing = segment->GetWing();
    std::string profileUID;

    for (int i=1; i <= wing.GetSectionCount(); i++) {
        CCPACSWingSection& section = wing.GetSection(i);
        for (int j=1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID ) {
                CCPACSWingSectionElement& element = section.GetSectionElement(j);
                profileUID = element.GetProfileIndex();
            }
        }
    }
    CCPACSConfiguration& config = wing.GetConfiguration();

    return (config.GetWingProfile(profileUID));
}

// Returns the positioning transformation (segment transformation) for the referenced section
CTiglTransformation CCPACSWingConnection::GetPositioningTransformation(void) const
{
    return (segment->GetWing().GetPositioningTransformation(sectionUID));
}

// Returns the section matrix referenced by this connection
CTiglTransformation CCPACSWingConnection::GetSectionTransformation(void) const
{
    CCPACSWing& wing           = segment->GetWing();
    CTiglTransformation    transformation;

    for (int i = 1; i <= wing.GetSectionCount(); i++) {
        CCPACSWingSection& section = wing.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID) {
                transformation = section.GetSectionTransformation();
            }
        }
    }
    return transformation;
}

// Returns the section element matrix referenced by this connection
CTiglTransformation CCPACSWingConnection::GetSectionElementTransformation(void) const
{
    CCPACSWing& wing = segment->GetWing();
    CTiglTransformation transformation;

    for (int i = 1; i <= wing.GetSectionCount(); i++) {
        CCPACSWingSection& section = wing.GetSection(i);
        for (int j = 1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID) {
                CCPACSWingSectionElement& element = section.GetSectionElement(j);
                transformation = element.GetSectionElementTransformation();
            }
        }
    }
    return transformation;
}

// Read CPACS section elements
void CCPACSWingConnection::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& connectionXPath)
{
    Cleanup();

    // Get subelement "element"
    char*            ptrElementUID = NULL;
    if (tixiGetTextElement(tixiHandle, connectionXPath.c_str(), &ptrElementUID) != SUCCESS) {
        throw CTiglError("Error: Can't read element <elementUID/> in CCPACSWingConnection::ReadCPACS", TIGL_XML_ERROR);
    }
    elementUID = ptrElementUID;

    // find the corresponding section to this segment
    CCPACSWing& wing = segment->GetWing();
    for (int i=1; i <= wing.GetSectionCount(); i++) {
        CCPACSWingSection& section = wing.GetSection(i);
        for (int j=1; j <= section.GetSectionElementCount(); j++) {
            if (section.GetSectionElement(j).GetUID() == elementUID ) {
                sectionUID = section.GetUID();
                sectionIndex = i;
                elementIndex = j;
            }
        }
    }
}

// Write CPACS connection element
void CCPACSWingConnection::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& connectionXPath, const std::string& direction) const
{
    TixiSaveExt::TixiSaveTextElement(tixiHandle, connectionXPath.c_str(), direction.c_str(), elementUID.c_str());
}

} // end namespace tigl
