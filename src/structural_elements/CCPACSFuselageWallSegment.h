/* 
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-05-14 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "generated/CPACSWallSegment.h"
#include "CTiglAbstractGeometricComponent.h"
#include <TopoDS_Compound.hxx>

class TopoDS_Shape;

namespace tigl
{

class CCPACSFuselage;
class CCPACSWalls;

namespace generated
{
    class CPACSWallPosition;
}

class CCPACSFuselageWallSegment : public generated::CPACSWallSegment, public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CCPACSFuselageWallSegment(CCPACSWallSegments* parent, CTiglUIDManager* uidMgr);

    std::string GetDefaultedUID() const override
    {
        return GetUID().value_or("UnkownWallSegment");
    }

    TiglGeometricComponentIntent GetComponentIntent() const override
    {
        return TIGL_INTENT_INNER_STRUCTURE | TIGL_INTENT_PHYSICAL;
    }

    TiglGeometricComponentType   GetComponentType() const override
    {
        return TIGL_COMPONENT_FUSELAGE_WALL;
    }

    TopoDS_Compound GetCutPlanes() const;
    
private:
    const CCPACSFuselage& GetFuselage() const;
    
    PNamedShape BuildLoft() const override;
    const CCPACSWalls& GetWalls() const;
    
    mutable TopoDS_Compound m_cutPlanes;
};

} // namespace tigl
