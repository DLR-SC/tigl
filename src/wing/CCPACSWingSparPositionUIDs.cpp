/* 
* Copyright (C) 2016 Airbus Defence and Space
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

#include "CCPACSWingSparPositionUIDs.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "to_string.h"


namespace tigl
{

CCPACSWingSparPositionUIDs::CCPACSWingSparPositionUIDs()
{
    Cleanup();
}

CCPACSWingSparPositionUIDs::~CCPACSWingSparPositionUIDs()
{
    Cleanup();
}

void CCPACSWingSparPositionUIDs::Cleanup(void)
{
    positionUIDs.clear();
}

void CCPACSWingSparPositionUIDs::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& positionUIDsXPath)
{
    Cleanup();

    int positionUIDCount;
    ReturnCode tixiRet = tixiGetNamedChildrenCount(tixiHandle, positionUIDsXPath.c_str(), "sparPositionUID", &positionUIDCount);
    if (tixiRet != SUCCESS) {
        LOG(ERROR) << "Missing spar position UIDs";
        throw CTiglError("Error: Missing spar position UIDs in CCPACSWingSparPositionUIDs::ReadCPACS!", TIGL_XML_ERROR);
    }
    else if (positionUIDCount < 2) {
        LOG(ERROR) << "At least two sparPositionUIDs are required!";
        throw CTiglError("Error: At least two sparPositionUIDs are required in CCPACSWingSparPositionUIDs::ReadCPACS!", TIGL_XML_ERROR);
    }

    // Loop over all positionUIDs
    for (int i = 1; i <= positionUIDCount; i++) {
        char* ptrPositionUID = NULL;
        if (tixiGetTextElement(tixiHandle, (positionUIDsXPath + "/sparPositionUID[" + std_to_string(i) + "]").c_str(), &ptrPositionUID) != SUCCESS) {
            throw CTiglError("Error reading sparPositionUID with index " + std_to_string(i));
        }
        positionUIDs.push_back(ptrPositionUID);
    }
}

void CCPACSWingSparPositionUIDs::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& positionUIDsXPath) const
{
    int test;
    if (tixiGetNamedChildrenCount(tixiHandle, positionUIDsXPath.c_str(), "sparPositionUID", &test) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSWingSparPositions::WriteCPACS", TIGL_XML_ERROR);
    }
    const int uidCount = GetSparPositionUIDCount();

    const std::string elementBasePath = positionUIDsXPath + "/sparPositionUID";
    for (int i = 1; i <= uidCount; i++) {
        const std::string elementPath = elementBasePath + "[" + std_to_string(i) + "]";
        if (tixiCheckElement(tixiHandle, elementPath.c_str()) == ELEMENT_NOT_FOUND) {
            if (tixiAddTextElement(tixiHandle, positionUIDsXPath.c_str(), "sparPositionUID", GetSparPositionUID(i).c_str()) != SUCCESS) {
                LOG(ERROR) << "XML error: tixiCreateElement failed";
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWingSparPositionUIDs::WriteCPACS", TIGL_XML_ERROR);
            }
        }
        else {
            if (tixiUpdateTextElement(tixiHandle, elementPath.c_str(), GetSparPositionUID(i).c_str()) != SUCCESS) {
                LOG(ERROR) << "XML error: tixiCreateElement failed";
                throw CTiglError("XML error: tixiCreateElement failed in CCPACSWingSparPositionUIDs::WriteCPACS", TIGL_XML_ERROR);
            }
        }
    }

    for (int i = uidCount+1; i <= test; i++) {
        const std::string elementPath = elementBasePath + "[" + std_to_string(uidCount + 1) + "]";
        tixiRemoveElement(tixiHandle, elementPath.c_str());
    }
}

int CCPACSWingSparPositionUIDs::GetSparPositionUIDCount() const
{
    return static_cast<int>(positionUIDs.size());
}

const std::string& CCPACSWingSparPositionUIDs::GetSparPositionUID(int index) const
{
    const int idx = index - 1;
    if (idx < 0 || idx >= GetSparPositionUIDCount()) {
        LOG(ERROR) << "Invalid index value";
        throw CTiglError("Error: Invalid index value in CCPACSWingSparPositionUIDs::GetSparPositionUID", TIGL_INDEX_ERROR);
    }
    return (positionUIDs[idx]);
}

} // end namespace tigl
