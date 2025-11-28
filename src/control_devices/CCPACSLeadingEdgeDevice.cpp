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
#include "ControlSurfaceDeviceHelper.h"
#include "generated/CPACSControlSurfaceHingePoint.h"

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
        return shortName.str();
    }
    else {
        return "UNKNOWN";
    }
}

gp_Trsf CCPACSLeadingEdgeDevice::GetFlapTransform() const
{
    auto wingTrafo       = Wing().GetTransformation().getTransformationMatrix();
    gp_Pnt innerHingeOld = wingTrafo.Transform(m_hingePoints->inner);
    gp_Pnt outerHingeOld = wingTrafo.Transform(m_hingePoints->outer);
    return GetPath().GetSteps().GetTransformation(m_currentControlParam, innerHingeOld, outerHingeOld);
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
    ControlSurfaceDeviceHelper helper;
   
    hingePoints.inner = helper.calc_hinge_point(
        GetPath().GetInnerHingePoint(), 
        GetOuterShape().GetInnerBorder().GetEtaLE(),
        ComponentSegment(*this),
        m_uidMgr,
        "inner"
    );
    hingePoints.outer = helper.calc_hinge_point(
        GetPath().GetOuterHingePoint(),  
        GetOuterShape().GetOuterBorder().GetEtaLE(),
        ComponentSegment(*this),
        m_uidMgr,
        "outer"
    );

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
    ControlSurfaceDeviceHelper helper;
    return helper.GetNormalOfControlSurfaceDevice_helper(ComponentSegment(*this));
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
    ControlSurfaceDeviceHelper helper;
    return helper.GetTransformedFlapShape_helper(GetFlapShape()->DeepCopy(), GetFlapTransform());
}

PNamedShape CCPACSLeadingEdgeDevice::BuildLoft() const
{
    return GetFlapShape();
}

} // namespace tigl
