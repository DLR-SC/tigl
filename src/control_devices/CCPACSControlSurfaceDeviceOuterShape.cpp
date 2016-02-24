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

#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CCPACSControlSurfaceDevice.h"
#include "CBopCommon.h"
#include "CNamedShape.h"
#include "CTiglLogging.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepTools.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <cassert>


namespace tigl
{

CCPACSControlSurfaceDeviceOuterShape::CCPACSControlSurfaceDeviceOuterShape(
        CCPACSControlSurfaceDevice* device,
        CCPACSWingComponentSegment* segment)

    : innerBorder(segment),
      outerBorder(segment),
      _segment(segment),
      _csDevice(device)
{
    setUID("ControlSurfaceDevice_OuterShape");
}

// Read CPACS outerShape element
void CCPACSControlSurfaceDeviceOuterShape::ReadCPACS(TixiDocumentHandle tixiHandle,
                                                     const std::string& outerShapeXPath,
                                                     TiglControlSurfaceType type)
{

    char* elementPath;
    std::string tempString;
    char* ptrName = NULL;

    // Get outerBorder
    tempString = outerShapeXPath + "/outerBorder";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        outerBorder.ReadCPACS(tixiHandle, elementPath, type);
    }

    // Get innerBorder
    tempString = outerShapeXPath + "/innerBorder";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        innerBorder.ReadCPACS(tixiHandle, elementPath, type);
    }
}

const CCPACSControlSurfaceDeviceOuterShapeBorder& CCPACSControlSurfaceDeviceOuterShape::getInnerBorder() const
{
    return innerBorder;
}
const CCPACSControlSurfaceDeviceOuterShapeBorder& CCPACSControlSurfaceDeviceOuterShape::getOuterBorder() const
{
    return outerBorder;
}

PNamedShape CCPACSControlSurfaceDeviceOuterShape::GetLoft(PNamedShape wingCleanShape, gp_Vec upDir)
{
    if (_outerShape) {
        return _outerShape;
    }

    DLOG(INFO) << "Building " << _uid << " loft";
    if (needsWingIntersection()) {

        PNamedShape shapeBox = cutoutShape(wingCleanShape, upDir);

        assert(shapeBox);

        // perform the boolean intersection of the flap box with the wing 
        _outerShape = CBopCommon(wingCleanShape, shapeBox);

        for (unsigned int iFace = 0; iFace < _outerShape->GetFaceCount(); ++iFace) {
            CFaceTraits ft = _outerShape->GetFaceTraits(iFace);
            ft.SetOrigin(shapeBox);
            _outerShape->SetFaceTraits(iFace, ft);
        }

#ifdef DEBUG
        std::stringstream filenamestr2;
        filenamestr2 << _uid << ".brep";
        BRepTools::Write(_outerShape->Shape(), filenamestr2.str().c_str());
#endif
        
        return _outerShape;
    }
    else {
        // !needsWingIntersection

        // We need to model the flap like a wing by creating sections
        throw CTiglError("Modeling from ground up not yet supported", TIGL_ERROR);
    }
    
}

PNamedShape CCPACSControlSurfaceDeviceOuterShape::cutoutShape(PNamedShape wingShape, gp_Vec upDir)
{
    if (_cutterShape) {
        return _cutterShape;
    }
    
    if (needsWingIntersection()) {

        DLOG(INFO) << "Building " << _uid << " cutter shape";

        // Get Wires definng the Shape of the more complex CutOutShape.
        TopoDS_Wire innerWire = innerBorder.getWire(wingShape, upDir);
        TopoDS_Wire outerWire = outerBorder.getWire(wingShape, upDir);

        // make one shape out of the 2 wires and build connections inbetween.
        BRepOffsetAPI_ThruSections thrusections(true,true);
        thrusections.AddWire(outerWire);
        thrusections.AddWire(innerWire);
        thrusections.Build();

        _cutterShape = PNamedShape(new CNamedShape(thrusections.Shape(), _csDevice->GetUID().c_str()));
        _cutterShape->SetShortName(_csDevice->GetShortShapeName().c_str());

        assert(_cutterShape);

#ifdef DEBUG
        std::stringstream filenamestr;
        filenamestr << _uid << "_cutter.brep";
        BRepTools::Write(_cutterShape->Shape(), filenamestr.str().c_str());
#endif
        
        return _cutterShape;
    }
    else {
        return PNamedShape();
    }
}

void CCPACSControlSurfaceDeviceOuterShape::setUID(const std::string& uid)
{
    _uid = uid;
    outerBorder.setUID(uid + "_OuterBorder");
    innerBorder.setUID(uid + "_InnerBorder");
}

bool CCPACSControlSurfaceDeviceOuterShape::needsWingIntersection() const
{
    // TODO: implement
    return true;
}

}

// end namespace tigl
