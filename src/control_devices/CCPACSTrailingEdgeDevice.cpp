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
#include "CCPACSControlSurfaceStep.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CTiglFusePlane.h"

#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"
#include "tigletaxsifunctions.h"

// TODO: Replace by TiGL function
#include <BRepBuilderAPI_Transform.hxx>

namespace tigl
{

CCPACSTrailingEdgeDevice::CCPACSTrailingEdgeDevice(CCPACSTrailingEdgeDevices* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSTrailingEdgeDevice(parent, uidMgr)
    , m_type(TRAILING_EDGE_DEVICE)
    , m_hingePoints(*this, &CCPACSTrailingEdgeDevice::ComputeHingePoints)
{
}

void CCPACSTrailingEdgeDevice::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{
    CPACSTrailingEdgeDevice::ReadCPACS(tixiHandle, xpath);

    m_currentDeflection = Clamp(0., GetMinDeflection(), GetMaxDeflection());
}

// get short name for loft
std::string CCPACSTrailingEdgeDevice::GetShortName() const
{
    const CCPACSTrailingEdgeDevices* devices = GetParent();


    std::string tmp = devices->GetParent()->GetParent()->GetShortName();

    size_t idx = IndexFromUid(devices->GetTrailingEdgeDevices(), GetUID());
    if (idx < devices->GetTrailingEdgeDevices().size()) {
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

gp_Trsf CCPACSTrailingEdgeDevice::GetFlapTransform() const
{
    // this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
    // to the given relDeflection by using a linearInterpolation.
    std::vector<double> relDeflections, innerXTrans, outerXTrans, innerYTrans, innerZTrans, outerZTrans, rotations;

    const CCPACSControlSurfaceSteps& steps = GetPath().GetSteps();
    for (const auto& step : steps.GetSteps()) {
        relDeflections.push_back(step->GetRelDeflection());
        CTiglPoint innerHingeTrans(0,0,0);
        if (step->GetInnerHingeTranslation().is_initialized()) {
            innerHingeTrans = step->GetInnerHingeTranslation()->AsPoint();
        }
        innerXTrans.push_back(innerHingeTrans.x);
        innerYTrans.push_back(innerHingeTrans.y);
        innerZTrans.push_back(innerHingeTrans.z);

        CTiglPoint outerHingeTrans(0,0,0);
        if (step->GetOuterHingeTranslation().is_initialized()) {
            outerHingeTrans.x = step->GetOuterHingeTranslation()->GetX();
            outerHingeTrans.z = step->GetOuterHingeTranslation()->GetZ();
        }

        outerXTrans.push_back(outerHingeTrans.x);
        outerZTrans.push_back(outerHingeTrans.z);
        rotations.push_back(step->GetHingeLineRotation().value_or(0.));
    }

    double rotation = Interpolate( relDeflections, rotations, m_currentDeflection );
    double innerTranslationX = Interpolate( relDeflections, innerXTrans, m_currentDeflection );
    double innerTranslationY = Interpolate( relDeflections, innerYTrans, m_currentDeflection );
    double innerTranslationZ = Interpolate( relDeflections, innerZTrans, m_currentDeflection );
    double outerTranslationX = Interpolate( relDeflections, outerXTrans, m_currentDeflection );
    double outerTranslationZ = Interpolate( relDeflections, outerZTrans, m_currentDeflection );

    gp_Pnt innerHingeOld = m_hingePoints->inner;
    gp_Pnt outerHingeOld = m_hingePoints->outer;

    // innerTranslationY on hingePoint1 on purpose, maybe consider setting it to zero as default. See CPACS definition on
    // Path/Step/HingeLineTransformation for more informations.
    gp_Pnt outerHingeNew = outerHingeOld.XYZ() + gp_XYZ(outerTranslationX, innerTranslationY, outerTranslationZ);
    gp_Pnt innerHingeNew = innerHingeOld.XYZ() + gp_XYZ(innerTranslationX, innerTranslationY, innerTranslationZ);

    // calculating the needed transformations
    CTiglControlSurfaceTransformation transformation(innerHingeOld, outerHingeOld, innerHingeNew, outerHingeNew, rotation);

    return transformation.getTotalTransformation();
}

double CCPACSTrailingEdgeDevice::GetMinDeflection() const
{
    return GetPath().GetSteps().GetSteps().front()->GetRelDeflection();
}

double CCPACSTrailingEdgeDevice::GetMaxDeflection() const
{
    return GetPath().GetSteps().GetSteps().back()->GetRelDeflection();
}

double CCPACSTrailingEdgeDevice::GetDeflection() const
{
    return m_currentDeflection;
}

void CCPACSTrailingEdgeDevice::SetDeflection(const double deflect)
{
    // clamp currentDeflection to minimum and maximum values
    m_currentDeflection = Clamp(deflect, GetMinDeflection(), GetMaxDeflection());

    // make sure the wing gets relofted with flaps
    Invalidate();
}

void CCPACSTrailingEdgeDevice::Invalidate()
{
    CCPACSWing& wing = Wing();
    wing.SetBuildFlaps(true);
    wing.Invalidate();
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


const CCPACSWing &CCPACSTrailingEdgeDevice::Wing() const
{
    return *ComponentSegment(*this).GetParent()->GetParent();
}

CCPACSWing &CCPACSTrailingEdgeDevice::Wing()
{
    return const_cast<CCPACSWing&>(const_cast<const CCPACSTrailingEdgeDevice*>(this)->Wing());
}

void CCPACSTrailingEdgeDevice::ComputeHingePoints(CCPACSTrailingEdgeDevice::HingePoints &hingePoints) const
{

    // get the Loft geometry of the Component Segment
    const CCPACSWingComponentSegment& cSegment = ComponentSegment(*this);
    CTiglWingStructureReference wsr(cSegment);

    // xsi positions of hinge points
    std::vector<double> hingeXsi = {
        GetPath().GetInnerHingePoint().GetHingeXsi(),
        GetPath().GetOuterHingePoint().GetHingeXsi()
    };

    // relative hinge heights
    std::vector<double> relHingeHeight = {
        GetPath().GetInnerHingePoint().GetHingeRelHeight(),
        GetPath().GetOuterHingePoint().GetHingeRelHeight()
    };

    std::vector<gp_Pnt> points(2);

    // Loop over both hinge line points and determine their location
    for (size_t i = 0; i < hingeXsi.size(); i++) {
        // variable part of name for error msg
        std::string innerOuter = (i == 0 ? "inner" : "outer");
        // inner/outer border of trailing edge device
        const CCPACSControlSurfaceBorderTrailingEdge& border =
            (i == 0 ? GetOuterShape().GetInnerBorder() : GetOuterShape().GetOuterBorder());

        if (relHingeHeight[i] < 0. || 1. < relHingeHeight[i]) {
            LOG(ERROR) << "The " << innerOuter << "HingeHeight is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeHeight is not between 0. and 1. in CCPACSTrailingEdgeDevice::buildHingePoints.");
        }
        if (hingeXsi[i] < 0. || 1. < hingeXsi[i]) {
            LOG(ERROR) << "The " << innerOuter << "HingeXsi is not between 0. and 1.";
            throw CTiglError("The " + innerOuter +
                             "HingeXsi is not between 0. and 1. in CCPACSTrailingEdgeDevice::buildHingePoints.");
        }
        if (hingeXsi[i] > border.GetXsiLE().GetXsi()) {
            LOG(ERROR) << "The " << innerOuter << "HingeXsi is behind the trailing edge device leading edge";
            throw CTiglError("The " + innerOuter + "HingeXsi is behind the trailing edge device leading edge");
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

        points[i] = lower;
    }

    hingePoints.inner = points[0];
    hingePoints.outer = points[1];
}

gp_Vec CCPACSTrailingEdgeDevice::GetNormalOfControlSurfaceDevice() const
{
    const CCPACSWingComponentSegment& compSeg = ComponentSegment(*this);
    gp_Pnt point1 = compSeg.GetPoint(0,0);
    gp_Pnt point2 = compSeg.GetPoint(0,1);
    gp_Pnt point3 = compSeg.GetPoint(1,0);

    gp_Vec dir1to2 = -(gp_Vec(point1.XYZ()) - gp_Vec(point2.XYZ()));
    gp_Vec dir1to3 = -(gp_Vec(point1.XYZ()) - gp_Vec(point3.XYZ()));

    gp_Vec nvV = dir1to2^dir1to3;
    nvV.Normalize();
    return nvV;
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
    if (!GetWingCutOut()) {
        return GetOuterShape().CutoutShape(
                    ComponentSegment(*this).GetWing().GetWingCleanShape(),
                    GetNormalOfControlSurfaceDevice());
    }
    else {
        return GetWingCutOut()->GetLoft(
                    ComponentSegment(*this).GetWing().GetWingCleanShape(),
                    GetOuterShape(),
                    GetNormalOfControlSurfaceDevice());
    }
}

PNamedShape CCPACSTrailingEdgeDevice::GetFlapShape() const
{
    PNamedShape loft =  GetOuterShape().GetLoft(
                Wing().GetWingCleanShape(),
                GetNormalOfControlSurfaceDevice());
    loft->SetName(GetUID().c_str());
    return loft;
}

PNamedShape CCPACSTrailingEdgeDevice::GetTransformedFlapShape() const
{
    PNamedShape deviceShape = GetFlapShape()->DeepCopy();
    gp_Trsf T = GetFlapTransform();
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

PNamedShape CCPACSTrailingEdgeDevice::BuildLoft() const
{
    return GetFlapShape();
}

} // namespace tigl
