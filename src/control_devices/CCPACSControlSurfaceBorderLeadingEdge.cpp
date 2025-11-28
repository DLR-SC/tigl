/*
 * Copyright (C) 2025 German Aerospace Center (DLR/SC)
 *
 * Created: 2025 Ole Albers <ole.albers@dlr.de>
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

#include "CCPACSControlSurfaceBorderLeadingEdge.h"
#include "CCPACSControlSurfaceOuterShapeLeadingEdge.h"
#include "CCPACSLeadingEdgeDevice.h"
#include "CControlSurfaceBorderBuilder.h"
#include "tigletaxsifunctions.h"

#include "CNamedShape.h"
#include "Debugging.h"
#include <cassert>

namespace tigl
{

CCPACSControlSurfaceBorderLeadingEdge::CCPACSControlSurfaceBorderLeadingEdge(
    CCPACSControlSurfaceOuterShapeLeadingEdge* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceBorderLeadingEdge(parent, uidMgr)
{
}

TopoDS_Wire CCPACSControlSurfaceBorderLeadingEdge::GetWire(PNamedShape wingShape, gp_Vec upDir) const
{
    assert(wingShape);

    // Compute cutout plane
    CTiglControlSurfaceBorderCoordinateSystem coords = GetCoordinateSystem(upDir);
    CControlSurfaceBorderBuilder builder(coords, wingShape->Shape());

    TopoDS_Wire wire;
    if (getXsiTE() > 1 - 1e-3) {

        wire = builder.wholeWingBorder();
    }

    else if (GetLeadingEdgeShape_choice2().is_initialized()) {
        throw CTiglError("Leading edge device with LeadingEdgeShape not implemented");
    }

    else if (GetInnerShape_choice1().is_initialized()) {

        double xsiTEUpper;
        double xsiTELower;

        if (GetXsiTE_choice1().is_initialized()) {
            xsiTEUpper = GetXsiTE_choice1()->GetXsi();
            xsiTELower = GetXsiTE_choice1()->GetXsi();
        }
        else {
            if (!GetXsiTEUpper_choice2().is_initialized() || !GetXsiTELower_choice2().is_initialized()) {
                throw CTiglError("LED relative chordwise trailing edge xsiTE and/ or xsiTEUpper and xsiTELower not "
                                 "sepcified in CPACS file.");
            }
            xsiTEUpper = GetXsiTEUpper_choice2().value();
            xsiTELower = GetXsiTELower_choice2().value();
        }

        wire = builder.borderWithInnerShape(GetInnerShape_choice1()->GetRelHeightTE(),
                                            GetInnerShape_choice1()->GetXsiTE(), xsiTEUpper, xsiTELower);
    }

    else if (GetAirfoil_choice3().is_initialized()) {

        wire = GetAirfoilWire(coords);
    }

    else {
        throw CTiglError("Airfoil definition of leading edge device innner/outer border missing.");
    }

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(wire, "controldevice-border-wire");
#endif

    return wire;
}

TopoDS_Wire
CCPACSControlSurfaceBorderLeadingEdge::GetAirfoilWire(CTiglControlSurfaceBorderCoordinateSystem& coords) const
{
    assert(GetAirfoil_choice3().is_initialized());

    CCPACSWingProfile& profile = uidMgr().ResolveObject<CCPACSWingProfile>(GetAirfoil_choice3()->GetAirfoilUID());
    TopoDS_Wire w              = profile.GetWire();

    // scale
    CTiglTransformation scale;
    scale.AddScaling(coords.getLe().Distance(coords.getLe()), 1, GetAirfoil_choice3()->GetScalZ());

    // bring the wire into the coordinate system of
    // the airfoil by swapping z with y
    gp_Trsf trafo;
    trafo.SetTransformation(gp_Ax3(gp_Pnt(0, 0, 0), gp_Vec(0, -1, 0), gp_Vec(1, 0, 0)));
    CTiglTransformation flipZY(trafo);

    // put the airfoil to the correct place
    CTiglTransformation position(coords.globalTransform());

    // compute the total transform
    CTiglTransformation total;
    total.PreMultiply(scale);
    total.PreMultiply(flipZY);
    total.PreMultiply(position);
    w = TopoDS::Wire(total.Transform(w));
    return w;
}

CTiglControlSurfaceBorderCoordinateSystem CCPACSControlSurfaceBorderLeadingEdge::GetCoordinateSystem(gp_Vec upDir) const
{
    const auto& segment = ComponentSegment(*this);
    auto& etaLE         = GetEtaLE();
    auto& etaTE         = GetEtaTE().is_initialized() ? GetEtaTE().value() : GetEtaLE();
    gp_Pnt pLE          = segment.GetPoint(transformEtaToCSOrTed(etaLE, *m_uidMgr), getXsiLE());
    gp_Pnt pTE          = segment.GetPoint(transformEtaToCSOrTed(etaTE, *m_uidMgr), getXsiTE());

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

CCPACSControlSurfaceBorderLeadingEdge::ShapeType CCPACSControlSurfaceBorderLeadingEdge::GetShapeType() const
{
    if (GetAirfoil_choice3().is_initialized()) {
        return ShapeType::AIRFOIL;
    }
    else if (GetLeadingEdgeShape_choice2().is_initialized()) {
        return ShapeType::LE_SHAPE;
    }
    else {
        return ShapeType::SIMPLE;
    }
}

double CCPACSControlSurfaceBorderLeadingEdge::getEtaTE() const
{
    if (GetEtaTE().is_initialized()) {
        return GetEtaTE()->GetEta();
    }
    else {
        return getEtaLE();
    }
}

double CCPACSControlSurfaceBorderLeadingEdge::getEtaLE() const
{
    return GetEtaLE().GetEta();
}

double CCPACSControlSurfaceBorderLeadingEdge::getXsiLE() const
{
    return 0.0;
}

double CCPACSControlSurfaceBorderLeadingEdge::getXsiTE() const
{
    if (GetXsiTE_choice1().is_initialized()) {
        return GetXsiTE_choice1()->GetXsi();
    }
    else {
        return std::max(GetXsiTEUpper_choice2().value(), GetXsiTELower_choice2().value());
    }
}

const CTiglUIDManager& CCPACSControlSurfaceBorderLeadingEdge::uidMgr() const
{
    if (GetParent() && GetParent()->GetParent()) {
        return GetParent()->GetParent()->GetUIDManager();
    }
    else {
        throw CTiglError("Missing parent pointer.");
    }
}

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSControlSurfaceBorderLeadingEdge& self)
{
    if (!self.GetParent() || !self.GetParent()->GetParent()) {
        throw CTiglError("Missing parant Pointer");
    }

    const auto& led = self.GetParent()->GetParent();

    return ComponentSegment(*led);
}

} // namespace tigl
