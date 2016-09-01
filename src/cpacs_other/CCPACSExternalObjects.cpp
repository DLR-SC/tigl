/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-19 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSExternalObjects.h"

#include "CCPACSExternalObject.h"
#include "CTiglError.h"

#define CPACS_EXTERNAL_COMPONENTS_NODE "genericGeometryComponents"
#define CPACS_EXTERNAL_COMPONENT_NODE "genericGeometryComponent"

namespace tigl
{

CCPACSExternalObjects::CCPACSExternalObjects(CCPACSConfiguration* config)
    : _config(config)
{
}

CCPACSExternalObjects::~CCPACSExternalObjects()
{
    Cleanup();
}

void CCPACSExternalObjects::ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID)
{
    Cleanup();
    char *tmpString = NULL;

    if (tixiUIDGetXPath(tixiHandle, configurationUID, &tmpString) != SUCCESS) {
        throw CTiglError("XML error: tixiUIDGetXPath failed in CCPACSExternalObjects::ReadCPACS", TIGL_XML_ERROR);
    }

    std::string componentXPath= tmpString;
    componentXPath += "[@uID=\"";
    componentXPath += configurationUID;
    componentXPath += "\"]/";
    componentXPath += CPACS_EXTERNAL_COMPONENTS_NODE;

    if (tixiCheckElement(tixiHandle, componentXPath.c_str()) != SUCCESS) {
        return;
    }

    /* Get external object count */
    int objectCount;
    if (tixiGetNamedChildrenCount(tixiHandle, componentXPath.c_str(), CPACS_EXTERNAL_COMPONENT_NODE, &objectCount) != SUCCESS) {
        throw CTiglError("XML error: tixiGetNamedChildrenCount failed in CCPACSExternalObjects::ReadCPACS", TIGL_XML_ERROR);
    }

    // Loop over all component
    for (int i = 1; i <= objectCount; i++) {
        CCPACSExternalObject* obj = new CCPACSExternalObject(_config);
        _objects.push_back(obj);

        std::ostringstream xpath;
        xpath << componentXPath << "/" + std::string(CPACS_EXTERNAL_COMPONENT_NODE) + "[" << i << "]";
        obj->ReadCPACS(tixiHandle, xpath.str());
    }
}

CCPACSExternalObject&CCPACSExternalObjects::GetObject(int index) const
{
    index --;
    if (index < 0 || index >= GetObjectCount()) {
        throw CTiglError("Error: Invalid index in CCPACSExternalObjects::GetObject", TIGL_INDEX_ERROR);
    }
    return (*_objects[index]);
}

int CCPACSExternalObjects::GetObjectCount() const
{
    return _objects.size();
}

    // Cleanup routine
void CCPACSExternalObjects::Cleanup()
{
    for (CCPACSExtObjectContainer::size_type i = 0; i < _objects.size(); i++) {
        delete _objects[i];
    }
    _objects.clear();
}

}

