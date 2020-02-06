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
#include "CCPACSTrailingEdgeDevice.h"
#include "CNamedShape.h"
#include "Debugging.h"
#include "generated/CPACSCutOutControlPoint.h"
#include "CControlSurfaceBorderBuilder.h"

#include <gp_Vec.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

namespace tigl
{

CCPACSControlSurfaceWingCutOut::CCPACSControlSurfaceWingCutOut(CCPACSTrailingEdgeDevice* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceWingCutOut(parent, uidMgr)
{
}

PNamedShape CCPACSControlSurfaceWingCutOut::GetLoft(PNamedShape wingCleanShape, const CCPACSControlSurfaceOuterShapeTrailingEdge& outerShape, const gp_Vec &upDir) const
{
    DLOG(INFO) << "Building " << GetParent()->GetUID() << " wing cutout shape";

    // Get Wires definng the Shape of the more complex CutOutShape.
    TopoDS_Wire innerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::InnerBorder, wingCleanShape, &outerShape.GetInnerBorder(), upDir);
    TopoDS_Wire outerWire = GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition::OuterBorder, wingCleanShape, &outerShape.GetOuterBorder(), upDir);

    // TODO: Replace by own lofting
    // make one shape out of the 2 wires and build connections inbetween.
    BRepOffsetAPI_ThruSections thrusections(true,true);
    thrusections.AddWire(outerWire);
    thrusections.AddWire(innerWire);
    thrusections.Build();

    PNamedShape cutout(new CNamedShape(thrusections.Shape(), GetParent()->GetUID().c_str()));

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(_loft->Shape(), GetParent()->GetUID() + "_cutout");
#endif

    cutout->SetShortName(GetParent()->GetShortName());

    return cutout;
}

TopoDS_Wire CCPACSControlSurfaceWingCutOut::GetCutoutWire(CCPACSControlSurfaceWingCutOut::CutoutPosition pos,
                                                          PNamedShape wingCleanShape,
                                                          const CCPACSControlSurfaceBorderTrailingEdge* outerBorder,
                                                          gp_Vec upDir) const
{
    assert(wingCleanShape);

    TopoDS_Wire wire;

    CTiglControlSurfaceBorderCoordinateSystem coords(GetCutoutCS(pos == CutoutPosition::InnerBorder, outerBorder, upDir));
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
        const tigl::generated::CPACSCutOutControlPoint& lePoint = pos == CutoutPosition::InnerBorder ? lePoints->GetInnerBorder() : lePoints->GetOuterBorder();

        double xsiNose = lePoint.GetXsi();
        // TODO: calculate xsiNose into coordinate system of the border

        wire = builder.borderWithLEShape(lePoint.GetRelHeight(), xsiNose, xsiUpper, xsiLower);
    }
    else {
        wire = builder.borderSimple(xsiUpper, xsiLower);
    }

    return wire;
}

CTiglControlSurfaceBorderCoordinateSystem
CCPACSControlSurfaceWingCutOut::GetCutoutCS(bool isInnerBorder, const CCPACSControlSurfaceBorderTrailingEdge* outerShapeBorder, const gp_Vec &upDir) const
{
    const auto& cutOutBorder =
            isInnerBorder ? GetInnerBorder() : GetOuterBorder();

    if (!cutOutBorder) {
        return outerShapeBorder->GetCoordinateSystem(upDir);
    }

    if (!cutOutBorder->GetEtaLE_choice2() || !cutOutBorder->GetEtaTE_choice2()) {
        throw CTiglError("Cutout border of '" + GetParent()->GetUID() + "' requires etaLE and etaTE values to proceed.");
    }

    double lEta = cutOutBorder->GetEtaLE_choice2().value().GetEta();
    double lXsi = outerShapeBorder->getXsiLE();
    double tEta = cutOutBorder->GetEtaTE_choice2().value().GetEta();
    double tXsi = outerShapeBorder->getXsiTE();

    const auto& segment = ComponentSegment(*this);
    gp_Pnt pLE = segment.GetPoint(lEta, lXsi);
    gp_Pnt pTE = segment.GetPoint(tEta, tXsi);

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

const CCPACSWingComponentSegment &ComponentSegment(const CCPACSControlSurfaceWingCutOut& self)
{
    if (!self.GetParent()) {
        throw CTiglError("Missing parant Pointer");
    }

    const auto& ted = self.GetParent();

    return ComponentSegment(*ted);
}

} // namespace tigl
