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

template <typename T>
class CTiglShapeGeomComponentAdaptor : public ITiglGeometricComponent
{
public:
    typedef PNamedShape(T::* GetShapeFunc)() const;

    CTiglShapeGeomComponentAdaptor(const T* parent, GetShapeFunc getShapeFunc, CTiglUIDManager* uidMgr)
        : m_parent(parent)
        , m_getShapeFunc(getShapeFunc)
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
    }

    void SetUID(const std::string &uid)
    {
        unregisterShape();
        m_uid = uid;
        if (!m_uid.empty() && m_uidMgr) {
            m_uidMgr->RegisterObject(m_uid, *this);
        }
    }

    // Returns the unique id of this component or an empty string if the component does not have a uid
    std::string GetDefaultedUID() const OVERRIDE
    {
        return m_uid;
    }

    PNamedShape GetLoft() const OVERRIDE
    {
        if (m_parent) {
            return (m_parent->*m_getShapeFunc)();
        }
        // support for testing of class without parent
        return PNamedShape();
    }

    TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_OTHER;
    }

    TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE
    {
        return TIGL_INTENT_PHYSICAL;
    }

private:
    void unregisterShape()
    {
        if (!m_uid.empty() && m_uidMgr) {
            m_uidMgr->TryUnregisterObject(m_uid);
        }
    }

    const T* m_parent;
    GetShapeFunc m_getShapeFunc;
    std::string m_uid;
    CTiglUIDManager* m_uidMgr;
};

} // namespace tigl
