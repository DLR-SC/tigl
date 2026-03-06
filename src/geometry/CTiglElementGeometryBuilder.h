/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-25 Marko Alder <marko.alder@dlr.de>
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

#include <generated/CPACSElementGeometry.h>
#include "tigl_internal.h"

#include "CPACSCone.h"
#include "CPACSCuboid.h"
#include "CPACSCylinder.h"
#include "CPACSEllipsoid.h"
#include "CPACSExternalGeometry.h"
#include "CPACSMultiSegmentShape.h"

namespace tigl
{

class CTiglElementGeometryBuilder
{
public:
    TIGL_EXPORT CTiglElementGeometryBuilder(const CTiglRelativelyPositionedComponent& refComponent,
                                            const CCPACSConfiguration& refConfig, const CCPACSElementGeometry& geometry,
                                            const std::string& shapeName, const std::string& cpacsDocumentPath);

    TIGL_EXPORT PNamedShape BuildShape();

private:
    const CCPACSElementGeometry* m_geometry                  = nullptr;
    const CTiglRelativelyPositionedComponent* m_refComponent = nullptr;
    const CCPACSConfiguration* m_refConfig                   = nullptr;

    std::string m_shapeName;
    std::string m_cpacsDocumentPath;

    TopoDS_Shape BuildCuboidShape(const CCPACSCuboid& c);
    TopoDS_Shape BuildCylinderShape(const CCPACSCylinder& c);
    TopoDS_Shape BuildConeShape(const CCPACSCone& c);
    TopoDS_Shape BuildEllipsoidShape(const CCPACSEllipsoid& e);
    TopoDS_Shape BuildMultiSegmentShape(const CCPACSMultiSegmentShape& m);
    TopoDS_Shape BuildExternalShape(const CCPACSExternalGeometry& e);
};

} //namespace tigl
