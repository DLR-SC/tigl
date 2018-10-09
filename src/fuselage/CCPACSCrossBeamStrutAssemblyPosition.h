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

#include "generated/CPACSCrossBeamStrutAssemblyPosition.h"
#include "ITiglGeometricComponent.h"
#include "Cache.h"

namespace tigl
{
class CCPACSCrossBeamStrutAssemblyPosition : public generated::CPACSCrossBeamStrutAssemblyPosition, public ITiglGeometricComponent
{
public:
    TIGL_EXPORT CCPACSCrossBeamStrutAssemblyPosition(CCPACSCargoCrossBeamStrutsAssembly* parent,
                                                     CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;
    TIGL_EXPORT PNamedShape GetLoft() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT TopoDS_Shape GetGeometry(bool just1DElements, TiglCoordinateSystem cs = GLOBAL_COORDINATE_SYSTEM) const;

private:
    void BuildGeometry1D(TopoDS_Shape& cache) const;
    void BuildGeometry3D(TopoDS_Shape& cache) const;
    void BuildGeometry(TopoDS_Shape& cache, bool just1DElements) const;

private:
    Cache<TopoDS_Shape, CCPACSCrossBeamStrutAssemblyPosition> m_geometry1D;
    Cache<TopoDS_Shape, CCPACSCrossBeamStrutAssemblyPosition> m_geometry3D;
};

} // namespace tigl
