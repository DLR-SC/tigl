/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-05-30 Martin Siggel <martin.siggel@dlr.de>
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

#pragma once

#include "tigl_internal.h"

#include "ITiglGeometricComponent.h"
#include "PNamedShape.h"
#include "CTiglUIDManager.h"

namespace tigl
{

class CTiglUIDManager;

class CTiglShapeGeomComponentAdaptor : public ITiglGeometricComponent
{
public:
    CTiglShapeGeomComponentAdaptor(ITiglGeometricComponent* parent, CTiglUIDManager* uidMgr)
        : m_parent(parent)
        , m_uid("")
        , m_uidMgr(uidMgr)
    {
    }

    ~CTiglShapeGeomComponentAdaptor()
    {
        unregisterShape();
    }

    void Reset()
    {
        unregisterShape();
        m_uid = "";
        m_shape.reset();
    }

    void SetUID(const std::string &uid)
    {
        unregisterShape();

        m_uid = uid;

        if (!m_uid.empty() && m_uidMgr) {
            m_uidMgr->AddGeometricComponent(GetDefaultedUID(), this);
        }
    }


    void SetShape(PNamedShape shape)
    {

        m_shape = shape;
    }

    // Returns the unique id of this component or an empty string if the component does not have a uid
    std::string GetDefaultedUID() const OVERRIDE
    {
        return m_uid;
    }

    PNamedShape GetLoft() OVERRIDE
    {
        if (m_parent) {
            // The shape has to be build somewhere
            // This is the parent's tasl
            m_parent->GetLoft();
        }

        return m_shape;
    }

    TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_PHYSICAL;
    }

private:
    void unregisterShape()
    {
        if (!m_uid.empty() && m_uidMgr && m_uidMgr->HasGeometricComponent(GetDefaultedUID())) {
            m_uidMgr->RemoveGeometricComponent(GetDefaultedUID());
        }
    }

    ITiglGeometricComponent* m_parent;
    PNamedShape m_shape;
    std::string m_uid;
    CTiglUIDManager* m_uidMgr;

};

} // namespace tigl
