/*
 * Copyright (C) 2007-2020 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#include "CCPACSControlSurfaceWingCutOut.h"

#include "CCPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "CCPACSControlSurfaceOuterShapeLeadingEdge.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSLeadingEdgeDevice.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CNamedShape.h"
#include "Debugging.h"
#include "generated/CPACSCutOutControlPoint.h"
#include "CControlSurfaceBorderBuilder.h"
#include "tigletaxsifunctions.h"

#include <gp_Vec.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

namespace tigl
{

CTiglAbstractGeometricComponent const* CCPACSControlSurfaceWingCutOut::GetParentComponent() const
{
    if (IsParent<CCPACSTrailingEdgeDevice>()) {
        return (CCPACSTrailingEdgeDevice*)GetParent<CCPACSTrailingEdgeDevice>()->GetParent();
    }
    if (IsParent<CCPACSLeadingEdgeDevice>()) {
        return (CCPACSLeadingEdgeDevice*)GetParent<CCPACSLeadingEdgeDevice>()->GetParent();
    }
    // if (IsParent<CCPACSSpoiler>())
    // {
    //   return GetParent<CCPACSSpoiler>()->GetParent();
    // }
    throw CTiglError("Unexpected error:");
}

CCPACSControlSurfaceWingCutOut::CCPACSControlSurfaceWingCutOut(CCPACSTrailingEdgeDevice* parent,
                                                               CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceWingCutOut(parent, uidMgr)
{
}

CCPACSControlSurfaceWingCutOut::CCPACSControlSurfaceWingCutOut(CCPACSLeadingEdgeDevice* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceWingCutOut(parent, uidMgr)
{
}

PNamedShape CCPACSControlSurfaceWingCutOut::GetLoft(PNamedShape wingCleanShape,
                                                    const CCPACSControlSurfaceOuterShapeTrailingEdge& outerShape,
                                                    const gp_Vec& upDir) const
{
    DLOG(INFO) << "Building " << GetParentComponent()->GetDefaultedUID() << " wing cutout shape";

    // Get Wires definng the Shape of the more complex CutOutShape.
    TopoDS_Wire innerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::InnerBorder, wingCleanShape,
                                          &outerShape.GetInnerBorder(), upDir);
    TopoDS_Wire outerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::OuterBorder, wingCleanShape,
                                          &outerShape.GetOuterBorder(), upDir);

    // TODO: Replace by own lofting
    // make one shape out of the 2 wires and build connections inbetween.
    BRepOffsetAPI_ThruSections thrusections(true, true);
    thrusections.AddWire(outerWire);
    thrusections.AddWire(innerWire);
    thrusections.Build();

    PNamedShape cutout(new CNamedShape(thrusections.Shape(), GetParentComponent()->GetDefaultedUID().c_str()));

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(cutout->Shape(), GetParentComponent()->GetDefaultedUID() + "_cutout");
#endif

    cutout->SetShortName(GetParent<CCPACSTrailingEdgeDevice>()->GetShortName());

    return cutout;
}

PNamedShape CCPACSControlSurfaceWingCutOut::GetLoft(PNamedShape wingCleanShape,
                                                    const CCPACSControlSurfaceOuterShapeLeadingEdge& outerShape,
                                                    const gp_Vec& upDir) const
{
    DLOG(INFO) << "Building " << GetParentComponent()->GetDefaultedUID() << " wing cutout shape";

    // Get Wires definng the Shape of the more complex CutOutShape.
    TopoDS_Wire innerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::InnerBorder, wingCleanShape,
                                          &outerShape.GetInnerBorder(), upDir);
    TopoDS_Wire outerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::OuterBorder, wingCleanShape,
                                          &outerShape.GetOuterBorder(), upDir);

    // TODO: Replace by own lofting
    // make one shape out of the 2 wires and build connections inbetween.
    BRepOffsetAPI_ThruSections thrusections(true, true);
    thrusections.AddWire(outerWire);
    thrusections.AddWire(innerWire);
    thrusections.Build();

    PNamedShape cutout(new CNamedShape(thrusections.Shape(), GetParentComponent()->GetDefaultedUID().c_str()));

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(cutout->Shape(), GetParentComponent()->GetDefaultedUID() + "_cutout");
#endif

    cutout->SetShortName(GetParent<CCPACSLeadingEdgeDevice>()->GetShortName());

    return cutout;
}

TopoDS_Wire CCPACSControlSurfaceWingCutOut::GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition pos,
                                                          PNamedShape wingCleanShape,
                                                          const CCPACSControlSurfaceBorderTrailingEdge* outerBorder,
                                                          gp_Vec upDir) const
{
    assert(wingCleanShape);

    TopoDS_Wire wire;

    CTiglControlSurfaceBorderCoordinateSystem coords(
        GetCutoutCS(pos == CutoutPosition::InnerBorder, outerBorder, upDir));
    CControlSurfaceBorderBuilder builder(coords, wingCleanShape->Shape());

    double xsiUpper, xsiLower;
    if (pos == CutoutPosition::InnerBorder) {
        if (!GetUpperSkin().GetXsiInnerBorder_choice2() || !GetLowerSkin().GetXsiInnerBorder_choice2()) {
            throw CTiglError("Inner Border xsi values must be set for upper and lower skin");
        }

        xsiUpper = GetUpperSkin().GetXsiInnerBorder_choice2().value();
        xsiLower = GetLowerSkin().GetXsiInnerBorder_choice2().value();
    }
    else {
        if (!GetUpperSkin().GetXsiOuterBorder_choice2() || !GetLowerSkin().GetXsiOuterBorder_choice2()) {
            throw CTiglError("Outer Border xsi values must be set for upper and lower skin");
        }

        xsiUpper = GetUpperSkin().GetXsiOuterBorder_choice2().value();
        xsiLower = GetLowerSkin().GetXsiOuterBorder_choice2().value();
    }

    // TODO: calculate xsis into coordinate system of the border

    const auto& lePoints = GetCutOutProfileControlPoint();
    if (lePoints) {
        const tigl::generated::CPACSCutOutControlPoint& lePoint =
            pos == CutoutPosition::InnerBorder ? lePoints->GetInnerBorder() : lePoints->GetOuterBorder();

        double xsiNose = lePoint.GetXsi();
        // TODO: calculate xsiNose into coordinate system of the border

        wire = builder.borderWithLEShape(lePoint.GetRelHeight(), xsiNose, xsiUpper, xsiLower);
    }
    else {
        wire = builder.borderSimple(xsiUpper, xsiLower);
    }

    return wire;
}

TopoDS_Wire CCPACSControlSurfaceWingCutOut::GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition pos,
                                                          PNamedShape wingCleanShape,
                                                          const CCPACSControlSurfaceBorderLeadingEdge* outerBorder,
                                                          gp_Vec upDir) const
{
    //TODO
    assert(wingCleanShape);

    TopoDS_Wire wire;

    CTiglControlSurfaceBorderCoordinateSystem coords(
        GetCutoutCS(pos == CutoutPosition::InnerBorder, outerBorder, upDir));
    CControlSurfaceBorderBuilder builder(coords, wingCleanShape->Shape());

    double xsiTEUpper;
    double xsiTELower;

    if (outerBorder->GetXsiTE_choice1().is_initialized()) {
        xsiTEUpper = outerBorder->GetXsiTE_choice1()->GetXsi();
        xsiTELower = outerBorder->GetXsiTE_choice1()->GetXsi();
    }
    else {
        if (!outerBorder->GetXsiTEUpper_choice2().is_initialized() ||
            !outerBorder->GetXsiTELower_choice2().is_initialized()) {
            throw CTiglError("LED relative chordwise trailing edge xsiTE and/ or xsiTEUpper and xsiTELower not "
                             "sepcified in CPACS file.");
        }
        xsiTEUpper = outerBorder->GetXsiTEUpper_choice2().value();
        xsiTELower = outerBorder->GetXsiTELower_choice2().value();
    }

    // TODO: calculate xsis into coordinate system of the border

    const auto& lePoints = GetCutOutProfileControlPoint();
    if (lePoints) {
        const tigl::generated::CPACSCutOutControlPoint& lePoint =
            pos == CutoutPosition::InnerBorder ? lePoints->GetInnerBorder() : lePoints->GetOuterBorder();

        double xsiNose = lePoint.GetXsi();
        // TODO: calculate xsiNose into coordinate system of the border

        wire = builder.borderWithInnerShape(lePoint.GetRelHeight(), xsiNose, xsiTEUpper, xsiTELower);
    }
    else {
        wire = builder.borderSimple(xsiTEUpper, xsiTELower);
    }

    return wire;
}

CTiglControlSurfaceBorderCoordinateSystem CCPACSControlSurfaceWingCutOut::GetCutoutCS(
    bool isInnerBorder, const CCPACSControlSurfaceBorderTrailingEdge* outerShapeBorder, const gp_Vec& upDir) const
{
    const auto& cutOutBorder = isInnerBorder ? GetInnerBorder() : GetOuterBorder();

    if (!cutOutBorder) {
        return outerShapeBorder->GetCoordinateSystem(upDir);
    }

    if (!cutOutBorder->GetEtaLE_choice2() || !cutOutBorder->GetEtaTE_choice2()) {
        throw CTiglError("Cutout border of '" + GetParentComponent()->GetDefaultedUID() +
                         "' requires etaLE and etaTE values to proceed.");
    }

    double lEta = transformEtaToCSOrTed(cutOutBorder->GetEtaLE_choice2().value(), *m_uidMgr);
    double lXsi = transformXsiToCSOrTed(outerShapeBorder->GetXsiLE(), *m_uidMgr);
    double tEta = transformEtaToCSOrTed(cutOutBorder->GetEtaTE_choice2().value(), *m_uidMgr);
    double tXsi = outerShapeBorder->getXsiTE(); // this is always 1.0

    const auto& segment = ComponentSegment(*this);
    gp_Pnt pLE          = segment.GetPoint(lEta, lXsi);
    gp_Pnt pTE          = segment.GetPoint(tEta, tXsi);

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

CTiglControlSurfaceBorderCoordinateSystem CCPACSControlSurfaceWingCutOut::GetCutoutCS(
    bool isInnerBorder, const CCPACSControlSurfaceBorderLeadingEdge* outerShapeBorder, const gp_Vec& upDir) const
{
    const auto& cutOutBorder = isInnerBorder ? GetInnerBorder() : GetOuterBorder();

    if (!cutOutBorder) {
        return outerShapeBorder->GetCoordinateSystem(upDir);
    }

    if (!cutOutBorder->GetEtaLE_choice2() || !cutOutBorder->GetEtaTE_choice2()) {
        throw CTiglError("Cutout border of '" + GetParentComponent()->GetDefaultedUID() +
                         "' requires etaLE and etaTE values to proceed.");
    }

    double lEta = transformEtaToCSOrTed(cutOutBorder->GetEtaLE_choice2().value(), *m_uidMgr);
    double tXsi = outerShapeBorder->GetXsiTE_choice1()->GetXsi();
    double tEta = transformEtaToCSOrTed(cutOutBorder->GetEtaTE_choice2().value(), *m_uidMgr);
    double lXsi = outerShapeBorder->getXsiLE(); // this is always 0.0

    const auto& segment = ComponentSegment(*this);
    gp_Pnt pLE          = segment.GetPoint(lEta, lXsi);
    gp_Pnt pTE          = segment.GetPoint(tEta, tXsi);

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSControlSurfaceWingCutOut& self)
{
    if (self.IsParent<CCPACSTrailingEdgeDevice>()) {
        return ComponentSegment(*self.GetParent<CCPACSTrailingEdgeDevice>());
    }
    if (self.IsParent<CCPACSLeadingEdgeDevice>()) {
        return ComponentSegment(*self.GetParent<CCPACSLeadingEdgeDevice>());
    }

    throw CTiglError("Missing parent Pointer");
}

} // namespace tigl
