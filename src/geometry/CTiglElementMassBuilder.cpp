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

#include "CTiglElementMassBuilder.h"
#include "CTiglError.h"

#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <gp_Pnt.hxx>

#include <utility>

namespace tigl
{

CTiglElementMassBuilder::CTiglElementMassBuilder(const CCPACSElementMass& massDef, std::string uid,
                                                 const TopoDS_Shape& shape)
    : m_massDef(&massDef)
    , m_uid(std::move(uid))
    , m_shape(shape)
{
}

CTiglElementMassBuilder::Result CTiglElementMassBuilder::EvaluateMass() const
{
    Result result;

    // read inertia values from CPACS
    if (const auto& mi = m_massDef->GetMassInertia(); mi) {
        CTiglMassInertia out;
        out.Jxx             = mi->GetJxx();
        out.Jyy             = mi->GetJyy();
        out.Jzz             = mi->GetJzz();
        out.Jxy             = mi->GetJxy();
        out.Jxz             = mi->GetJxz();
        out.Jyz             = mi->GetJyz();
        result.inertiaLocal = out;
    }

    boost::optional<GProp_GProps> props;

    auto ensureProps = [&]() {
        if (props) {
            return;
        }

        GProp_GProps p;
        BRepGProp::VolumeProperties(m_shape, p);

        if (p.Mass() <= 0.0) {
            throw CTiglError("Cannot compute mass properties of component with uID \"" + m_uid + "\" (zero volume).");
        }

        props = p;
    };

    // CoG
    if (const auto& loc = m_massDef->GetLocation(); loc) {
        result.cogLocal = loc->AsPoint();
    }
    else {
        ensureProps();
        const gp_Pnt c  = props->CentreOfMass();
        result.cogLocal = CTiglPoint(c.X(), c.Y(), c.Z());
    }

    // Mass
    if (const auto& m = m_massDef->GetMass_choice2(); m) {
        result.mass = *m;
    }
    else if (const auto& rho = m_massDef->GetDensity_choice1(); rho) {
        ensureProps();
        result.mass = (*rho) * props->Mass();
    }
    else {
        throw CTiglError("Invalid mass definition (no mass and no density) for uID \"" + m_uid + "\".");
    }

    return result;
}

} // namespace tigl