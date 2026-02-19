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
#include <generated/CPACSSubElement.h>
#include "tigl_internal.h"

namespace tigl
{

class CTiglVehicleElementBuilder
{
public:
    explicit CTiglVehicleElementBuilder(const CTiglRelativelyPositionedComponent& refComponent,
                                        const CCPACSConfiguration& refConfig,
                                        const CCPACSElementGeometry& geometry, const std::string& shapeName,
                                        const std::string& cpacsDocumentPath);

    TIGL_EXPORT PNamedShape BuildShape();

private:
    template <typename TGeom> TopoDS_Shape BuildSingleShapeImpl(const TGeom& geom)
    {
        if (const auto& p = geom.GetCuboid_choice1())
            return BuildCuboidShape(*p);
        else if (const auto& c = geom.GetCylinder_choice2())
            return BuildCylinderShape(*c);
        else if (const auto& c = geom.GetCone_choice3())
            return BuildConeShape(*c);
        else if (const auto& e = geom.GetEllipsoid_choice4())
            return BuildEllipsoidShape(*e);
        else if (const auto& m = geom.GetMultiSegmentShape_choice5())
            return BuildMultiSegmentShape(*m);
        else if (const auto& e = geom.GetExternal_choice6())
            return BuildExternalShape(*e);

        std::string uid = "unknown";
        if (const auto* parent = geom.GetNextUIDParent()) {
            uid = parent->GetObjectUID().get_value_or(uid);
        }
        throw CTiglError("Unsupported geometry for uID=\"" + uid + "\"");
    }

    const CCPACSElementGeometry* m_geometry                  = nullptr;
    const CTiglTransformation* m_transformation              = nullptr;
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

    TopoDS_Shape BuildSingleShape(const CCPACSElementGeometry& geom);
    TopoDS_Shape BuildSingleShape(const CCPACSSubElement& geom);
};

} //namespace tigl
