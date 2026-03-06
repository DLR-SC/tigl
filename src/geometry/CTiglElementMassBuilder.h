/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-03-05 Marko Alder <marko.alder@dlr.de>
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

#include <boost/optional.hpp>
#include "CTiglPoint.h"
#include "CTiglMassInertia.h"
#include "CPACSElementMass.h"

#include "tigl_internal.h"

#include <TopoDS_Shape.hxx>

namespace tigl
{

class CTiglElementMassBuilder
{
public:
    struct Result {
        boost::optional<double> mass                   = boost::none;
        boost::optional<CTiglPoint> cogLocal           = boost::none;
        boost::optional<CTiglMassInertia> inertiaLocal = boost::none;
    };

    TIGL_EXPORT CTiglElementMassBuilder(const CCPACSElementMass& massDef, std::string uid, const TopoDS_Shape& shape);

    TIGL_EXPORT Result EvaluateMass() const;

private:
    const CCPACSElementMass* m_massDef;
    const std::string m_uid;
    const TopoDS_Shape m_shape;
};

} //namespace tigl