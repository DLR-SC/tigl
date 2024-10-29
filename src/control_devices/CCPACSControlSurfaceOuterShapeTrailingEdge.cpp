/*
 * Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#include "CCPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "generated/CPACSTrailingEdgeDevices.h"
#include "CNamedShape.h"
#include "CTiglLogging.h"
#include "CBopCommon.h"
#include "Debugging.h"

#include <BRepOffsetAPI_ThruSections.hxx>

namespace tigl
{

CCPACSControlSurfaceOuterShapeTrailingEdge::CCPACSControlSurfaceOuterShapeTrailingEdge(CCPACSTrailingEdgeDevice* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSControlSurfaceOuterShapeTrailingEdge(parent, uidMgr)
{
}

PNamedShape CCPACSControlSurfaceOuterShapeTrailingEdge::GetLoft(PNamedShape wingCleanShape, gp_Vec upDir) const
{
    DLOG(INFO) << "Building " << _uid << " loft";
    PNamedShape shapeBox = CutoutShape(wingCleanShape, upDir);
    assert(shapeBox);
    if (NeedsWingIntersection()) {

        // perform the boolean intersection of the flap box with the wing
        PNamedShape outerShape = CBopCommon(wingCleanShape, shapeBox);

        for (int iFace = 0; iFace < static_cast<int>(outerShape->GetFaceCount()); ++iFace) {
            CFaceTraits ft = outerShape->GetFaceTraits(iFace);
            ft.SetOrigin(shapeBox);
            outerShape->SetFaceTraits(iFace, ft);
        }

#ifdef DEBUG
        DEBUG_SCOPE(debug);
        debug.dumpShape(outerShape->Shape(), _uid);
#endif

        return outerShape;
    }
    else {
        return shapeBox;
    }
}

PNamedShape CCPACSControlSurfaceOuterShapeTrailingEdge::CutoutShape(PNamedShape wingShape, gp_Vec upDir) const
{

    DLOG(INFO) << "Building " << _uid << " cutter shape";

    // Get Wires definng the Shape of the more complex CutOutShape.
    TopoDS_Wire innerWire = GetInnerBorder().GetWire(wingShape, upDir);
    TopoDS_Wire outerWire = GetOuterBorder().GetWire(wingShape, upDir);

    // make one shape out of the 2 wires and build connections inbetween.
    // TODO: Replace by own lofting algo
    BRepOffsetAPI_ThruSections thrusections(true,true);
    thrusections.AddWire(outerWire);
    thrusections.AddWire(innerWire);
    thrusections.Build();

    PNamedShape cutterShape = PNamedShape(new CNamedShape(thrusections.Shape(), GetParent()->GetUID().c_str()));
    cutterShape->SetShortName(GetParent()->GetShortName().c_str());

    assert(cutterShape);

#ifdef DEBUG
    DEBUG_SCOPE(debug);
    debug.dumpShape(cutterShape->Shape(), _uid + "_cutter");
#endif

    return cutterShape;
}

void CCPACSControlSurfaceOuterShapeTrailingEdge::SetUID(const std::string& uid)
{
    _uid = uid;
    //GetOuterBorder().SetUID(uid + "_OuterBorder");
    //GetInnerBorder().SetUID(uid + "_InnerBorder");
}

bool CCPACSControlSurfaceOuterShapeTrailingEdge::NeedsWingIntersection() const
{
    if (GetInnerBorder().GetShapeType() == CCPACSControlSurfaceBorderTrailingEdge::ShapeType::AIRFOIL) {
        return false;
    }
    else {
        return true;
    }
}



} // namespace tigl
