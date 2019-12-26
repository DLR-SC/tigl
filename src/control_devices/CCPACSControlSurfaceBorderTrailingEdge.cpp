/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "CControlSurfaceBorderBuilder.h"
#include "CCPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "CCPACSTrailingEdgeDevice.h"

#include "CNamedShape.h"
#include "Debugging.h"
#include <cassert>

namespace tigl
{

CCPACSControlSurfaceBorderTrailingEdge::CCPACSControlSurfaceBorderTrailingEdge(
    CCPACSControlSurfaceOuterShapeTrailingEdge* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceBorderTrailingEdge(parent, uidMgr)
{
}

TopoDS_Wire CCPACSControlSurfaceBorderTrailingEdge::GetWire(PNamedShape wingShape, gp_Vec upDir) const
{
    assert(wingShape);

    // Compute cutout plane
    CTiglControlSurfaceBorderCoordinateSystem coords = GetCoordinateSystem(upDir);
    CControlSurfaceBorderBuilder builder(coords, wingShape->Shape());



    TopoDS_Wire wire;
    if (GetLeadingEdgeShape().is_initialized()) {
        wire = builder.borderWithLEShape(GetLeadingEdgeShape()->GetRelHeightLE(), 1.0,
                                          GetLeadingEdgeShape()->GetXsiUpperSkin(),
                                          GetLeadingEdgeShape()->GetXsiLowerSkin());
    }
    else if (GetInnerShape().is_initialized()) {
        throw CTiglError("Trailing Edge Device with InnerShape not yet implemented");
        /*
        double xsiTEUpper = ...;
        double xsiTELower = ...;
        wire = builder.borderWithInnerShape(GetInnerShape()->GetRelHeightTE(),
                                             GetInnerShape()->GetXsiTE(), xsiTEUpper, xsiTELower);
        */
    }
    else if (GetAirfoil().is_initialized()) {
        wire = GetAirfoilWire(coords);
    }
    else {
        wire = builder.borderSimple(1.0, 1.0);
    }

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(wire, "controldevice-border-wire");
#endif

    return wire;
}

TopoDS_Wire CCPACSControlSurfaceBorderTrailingEdge::GetAirfoilWire(CTiglControlSurfaceBorderCoordinateSystem& coords) const
{
    assert(GetAirfoil().is_initialized());

    CCPACSWingProfile& profile = uidMgr().ResolveObject<CCPACSWingProfile>(GetAirfoil()->GetAirfoilUID());
    TopoDS_Wire w = profile.GetWire();

    // scale
    CTiglTransformation scale;
    scale.AddScaling(coords.getLe().Distance(coords.getTe()), 1, GetAirfoil()->GetScalZ());

    // bring the wire into the coordinate system of
    // the airfoil by swapping z with y
    gp_Trsf trafo;
    trafo.SetTransformation(gp_Ax3(gp_Pnt(0,0,0), gp_Vec(0,-1,0), gp_Vec(1,0,0)));
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

CTiglControlSurfaceBorderCoordinateSystem CCPACSControlSurfaceBorderTrailingEdge::GetCoordinateSystem(gp_Vec upDir) const
{
    const auto& segment = ComponentSegment(*this);
    gp_Pnt pLE = segment.GetPoint(getEtaLE(), getXsiLE());
    gp_Pnt pTE = segment.GetPoint(getEtaTE(), getXsiTE());

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

CCPACSControlSurfaceBorderTrailingEdge::ShapeType CCPACSControlSurfaceBorderTrailingEdge::GetShapeType() const
{
    if (GetAirfoil().is_initialized()) {
        return ShapeType::AIRFOIL;
    }
    else if (GetLeadingEdgeShape().is_initialized()) {
        return ShapeType::LE_SHAPE;
    }
    else {
        return ShapeType::SIMPLE;
    }
}

double CCPACSControlSurfaceBorderTrailingEdge::getEtaTE() const
{
    if (GetEtaTE().is_initialized()) {
        return GetEtaTE()->GetEta();
    }
    else {
        return getEtaLE();
    }
}

double CCPACSControlSurfaceBorderTrailingEdge::getEtaLE() const
{
    return GetEtaLE().GetEta();
}

double CCPACSControlSurfaceBorderTrailingEdge::getXsiLE() const
{
    return GetXsiLE().GetXsi();
}

double CCPACSControlSurfaceBorderTrailingEdge::getXsiTE() const
{
    return 1.0;
}

const CTiglUIDManager &CCPACSControlSurfaceBorderTrailingEdge::uidMgr() const
{
    if (GetParent() && GetParent()->GetParent()) {
        return GetParent()->GetParent()->GetUIDManager();
    }
    else {
        throw CTiglError("Missing parent pointer.");
    }
}

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSControlSurfaceBorderTrailingEdge& self)
{
    if (!self.GetParent() || !self.GetParent()->GetParent()) {
        throw CTiglError("Missing parant Pointer");
    }

    const auto& ted = self.GetParent()->GetParent();

    return ComponentSegment(*ted);
}

} // namespace tigl
