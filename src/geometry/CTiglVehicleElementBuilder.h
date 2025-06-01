/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
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

#include <generated/CPACSVehicleElementBase.h>
#include "tigl_internal.h"

#pragma once

namespace tigl
{
class CTiglVehicleElementBuilder
{
public:
    explicit CTiglVehicleElementBuilder(const CCPACSVehicleElementBase& vehicleElement);
    explicit CTiglVehicleElementBuilder(const CCPACSVehicleElementBase& vehicleElement,
                                        const CTiglTransformation& transformation);

    TIGL_EXPORT operator PNamedShape();

    TIGL_EXPORT PNamedShape BuildShape();

private:
    const CCPACSVehicleElementBase& m_vehicleElement;
    CTiglTransformation const* m_transformation = nullptr;

    TopoDS_Shape BuildCuboidShape(const CCPACSCuboid& p);
    TopoDS_Shape BuildCylinderShape(const CCPACSCylinder& f);
    TopoDS_Shape BuildEllipsoidShape(const CCPACSEllipsoid& e);
};

} //namespace tigl
