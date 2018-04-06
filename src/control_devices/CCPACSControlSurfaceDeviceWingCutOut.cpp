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

#include "CTiglError.h"

#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"

#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CControlSurfaceBorderBuilder.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"
#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CNamedShape.h"
#include "CTiglLogging.h"

#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepTools.hxx>

#include <string>
#include <cassert>

namespace tigl
{

CCPACSControlSurfaceDeviceWingCutOut::CCPACSControlSurfaceDeviceWingCutOut(const CCPACSControlSurfaceDevice& dev,
                                                                           const CCPACSWingComponentSegment& cs)
    : _segment(cs)
    , _csDevice(dev)
{
}

// Read CPACS ControlSurface DeviceWingCutOut element
void CCPACSControlSurfaceDeviceWingCutOut::ReadCPACS(
        TixiDocumentHandle tixiHandle,
        const std::string& xpath)
{
    char* elementPath;
    std::string tempString;


    tempString = xpath + "/cutOutProfiles";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        wingCutOutProfiles.ReadCPACS(tixiHandle,elementPath);
    }

    if (tixiCheckElement(tixiHandle, (xpath + "/upperSkin").c_str()) == SUCCESS) {
        _upperSkin.ReadCPACS(tixiHandle, xpath + "/upperSkin");
    }
    else {
        throw CTiglError("Missing upperSkin element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/upperSkin").c_str()) == SUCCESS) {
        _lowerSkin.ReadCPACS(tixiHandle, xpath + "/lowerSkin");
    }
    else {
        throw CTiglError("Missing lowerSkin element in path: " + xpath + "!", TIGL_OPEN_FAILED);
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/innerBorder").c_str()) == SUCCESS) {
        _innerBorder = CCPACSControlSurfaceSkinCutoutBorderPtr(new CCPACSControlSurfaceSkinCutoutBorder);
        _innerBorder->ReadCPACS(tixiHandle, xpath + "/innerBorder");
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/outerBorder").c_str()) == SUCCESS) {
        _outerBorder = CCPACSControlSurfaceSkinCutoutBorderPtr(new CCPACSControlSurfaceSkinCutoutBorder);
        _outerBorder->ReadCPACS(tixiHandle, xpath + "/outerBorder");
    }
    
    if (tixiCheckElement(tixiHandle, (xpath + "/cutOutProfileControlPoint").c_str()) == SUCCESS) {
        _cutOutProfileControlPoints = CCPACSCutOutControlPointsPtr(new CCPACSCutOutControlPoints);
        _cutOutProfileControlPoints->ReadCPACS(tixiHandle, xpath + "/cutOutProfileControlPoint");
    }
}

const CCPACSCutOutControlPointsPtr CCPACSControlSurfaceDeviceWingCutOut::cutOutProfileControlPoints() const
{
    return _cutOutProfileControlPoints;
}

const CCPACSControlSurfaceSkinCutOut& CCPACSControlSurfaceDeviceWingCutOut::upperSkin() const
{
    return _upperSkin;
}

const CCPACSControlSurfaceSkinCutOut& CCPACSControlSurfaceDeviceWingCutOut::lowerSkin() const
{
    return _lowerSkin;
}

const CCPACSControlSurfaceSkinCutoutBorderPtr CCPACSControlSurfaceDeviceWingCutOut::innerBorder() const
{
    return _innerBorder;
}

const CCPACSControlSurfaceSkinCutoutBorderPtr CCPACSControlSurfaceDeviceWingCutOut::outerBorder() const
{
    return _outerBorder;
}

PNamedShape CCPACSControlSurfaceDeviceWingCutOut::GetLoft(PNamedShape wingCleanShape,
                                                              CCPACSControlSurfaceDeviceOuterShape* outerShape,
                                                              gp_Vec upDir)
{
    if (_loft) {
        return _loft;
    }

    DLOG(INFO) << "Building " << _csDevice.GetUID() << " wing cutout shape";

    // Get Wires definng the Shape of the more complex CutOutShape.
    TopoDS_Wire innerWire = getCutoutWire(true, wingCleanShape, &outerShape->getInnerBorder(), upDir);
    TopoDS_Wire outerWire = getCutoutWire(false, wingCleanShape, &outerShape->getOuterBorder(), upDir);

    // make one shape out of the 2 wires and build connections inbetween.
    BRepOffsetAPI_ThruSections thrusections(true,true);
    thrusections.AddWire(outerWire);
    thrusections.AddWire(innerWire);
    thrusections.Build();

    _loft = PNamedShape(new CNamedShape(thrusections.Shape(), _csDevice.GetUID().c_str()));

#ifdef DEBUG
    std::stringstream filenamestr;
    filenamestr << _csDevice->GetUID() << "_cutout.brep";
    BRepTools::Write(_loft->Shape(), filenamestr.str().c_str());
#endif


    _loft->SetShortName(_csDevice.GetShortShapeName().c_str());

    return _loft;
}

TopoDS_Wire CCPACSControlSurfaceDeviceWingCutOut::getCutoutWire(bool isInnerBorder,
                                                                PNamedShape wingCleanShape,
                                                                const CCPACSControlSurfaceDeviceOuterShapeBorder* outerBorder,
                                                                gp_Vec upDir)
{
    assert(wingCleanShape);

    TopoDS_Wire wire;

    CTiglControlSurfaceBorderCoordinateSystem coords(getCutoutCS(isInnerBorder, outerBorder, upDir));
    CControlSurfaceBorderBuilder builder(coords, wingCleanShape->Shape());

    double xsiUpper, xsiLower;
    if (isInnerBorder) {
        xsiUpper = upperSkin().xsiInnerBorder();
        xsiLower = lowerSkin().xsiInnerBorder();
    }
    else {
        xsiUpper = upperSkin().xsiOuterBorder();
        xsiLower = lowerSkin().xsiOuterBorder();
    }

    // TODO: calculate xsis into coordinate system of the border

    CCPACSCutOutControlPointsPtr lePoints = cutOutProfileControlPoints();
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

        wire = builder.borderWithLEShape(lePoint->relHeight(), xsiNose, xsiUpper, xsiLower);
    }
    else {
        wire = builder.borderSimple(xsiUpper, xsiLower);
    }

    return wire;
}

CTiglControlSurfaceBorderCoordinateSystem
CCPACSControlSurfaceDeviceWingCutOut::getCutoutCS(bool isInnerBorder,
                                                  const CCPACSControlSurfaceDeviceOuterShapeBorder* outerShapeBorder,
                                                  gp_Vec upDir)
{
    CCPACSControlSurfaceSkinCutoutBorderPtr cutOutBorder = 
            isInnerBorder ? innerBorder() : outerBorder();

    if (!cutOutBorder) {
        return outerShapeBorder->getCoordinateSystem(upDir);
    }

    double lEta = cutOutBorder->etaLE();
    double lXsi = outerShapeBorder->getXsiLE();
    double tEta = cutOutBorder->etaTE();
    double tXsi = outerShapeBorder->getXsiTE();

    gp_Pnt pLE = _segment.GetPoint(lEta, lXsi);
    gp_Pnt pTE = _segment.GetPoint(tEta, tXsi);

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

} // end namespace tigl
