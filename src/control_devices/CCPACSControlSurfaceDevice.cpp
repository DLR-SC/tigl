/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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

#include <iostream>
#include <sstream>
#include <exception>
#include <cassert>
#include <algorithm>
#include <limits>

#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CCPACSControlSurfaceDeviceSteps.h"
#include "tiglcommonfunctions.h"

#include <BRepBuilderAPI_Transform.hxx>


namespace tigl
{

CCPACSControlSurfaceDevice::CCPACSControlSurfaceDevice(CCPACSConfiguration* config, CCPACSWingComponentSegment* segment)
    : outerShape(this, segment), _segment(segment), _config(config)
{
    SetUID("ControlDevice");
    _hingeLine = CSharedPtr<CTiglControlSurfaceHingeLine>(new CTiglControlSurfaceHingeLine(&outerShape, &path,_segment));
}

// Read CPACS trailingEdgeDevice elements
void CCPACSControlSurfaceDevice::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& controlSurfaceDeviceXPath, TiglControlSurfaceType type)
{
    char*       elementPath;
    std::string tempString;

    // Get sublement "outerShape"
    char* ptrName = NULL;
    tempString    = controlSurfaceDeviceXPath + "/outerShape";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        outerShape.ReadCPACS(tixiHandle, elementPath, type);
    }

    // Get Path
    tempString = controlSurfaceDeviceXPath + "/path";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        path.ReadCPACS(tixiHandle, elementPath);
    }


    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, controlSurfaceDeviceXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get WingCutOut, this is not implemented yet.
    tempString = controlSurfaceDeviceXPath + "/wingCutOut";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        wingCutOut = CSharedPtr<CCPACSControlSurfaceDeviceWingCutOut>(new CCPACSControlSurfaceDeviceWingCutOut(this, _segment));
        wingCutOut->ReadCPACS(tixiHandle, elementPath);
    }

    _type = type;

    currentDeflection = GetMinDeflection() > 0? GetMinDeflection() : 0;
    currentDeflection = currentDeflection > GetMaxDeflection()? GetMaxDeflection() : currentDeflection;
}

PNamedShape CCPACSControlSurfaceDevice::BuildLoft()
{
    return getFlapShape();
}

const CCPACSControlSurfaceDeviceOuterShape& CCPACSControlSurfaceDevice::getOuterShape() const
{
    return outerShape;
}

const CCPACSControlSurfaceDevicePath& CCPACSControlSurfaceDevice::getMovementPath() const
{
    return path;
}

gp_Trsf CCPACSControlSurfaceDevice::GetFlapTransform() const
{
    // this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
    // to the given relDeflection by using a linearInterpolation.
    std::vector<double> relDeflections, innerXTrans, outerXTrans, innerYTrans, innerZTrans, outerZTrans, rotations;
    
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    for (int istep = 1; istep <= steps.GetStepCount(); ++istep) {
        CCPACSControlSurfaceDeviceStep& step = steps.GetStep(istep);
        relDeflections.push_back(step.getRelDeflection());
        innerXTrans.push_back(step.getInnerHingeTranslation().x);
        innerYTrans.push_back(step.getInnerHingeTranslation().y);
        innerZTrans.push_back(step.getInnerHingeTranslation().z);
        outerXTrans.push_back(step.getOuterHingeTranslation().x);
        outerZTrans.push_back(step.getOuterHingeTranslation().z);
        rotations.push_back(step.getHingeLineRotation());
    }
    
    double rotation = Interpolate( relDeflections, rotations, currentDeflection );
    double innerTranslationX = Interpolate( relDeflections, innerXTrans, currentDeflection );
    double innerTranslationY = Interpolate( relDeflections, innerYTrans, currentDeflection );
    double innerTranslationZ = Interpolate( relDeflections, innerZTrans, currentDeflection );
    double outerTranslationX = Interpolate( relDeflections, outerXTrans, currentDeflection );
    double outerTranslationZ = Interpolate( relDeflections, outerZTrans, currentDeflection );

    gp_Pnt innerHingeOld = _hingeLine->getInnerHingePoint();;
    gp_Pnt outerHingeOld = _hingeLine->getOuterHingePoint();;

    // innerTranslationY on hingePoint1 on purpose, maybe consider setting it to zero as default. See CPACS definition on
    // Path/Step/HingeLineTransformation for more informations.
    gp_Pnt hingePoint1 = _hingeLine->getOuterHingePoint().XYZ() + gp_XYZ(outerTranslationX, innerTranslationY, outerTranslationZ);
    gp_Pnt hingePoint2 = _hingeLine->getInnerHingePoint().XYZ() + gp_XYZ(innerTranslationX, innerTranslationY, innerTranslationZ);

    // calculating the needed transformations
    CTiglControlSurfaceTransformation transformation(innerHingeOld, outerHingeOld, hingePoint2, hingePoint1, rotation);

    return transformation.getTotalTransformation();
}

PNamedShape CCPACSControlSurfaceDevice::getCutOutShape()
{
    if (!wingCutOut) {
        return outerShape.cutoutShape(
                    _segment->GetWing().GetWingCleanShape(),
                    getNormalOfControlSurfaceDevice());
    }
    else {
        return wingCutOut->GetLoft(
                    _segment->GetWing().GetWingCleanShape(),
                    &outerShape,
                    getNormalOfControlSurfaceDevice());
    }
}

PNamedShape CCPACSControlSurfaceDevice::getFlapShape()
{
    return outerShape.GetLoft(
                _segment->GetWing().GetWingCleanShape(),
                getNormalOfControlSurfaceDevice());
}

PNamedShape CCPACSControlSurfaceDevice::getTransformedFlapShape()
{
    PNamedShape deviceShape = getFlapShape()->DeepCopy();
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

gp_Vec CCPACSControlSurfaceDevice::getNormalOfControlSurfaceDevice()
{
    gp_Pnt point1 = _segment->GetPoint(0,0);
    gp_Pnt point2 = _segment->GetPoint(0,1);
    gp_Pnt point3 = _segment->GetPoint(1,0);

    gp_Vec dir1to2 = -(gp_Vec(point1.XYZ()) - gp_Vec(point2.XYZ()));
    gp_Vec dir1to3 = -(gp_Vec(point1.XYZ()) - gp_Vec(point3.XYZ()));

    gp_Vec nvV = dir1to2^dir1to3;
    nvV.Normalize();
    return nvV;
}

CCPACSWingComponentSegment* CCPACSControlSurfaceDevice::getSegment()
{
    return _segment;
}

// get short name for loft
std::string CCPACSControlSurfaceDevice::GetShortShapeName()
{
    std::string tmp = _segment->GetLoft()->ShortName();
    for (int j = 1; j <= _segment->getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount(); j++) {
        tigl::CCPACSControlSurfaceDevice& csd = _segment->getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(j);
        if (GetUID() == csd.GetUID()) {
            std::stringstream shortName;
            if (_type == LEADING_EDGE_DEVICE) {
                shortName << tmp << "LED" << j;
            }
            else if (_type == TRAILING_EDGE_DEVICE) {
                shortName << tmp << "TED" << j;
            }
            else if (_type == SPOILER) {
                shortName << tmp << "SPO" << j;
            }
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

TiglControlSurfaceType CCPACSControlSurfaceDevice::getType()
{
    return _type;
}

void CCPACSControlSurfaceDevice::SetUID(const std::string& uid)
{
    CTiglAbstractGeometricComponent::SetUID(uid);

    outerShape.setUID(uid + "OuterShape");
}



double CCPACSControlSurfaceDevice::GetMinDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    double ret = std::numeric_limits<double>::max();
    for (int i=1; i<= steps.GetStepCount(); i++) {
        double deflect = steps.GetStep(i).getRelDeflection();
        ret = std::min(ret, deflect);
    }
    return ret;
}

double CCPACSControlSurfaceDevice::GetMaxDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    double ret = -std::numeric_limits<double>::max();
    for (int i=1; i<= steps.GetStepCount(); i++) {
        double deflect = steps.GetStep(i).getRelDeflection();
        ret = std::max(ret, deflect);
    }
    return ret;
}

double CCPACSControlSurfaceDevice::GetDeflection() const
{
    return currentDeflection;
}

void CCPACSControlSurfaceDevice::SetDeflection(const double deflect)
{
    double maxDeflect = GetMaxDeflection();
    double minDeflect = GetMinDeflection();

    // clamp currentDeflection to minimum and maximum values
    currentDeflection = (deflect > maxDeflect? maxDeflect : deflect);
    currentDeflection = (currentDeflection < minDeflect? minDeflect : currentDeflection);

    // make sure the wing gets relofted with with flaps
    _segment->GetWing().SetBuildFlaps(true);
    _segment->GetWing().Invalidate();
}

} // end namespace tigl
