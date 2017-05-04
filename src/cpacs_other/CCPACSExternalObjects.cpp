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

CCPACSExternalObjects::CCPACSExternalObjects(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSGenericGeometryComponents(parent, uidMgr) {}

CCPACSExternalObject&CCPACSExternalObjects::GetObject(int index) const
{
    index --;
    if (index < 0 || index >= GetObjectCount()) {
        throw CTiglError("Error: Invalid index in CCPACSExternalObjects::GetObject", TIGL_INDEX_ERROR);
    }
    return *m_genericGeometryComponents[index];
}

int CCPACSExternalObjects::GetObjectCount() const
{
    return static_cast<int>(m_genericGeometryComponents.size());
}

}

