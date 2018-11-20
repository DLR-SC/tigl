/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include <TopoDS_Shape.hxx>

#include "generated/CPACSPressureBulkheadAssemblyPosition.h"
#include "ITiglGeometricComponent.h"
#include "Cache.h"

namespace tigl
{

class CCPACSPressureBulkheadAssemblyPosition : public generated::CPACSPressureBulkheadAssemblyPosition, public ITiglGeometricComponent
{
public:
    TIGL_EXPORT CCPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT virtual void SetFrameUID(const std::string& value) OVERRIDE;
    TIGL_EXPORT virtual void SetPressureBulkheadElementUID(const std::string& value) OVERRIDE;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;
    TIGL_EXPORT PNamedShape GetLoft() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE;

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT TopoDS_Shape GetGeometry(TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;

private:
    void BuildGeometry(TopoDS_Shape& cache) const;

private:
    Cache<TopoDS_Shape, CCPACSPressureBulkheadAssemblyPosition> m_geometry;
};

} // namespace tigl
