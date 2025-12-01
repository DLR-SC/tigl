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

#include "CCPACSTrailingEdgeDevice.h"
#include "generated/CPACSControlSurfacePath.h"
#include "generated/CPACSControlSurfaceStep.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CTiglFusePlane.h"
#include "ControlSurfaceDeviceHelper.h"
#include "generated/CPACSControlSurfaceHingePoint.h"

#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"
#include "tigletaxsifunctions.h"

// TODO: Replace by TiGL function
#include <BRepBuilderAPI_Transform.hxx>

namespace tigl
{

CCPACSTrailingEdgeDevice::CCPACSTrailingEdgeDevice(CCPACSTrailingEdgeDevices* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTrailingEdgeDevice(parent, uidMgr)
    , CTiglAbstractGeometricComponent()
    , m_hingePoints(*this, &CCPACSTrailingEdgeDevice::ComputeHingePoints)
    , m_cutoutShape(*this, &CCPACSTrailingEdgeDevice::ComputeCutoutShape)
    , m_flapShape(*this, &CCPACSTrailingEdgeDevice::ComputeFlapShape)
    , m_type(TRAILING_EDGE_DEVICE)
{
}

void CCPACSTrailingEdgeDevice::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    CPACSTrailingEdgeDevice::ReadCPACS(tixiHandle, xpath);

    m_currentControlParam = Clamp(0., GetMinControlParameter(), GetMaxControlParameter());
}

// get short name for loft
std::string CCPACSTrailingEdgeDevice::GetShortName() const
{
    const CCPACSTrailingEdgeDevices* devices = GetParent();

    std::string tmp = devices->GetParent()->GetParent()->GetShortName();

    size_t idx = IndexFromUid(devices->GetTrailingEdgeDevices(), GetUID());
    if (idx < devices->GetTrailingEdgeDevices().size()) {
        std::stringstream shortName;
        if (m_type == TRAILING_EDGE_DEVICE) {
            shortName << tmp << "TED" << idx;
        }
        return shortName.str();
    }
    else {
        return "UNKNOWN";
    }
}

gp_Trsf CCPACSTrailingEdgeDevice::GetFlapTransform() const
{
    auto wingTrafo       = Wing().GetTransformation().getTransformationMatrix();
    gp_Pnt innerHingeOld = wingTrafo.Transform(m_hingePoints->inner);
    gp_Pnt outerHingeOld = wingTrafo.Transform(m_hingePoints->outer);
    return GetPath().GetSteps().GetTransformation(m_currentControlParam, innerHingeOld, outerHingeOld);
}

double CCPACSTrailingEdgeDevice::GetMinControlParameter() const
{
    return GetPath().GetSteps().GetSteps().front()->GetControlParameter();
}

double CCPACSTrailingEdgeDevice::GetMaxControlParameter() const
{
    return GetPath().GetSteps().GetSteps().back()->GetControlParameter();
}

double CCPACSTrailingEdgeDevice::GetControlParameter() const
{
    return m_currentControlParam;
}

void CCPACSTrailingEdgeDevice::SetControlParameter(const double param)
{
    // clamp currentControlParam to minimum and maximum values
    double new_param = Clamp(param, GetMinControlParameter(), GetMaxControlParameter());

    if (fabs(new_param - m_currentControlParam) > 1e-6) {
        m_currentControlParam = new_param;
        // make sure the wing gets relofted with flaps
        Invalidate();
    }
}

void CCPACSTrailingEdgeDevice::Invalidate()
{
    CCPACSWing& wing = Wing();
    wing.SetBuildFlaps(true);
    wing.GetConfiguration().AircraftFusingAlgo()->Invalidate();
}

const CCPACSWingComponentSegment& ComponentSegment(const CCPACSTrailingEdgeDevice& self)
{
    return *self.GetParent()->GetParent()->GetParent();
}

CCPACSWingComponentSegment& ComponentSegment(CCPACSTrailingEdgeDevice& self)
{
    return const_cast<CCPACSWingComponentSegment&>(ComponentSegment(const_cast<const CCPACSTrailingEdgeDevice&>(self)));
}

const CCPACSWing& CCPACSTrailingEdgeDevice::Wing() const
{
    return *ComponentSegment(*this).GetParent()->GetParent();
}

CCPACSWing& CCPACSTrailingEdgeDevice::Wing()
{
    return const_cast<CCPACSWing&>(const_cast<const CCPACSTrailingEdgeDevice*>(this)->Wing());
}

void CCPACSTrailingEdgeDevice::ComputeHingePoints(CCPACSTrailingEdgeDevice::HingePoints& hingePoints) const
{
   
    hingePoints.inner = ControlSurfaceDeviceHelper::calc_hinge_point(
        GetPath().GetInnerHingePoint(), 
        GetOuterShape().GetInnerBorder().GetEtaLE(),
        ComponentSegment(*this),
        m_uidMgr,
        "inner"
    );
    hingePoints.outer = ControlSurfaceDeviceHelper::calc_hinge_point(
        GetPath().GetOuterHingePoint(),  
        GetOuterShape().GetOuterBorder().GetEtaLE(),
        ComponentSegment(*this),
        m_uidMgr,
        "outer"
    );
}

void CCPACSTrailingEdgeDevice::ComputeCutoutShape(PNamedShape& shape) const
{

    shape = GetOuterShape().CutoutShape(ComponentSegment(*this).GetWing().GetWingCleanShape(),
                                            GetNormalOfControlSurfaceDevice());
    

}

void CCPACSTrailingEdgeDevice::ComputeFlapShape(PNamedShape& shape) const
{
    shape = GetOuterShape().GetLoft(Wing().GetWingCleanShape(), GetNormalOfControlSurfaceDevice());
    shape->SetName(GetUID().c_str());
}

gp_Vec CCPACSTrailingEdgeDevice::GetNormalOfControlSurfaceDevice() const
{
    return ControlSurfaceDeviceHelper::GetNormalOfControlSurfaceDevice(ComponentSegment(*this));
}

TiglControlSurfaceType CCPACSTrailingEdgeDevice::GetType() const
{
    return m_type;
}

std::string CCPACSTrailingEdgeDevice::GetDefaultedUID() const
{
    return GetUID();
}

TiglGeometricComponentType CCPACSTrailingEdgeDevice::GetComponentType() const
{
    return TIGL_COMPONENT_CONTROL_SURFACE_DEVICE;
}

TiglGeometricComponentIntent CCPACSTrailingEdgeDevice::GetComponentIntent() const
{
    return TIGL_INTENT_OUTER_AERO_SURFACE | TIGL_INTENT_PHYSICAL;
}

PNamedShape CCPACSTrailingEdgeDevice::GetCutOutShape(void) const
{
    return *m_cutoutShape;
}

PNamedShape CCPACSTrailingEdgeDevice::GetFlapShape() const
{
    return *m_flapShape;
}

PNamedShape CCPACSTrailingEdgeDevice::GetTransformedFlapShape() const
{
    return ControlSurfaceDeviceHelper::GetTransformedFlapShape(GetFlapShape()->DeepCopy(), GetFlapTransform());
}

PNamedShape CCPACSTrailingEdgeDevice::BuildLoft() const
{
    return GetFlapShape();
}

} // namespace tigl
