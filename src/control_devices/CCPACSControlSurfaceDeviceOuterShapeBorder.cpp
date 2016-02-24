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

#include "CCPACSControlSurfaceDeviceOuterShapeBorder.h"
#include "CControlSurfaceBoarderBuilder.h"
#include "CNamedShape.h"
#include "CCPACSWingComponentSegment.h"

#include <BRepTools.hxx>

#include <cassert>

namespace tigl
{

CCPACSControlSurfaceDeviceOuterShapeBorder::CCPACSControlSurfaceDeviceOuterShapeBorder(CCPACSWingComponentSegment* segment)
    : _segment(segment)
{
    setUID("ControlSurfaceDevice_OuterShapeBorder");
    xsiType = "";
    xsiLE = -1;
    xsiTE = -1;
    etaLE = -1;
    etaTE = -1;
    leadingEdgeShapeAvailible = false;
}

// Read CPACS Border element
void CCPACSControlSurfaceDeviceOuterShapeBorder::ReadCPACS(
        TixiDocumentHandle tixiHandle, const std::string& BorderXPath,
        TiglControlSurfaceType type)
{
    char* elementPath;
    std::string tempString;
    // getting subelements

    tempString = BorderXPath + "/etaLE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetDoubleElement(tixiHandle, elementPath, &etaLE) != SUCCESS) {
        // couldnt read etaLE
    }

    tempString = BorderXPath + "/etaTE";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, elementPath, &etaTE) != SUCCESS) {
            // error
        }
    }
    else {
        // trailing edge is optional. default is eta of leading edge
        etaTE = etaLE;
    }

    if (type == TRAILING_EDGE_DEVICE) {

        tempString = BorderXPath + "/xsiLE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiLE) != SUCCESS) {
            // couldnt read xsiLE
        }

        xsiTE = 1;

        tempString = BorderXPath + "/leadingEdgeShape";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiCheckElement(tixiHandle,elementPath) == SUCCESS) {
            leadingEdgeShape.ReadCPACS(tixiHandle,elementPath,TRAILING_EDGE_DEVICE);
            leadingEdgeShapeAvailible = true;
        }
    }
    else if (type == LEADING_EDGE_DEVICE) {

        tempString = BorderXPath + "/xsiTEUpper";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiTE) != SUCCESS) {
            // couldnt read xsiLE
        }

        xsiLE = 0;
    }
    else if (type == SPOILER) {

        tempString = BorderXPath + "/xsiLE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiLE) != SUCCESS) {
            // couldnt read xsiLE
        }

        tempString = BorderXPath + "/xsiTE";
        elementPath = const_cast<char*>(tempString.c_str());
        if (tixiGetDoubleElement(tixiHandle, elementPath, &xsiTE) != SUCCESS) {
            // couldnt read xsiTE
        }
    }
}

double CCPACSControlSurfaceDeviceOuterShapeBorder::getEtaLE() const
{
    return etaLE;
}
double CCPACSControlSurfaceDeviceOuterShapeBorder::getEtaTE() const
{
    return etaTE;
}
double CCPACSControlSurfaceDeviceOuterShapeBorder::getXsiLE() const
{
    return xsiLE;
}
double CCPACSControlSurfaceDeviceOuterShapeBorder::getXsiTE() const
{
    return xsiTE;
}

TopoDS_Wire CCPACSControlSurfaceDeviceOuterShapeBorder::getWire(PNamedShape wingShape, gp_Vec upDir) const
{
    assert(wingShape);

    // Compute cutout plane
    CTiglControlSurfaceBorderCoordinateSystem coords = getCoordinateSystem(upDir);
    CControlSurfaceBoarderBuilder builder(coords, wingShape->Shape());

    const CCPACSControlSurfaceDeviceOuterShapeBorder * border = this;
    
    TopoDS_Wire wire;
    if (border->isLeadingEdgeShapeAvailable()) {
        CCPACSControlSurfaceDeviceBorderLeadingEdgeShape leShape = border->getLeadingEdgeShape();
        wire = builder.boarderWithLEShape(leShape.getRelHeightLE(), 1.0, leShape.getXsiUpperSkin(), leShape.getXsiLowerSkin());
    }
    else {
        wire = builder.boarderSimple(1.0, 1.0);
    }

#ifdef DEBUG
    std::stringstream filenamestr;
    filenamestr << _uid << "_wire.brep";
    BRepTools::Write(wire, filenamestr.str().c_str());
#endif

    return wire;
}

CTiglControlSurfaceBorderCoordinateSystem CCPACSControlSurfaceDeviceOuterShapeBorder::getCoordinateSystem(gp_Vec upDir) const
{
    gp_Pnt pLE = _segment->GetPoint(getEtaLE(), getXsiLE());
    gp_Pnt pTE = _segment->GetPoint(getEtaLE(), getXsiTE());

    CTiglControlSurfaceBorderCoordinateSystem coords(pLE, pTE, upDir);
    return coords;
}

CCPACSControlSurfaceDeviceBorderLeadingEdgeShape CCPACSControlSurfaceDeviceOuterShapeBorder::getLeadingEdgeShape() const
{
    return leadingEdgeShape;
}

bool CCPACSControlSurfaceDeviceOuterShapeBorder::isLeadingEdgeShapeAvailable() const
{
    return leadingEdgeShapeAvailible;
}

void CCPACSControlSurfaceDeviceOuterShapeBorder::setUID(const std::string& uid)
{
    _uid = uid;
}

} // end namespace tigl
