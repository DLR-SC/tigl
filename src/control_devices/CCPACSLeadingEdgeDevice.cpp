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

#include "CCPACSLeadingEdgeDevice.h"
#include "generated/CPACSControlSurfacePath.h"
#include "generated/CPACSControlSurfaceStep.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CTiglFusePlane.h"
#include "CCPACSControlSurfaceSteps.h"

#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"
#include "tigletaxsifunctions.h"

// TODO: Replace by TiGL function
#include <BRepBuilderAPI_Transform.hxx>

namespace tigl
{

CCPACSLeadingEdgeDevice::CCPACSLeadingEdgeDevice(CCPACSLeadingEdgeDevices* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSLeadingEdgeDevice(parent, uidMgr)
    , CTiglAbstractGeometricComponent()
    , m_hingePoints(*this, &CCPACSLeadingEdgeDevice::ComputeHingePoints)
    , m_cutoutShape(*this, &CCPACSLeadingEdgeDevice::ComputeCutoutShape)
    , m_flapShape(*this, &CCPACSLeadingEdgeDevice::ComputeFlapShape)
    , m_type(LEADING_EDGE_DEVICE)
{
}

void CCPACSLeadingEdgeDevice::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    CPACSLeadingEdgeDevice::ReadCPACS(tixiHandle, xpath);

    m_currentControlParam = Clamp(0., GetMinControlParameter(), GetMaxControlParameter());
}

// get short name for loft
std::string CCPACSLeadingEdgeDevice::GetShortName() const
{
    const CCPACSLeadingEdgeDevices* devices = GetParent();

    std::string tmp = devices->GetParent()->GetParent()->GetShortName();

    size_t idx = IndexFromUid(devices->GetLeadingEdgeDevices(), GetUID());
    if (idx < devices->GetLeadingEdgeDevices().size()) {
        std::stringstream shortName;
        if (m_type == LEADING_EDGE_DEVICE) {
            shortName << tmp << "LED" << idx;
        }
        else if (m_type == TRAILING_EDGE_DEVICE) {
            shortName << tmp << "TED" << idx;
        }
        else if (m_type == SPOILER) {
            shortName << tmp << "SPO" << idx;
        }
        return shortName.str();
    }
    else {
        return "UNKNOWN";
    }
}

gp_Trsf CCPACSLeadingEdgeDevice::GetFlapTransform() const
{
    return CCPACSControlSurfaceSteps::GetTransformation(m_currentControlParam);
}

double CCPACSLeadingEdgeDevice::GetMinControlParameter() const
{
    return GetPath().GetSteps().GetSteps().front()->GetControlParameter();
}

double CCPACSLeadingEdgeDevice::GetMaxControlParameter() const
{
    return GetPath().GetSteps().GetSteps().back()->GetControlParameter();
}

double CCPACSLeadingEdgeDevice::GetControlParameter() const
{
    return m_currentControlParam;
}

void CCPACSLeadingEdgeDevice::SetControlParameter(const double param)
{
    // clamp currentControlParam to minimum and maximum values
    double new_param = Clamp(param, GetMinControlParameter(), GetMaxControlParameter());

    if (fabs(new_param - m_currentControlParam) > 1e-6) {
        m_currentControlParam = new_param;
        // make sure the wing gets relofted with flaps
        Invalidate();
    }
}

void CCPACSLeadingEdgeDevice::Invalidate()
{
    CCPACSWing& wing = Wing();
    wing.SetBuildFlaps(true);
    wing.GetConfiguration().AircraftFusingAlgo()->Invalidate();
}

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSLeadingEdgeDevice& self)
{
    return *self.GetParent()->GetParent()->GetParent();
}

CCPACSWingComponentSegment& ComponentSegment(CCPACSLeadingEdgeDevice& self)
{
    return const_cast<CCPACSWingComponentSegment&>(ComponentSegment(const_cast<const CCPACSLeadingEdgeDevice&>(self)));
}

const CCPACSWing& CCPACSLeadingEdgeDevice::Wing() const
{
    return *ComponentSegment(*this).GetParent()->GetParent();
}

CCPACSWing& CCPACSLeadingEdgeDevice::Wing()
{
    return const_cast<CCPACSWing&>(const_cast<const CCPACSLeadingEdgeDevice*>(this)->Wing());
}

void CCPACSLeadingEdgeDevice::ComputeHingePoints(CCPACSLeadingEdgeDevice::HingePoints& hingePoints) const
{

    // get the Loft geometry of the Component Segment
    const CCPACSWingComponentSegment& cSegment = ComponentSegment(*this);
    CTiglWingStructureReference wsr(cSegment);

    // xsi positions of hinge points
    std::vector<double> hingeXsi = {GetPath().GetInnerHingePoint().GetHingeXsi(),
                                    GetPath().GetOuterHingePoint().GetHingeXsi()};

    // relative hinge heights
    std::vector<double> relHingeHeight = {GetPath().GetInnerHingePoint().GetHingeRelHeight(),
                                          GetPath().GetOuterHingePoint().GetHingeRelHeight()};

    std::vector<gp_Pnt> points(2);

    // Loop over both hinge line points and determine their location
    for (size_t i = 0; i < hingeXsi.size(); i++) {
        // variable part of name for error msg
        std::string innerOuter = (i == 0 ? "inner" : "outer");
        // inner/outer border of trailing edge device
        const CCPACSControlSurfaceBorderLeadingEdge& border =
            (i == 0 ? GetOuterShape().GetInnerBorder() : GetOuterShape().GetOuterBorder());

        if (relHingeHeight[i] < 0. || 1. < relHingeHeight[i]) {
            LOG(ERROR) << "The " << innerOuter << "HingeHeight is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeHeight is not between 0. and 1. in CCPACSLeadingEdgeDevice::buildHingePoints.");
        }
        if (hingeXsi[i] < 0. || 1. < hingeXsi[i]) {
            LOG(ERROR) << "The " << innerOuter << "HingeXsi is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeXsi is not between 0. and 1. in CCPACSLeadingEdgeDevice::buildHingePoints.");
        }

        // create the hinge line point and normal on the Wing component segment mid plane
        gp_Pnt myHingePoint =
            cSegment.GetPoint(transformEtaToCSOrTed(border.GetEtaLE(), *m_uidMgr), hingeXsi[i], WING_COORDINATE_SYSTEM);
        gp_Vec midplaneNormal = cSegment.GetMidplaneNormal(transformEtaToCSOrTed(border.GetEtaLE(), *m_uidMgr));

        // Project point on the inner and outer face of the Wing loft
        gp_Pnt upper, lower;
        upper = ProjectPointOnShape(wsr.GetUpperShape(WING_COORDINATE_SYSTEM), myHingePoint, midplaneNormal);
        lower = ProjectPointOnShape(wsr.GetLowerShape(WING_COORDINATE_SYSTEM), myHingePoint, midplaneNormal);
        // then get delta through distance of these points multiplied by rel hinge height
        gp_Vec delta(lower, upper);
        delta *= relHingeHeight[i];
        lower.Translate(delta);

        // do we have to translate the point?
        auto translation = (i == 0 ? GetPath().GetInnerHingePoint().GetTranslation()
                                   : GetPath().GetOuterHingePoint().GetTranslation());
        if (translation) {
            lower = lower.XYZ() + translation.value().AsPoint().Get_gp_Pnt().XYZ();
        }

        points[i] = lower;
    }

    hingePoints.inner = points[0];
    hingePoints.outer = points[1];
}

void CCPACSLeadingEdgeDevice::ComputeCutoutShape(PNamedShape& shape) const
{
    if (!GetWingCutOut()) {
        shape = GetOuterShape().CutoutShape(ComponentSegment(*this).GetWing().GetWingCleanShape(),
                                            GetNormalOfControlSurfaceDevice());
    }
    else {
        throw CTiglError("Not implemented yet.");
        
        shape = GetWingCutOut()->GetLoft(ComponentSegment(*this).GetWing().GetWingCleanShape(), GetOuterShape(),
                                         GetNormalOfControlSurfaceDevice());
    }
}

void CCPACSLeadingEdgeDevice::ComputeFlapShape(PNamedShape& shape) const
{
    shape = GetOuterShape().GetLoft(Wing().GetWingCleanShape(), GetNormalOfControlSurfaceDevice());
    shape->SetName(GetUID().c_str());
}

gp_Vec CCPACSLeadingEdgeDevice::GetNormalOfControlSurfaceDevice() const
{
    const CCPACSWingComponentSegment& compSeg = ComponentSegment(*this);
    gp_Pnt point1                             = compSeg.GetPoint(0, 0);
    gp_Pnt point2                             = compSeg.GetPoint(0, 1);
    gp_Pnt point3                             = compSeg.GetPoint(1, 0);

    gp_Vec dir1to2 = -(gp_Vec(point1.XYZ()) - gp_Vec(point2.XYZ()));
    gp_Vec dir1to3 = -(gp_Vec(point1.XYZ()) - gp_Vec(point3.XYZ()));

    gp_Vec nvV = dir1to2 ^ dir1to3;
    nvV.Normalize();
    return nvV;
}

TiglControlSurfaceType CCPACSLeadingEdgeDevice::GetType() const
{
    return m_type;
}

std::string CCPACSLeadingEdgeDevice::GetDefaultedUID() const
{
    return GetUID();
}

TiglGeometricComponentType CCPACSLeadingEdgeDevice::GetComponentType() const
{
    return TIGL_COMPONENT_CONTROL_SURFACE_DEVICE;
}

TiglGeometricComponentIntent CCPACSLeadingEdgeDevice::GetComponentIntent() const
{
    return TIGL_INTENT_OUTER_AERO_SURFACE | TIGL_INTENT_PHYSICAL;
}

PNamedShape CCPACSLeadingEdgeDevice::GetCutOutShape(void) const
{
    return *m_cutoutShape;
}

PNamedShape CCPACSLeadingEdgeDevice::GetFlapShape() const
{
    return *m_flapShape;
}

PNamedShape CCPACSLeadingEdgeDevice::GetTransformedFlapShape() const
{
    PNamedShape deviceShape = GetFlapShape()->DeepCopy();
    gp_Trsf T               = GetFlapTransform();
    BRepBuilderAPI_Transform form(deviceShape->Shape(), T);
    deviceShape->SetShape(form.Shape());

    // store the transformation property. Required e.g. for VTK metadata
    gp_GTrsf gT(T);
    CTiglTransformation tiglTrafo(gT);
    unsigned int nFaces = deviceShape->GetFaceCount();
    for (unsigned int iFace = 0; iFace < nFaces; ++iFace) {
        CFaceTraits ft = deviceShape->GetFaceTraits(iFace);
        ft.SetTransformation(tiglTrafo);
        deviceShape->SetFaceTraits(iFace, ft);
    }

    return deviceShape;
}

PNamedShape CCPACSLeadingEdgeDevice::BuildLoft() const
{
    return GetFlapShape();
}

} // namespace tigl
