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
* @brief  Implementation of CPACS configuration header handling routines.
*/

#include "CCPACSHeader.h"
#include "TixiSaveExt.h"

namespace tigl
{

// Constructor
CCPACSHeader::CCPACSHeader(const std::string& aName, const std::string& aCreator, const std::string& aTimestamp)
    : name(aName)
    , creator(aCreator)
    , timestamp(aTimestamp)
{
}

// Destructor
CCPACSHeader::~CCPACSHeader(void)
{
    Cleanup();
}

std::string CCPACSHeader::GetName(void) const
{
    return name;
}

std::string CCPACSHeader::GetCreator(void) const
{
    return creator;
}

std::string CCPACSHeader::GetTimestamp(void) const
{
    return timestamp;
}

void CCPACSHeader::SetDescription(const std::string& aDescription)
{
    description = aDescription;
}

const std::string& CCPACSHeader::GetDescription(void) const
{
    return description;
}

// Read CPACS header elements
void CCPACSHeader::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    Cleanup();

    char* ptrName      = NULL;
    char* ptrCreator   = NULL;
    char* ptrDescription = NULL;
    char* ptrTimestamp = NULL;

    if (tixiGetTextElement(tixiHandle, "/cpacs/header/name",      &ptrName) == SUCCESS) {
        name      = ptrName;
    }

    if (tixiGetTextElement(tixiHandle, "/cpacs/header/creator",   &ptrCreator) == SUCCESS) {
        creator   = ptrCreator;
    }

    if (tixiGetTextElement(tixiHandle, "/cpacs/header/description", &ptrDescription) == SUCCESS) {
        description = ptrDescription;
    }

    if (tixiGetTextElement(tixiHandle, "/cpacs/header/timestamp", &ptrTimestamp) == SUCCESS) {
        timestamp = ptrTimestamp;
    }
}

// Write (and Save) header element, or create it if doesn't exist yet
void CCPACSHeader::WriteCPACS(TixiDocumentHandle tixiHandle)
{
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "name", name.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "creator", creator.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "description", description.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "timestamp", timestamp.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "version", "1.0"); // TODO : let the user choose the version of his project
    TixiSaveExt::TixiSaveTextElement(tixiHandle, "/cpacs/header", "cpacsVersion", tiglGetVersion());
}

// Cleanup routine
void CCPACSHeader::Cleanup(void)
{
    name      = "";
    creator   = "";
    timestamp = "";
    description = "";
}

} // end namespace tigl
