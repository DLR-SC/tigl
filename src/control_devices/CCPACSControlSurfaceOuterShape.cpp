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
/**
 * @file
 * @brief  Implementation of CPACS ...  handling routines.
 */

#include <iostream>
#include <sstream>
#include <exception>

#include "CCPACSControlSurfaceOuterShape.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>


namespace tigl
{

CCPACSControlSurfaceOuterShape::CCPACSControlSurfaceOuterShape()
{

}

// Read CPACS outerShape element
void CCPACSControlSurfaceOuterShape::ReadCPACS(TixiDocumentHandle tixiHandle,
        const std::string& outerShapeXPath, TiglControlSurfaceType type)
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

const CCPACSControlSurfaceBorder& CCPACSControlSurfaceOuterShape::getInnerBorder()
{
    return innerBorder;
}
const CCPACSControlSurfaceBorder& CCPACSControlSurfaceOuterShape::getOuterBorder()
{
    return outerBorder;
}
}

// end namespace tigl
