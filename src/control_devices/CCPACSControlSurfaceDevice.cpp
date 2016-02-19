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

#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CControlSurfaceBoarderBuilder.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDeviceBorderLeadingEdgeShape.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CCPACSControlSurfaceDeviceSteps.h"
#include "tiglcommonfunctions.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepTools.hxx"

#define USE_ADVANCED_MODELING 1

namespace tigl
{

CCPACSControlSurfaceDevice::CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment)
    : _segment(segment)
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
        wingCutOut = CSharedPtr<CCPACSControlSurfaceDeviceWingCutOut>(new CCPACSControlSurfaceDeviceWingCutOut);
        wingCutOut->ReadCPACS(tixiHandle, elementPath);
    }

    _type = type;
}

void CCPACSControlSurfaceDevice::setLoft(PNamedShape loft)
{
    this->_outerShape = loft;
}

PNamedShape CCPACSControlSurfaceDevice::BuildLoft()
{
    if (!_outerShape) {
        // TODO: build
    }
    return _outerShape;
}

const CCPACSControlSurfaceDeviceOuterShape& CCPACSControlSurfaceDevice::getOuterShape() const
{
    return outerShape;
}

const CCPACSControlSurfaceDevicePath& CCPACSControlSurfaceDevice::getMovementPath() const
{
    return path;
}

gp_Trsf CCPACSControlSurfaceDevice::GetFlapTransform(double deflection) const
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
    
    double rotation = Interpolate( relDeflections, rotations, deflection );
    double innerTranslationX = Interpolate( relDeflections, innerXTrans, deflection );
    double innerTranslationY = Interpolate( relDeflections, innerYTrans, deflection );
    double innerTranslationZ = Interpolate( relDeflections, innerZTrans, deflection );
    double outerTranslationX = Interpolate( relDeflections, outerXTrans, deflection );
    double outerTranslationZ = Interpolate( relDeflections, outerZTrans, deflection );

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
    if (!_wingCutOutShape) {
        if (_type == TRAILING_EDGE_DEVICE) {
    
    
            // Get Wires definng the Shape of the more complex CutOutShape.
            TopoDS_Wire innerWire = getCutoutWire(true);
            TopoDS_Wire outerWire = getCutoutWire(false);
    
            // make one shape out of the 2 wires and build connections inbetween.
            BRepOffsetAPI_ThruSections thrusections(true,true);
            thrusections.AddWire(outerWire);
            thrusections.AddWire(innerWire);
            thrusections.Build();
    
            _wingCutOutShape = PNamedShape(new CNamedShape(thrusections.Shape(), GetUID().c_str()));
            _wingCutOutShape->SetShortName(GetShortShapeName().c_str());
    
#ifdef DEBUG
            std::stringstream filenamestr;
            filenamestr << GetUID() << "_cutout.brep";
            BRepTools::Write(_wingCutOutShape->Shape(), filenamestr.str().c_str());
#endif
        }
        else {
            throw CTiglError("Control surface type not yet supported", TIGL_ERROR);
        }
    }
    
    return _wingCutOutShape;

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

CSCoordSystem CCPACSControlSurfaceDevice::getOuterShapeCS(bool isInnerBorder)
{
    double lEta, lXsi, tEta, tXsi;
    if (isInnerBorder) {
        lEta = getOuterShape().getInnerBorder().getEtaLE();
        lXsi = getOuterShape().getInnerBorder().getXsiLE();
        tEta = getOuterShape().getInnerBorder().getEtaLE();
        tXsi = getOuterShape().getInnerBorder().getXsiTE();
    }
    else {
        lEta = getOuterShape().getOuterBorder().getEtaLE();
        lXsi = getOuterShape().getOuterBorder().getXsiLE();
        tEta = getOuterShape().getOuterBorder().getEtaLE();
        tXsi = getOuterShape().getOuterBorder().getXsiTE();
    }

    
    gp_Pnt pLE = _segment->GetPoint(lEta, lXsi);
    gp_Pnt pTE = _segment->GetPoint(tEta, tXsi);
    gp_Vec upDir = getNormalOfControlSurfaceDevice();

    CSCoordSystem coords(pLE, pTE, upDir);
    return coords;
}

CSCoordSystem CCPACSControlSurfaceDevice::getCutoutCS(bool isInnerBorder)
{
    // TODO: take account for custom etas two model gaps
    return getOuterShapeCS(isInnerBorder);
}



// Computes the wire for the cutout loft. This is done as follows
// Currently only for trailing edge devices
TopoDS_Wire CCPACSControlSurfaceDevice::getOuterShapeWire(bool isInnerBorder)
{
    // Compute cutout plane
    CSCoordSystem coords(getOuterShapeCS(isInnerBorder));
    CControlSurfaceBoarderBuilder builder(coords, _segment->GetLoft()->Shape());

    const CCPACSControlSurfaceDeviceOuterShapeBorder * border;
    if (isInnerBorder) {
        border = &(getOuterShape().getInnerBorder());
    }
    else {
        border = &(getOuterShape().getOuterBorder());
    }
    
    TopoDS_Wire wire;
#if USE_ADVANCED_MODELING
    if (border->isLeadingEdgeShapeAvailable()) {
        CCPACSControlSurfaceDeviceBorderLeadingEdgeShape leShape = border->getLeadingEdgeShape();
        wire = builder.boarderWithLEShape(leShape.getRelHeightLE(), 1.0, leShape.getXsiUpperSkin(), leShape.getXsiLowerSkin());
    }
    else {
        wire = builder.boarderSimple(1.0, 1.0);
    }
#else
    wire = builder.boarderSimple();
#endif

#ifdef DEBUG
    std::stringstream filenamestr;
    if (isInnerBorder) {
        filenamestr << GetUID() << "_outershape_innerwire.brep";
    }
    else {
        filenamestr << GetUID() << "_outershape_outerwire.brep";
    }
    BRepTools::Write(wire, filenamestr.str().c_str());
#endif

    return wire;
}

TopoDS_Wire CCPACSControlSurfaceDevice::getCutoutWire(bool isInnerBorder)
{
    TopoDS_Wire wire;
    if (!wingCutOut) {
        wire = getOuterShapeWire(isInnerBorder);
    }
    else {
        CSCoordSystem coords(getCutoutCS(isInnerBorder));
        CControlSurfaceBoarderBuilder builder(coords, _segment->GetLoft()->Shape());

        double xsiUpper, xsiLower;
        if (isInnerBorder) {
            xsiUpper = wingCutOut->upperSkin().xsiInnerBorder();
            xsiLower = wingCutOut->lowerSkin().xsiInnerBorder();
        }
        else {
            xsiUpper = wingCutOut->upperSkin().xsiOuterBorder();
            xsiLower = wingCutOut->lowerSkin().xsiOuterBorder();
        }
        
        // TODO: calculate xsis into coordinate system of the border

        CCPACSCutOutControlPointsPtr lePoints = wingCutOut->cutOutProfileControlPoints();
        if (lePoints) {
            const CCPACSCutOutControlPoint* lePoint;
            if (isInnerBorder) {
                lePoint = &(lePoints->innerBorder());
            }
            else {
                lePoint = &(lePoints->outerBorder());
            }
            
            double xsiNose = lePoint->xsi();
            // TODO: calculate xsiNose into coordinate system of the border
            
            wire = builder.boarderWithLEShape(lePoint->relHeight(), xsiNose, xsiUpper, xsiLower);
        }
        else {
            wire = builder.boarderSimple(xsiUpper, xsiLower);
        }
        
        throw CTiglError("WingCut moddeling not yet implemented");
    }

    return wire;
}

double CCPACSControlSurfaceDevice::GetMinDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    CCPACSControlSurfaceDeviceStep step = steps.GetStep(1);
    
    return step.getRelDeflection();
}

double CCPACSControlSurfaceDevice::GetMaxDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    CCPACSControlSurfaceDeviceStep step = steps.GetStep(steps.GetStepCount());
    
    return step.getRelDeflection();
}

} // end namespace tigl
