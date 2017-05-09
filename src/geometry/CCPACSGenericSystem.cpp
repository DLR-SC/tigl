/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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
* @brief  Implementation of CPACS wing handling routines.
*/

#include <iostream>
#include <algorithm>

#include "CCPACSGenericSystem.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include "BRepBuilderAPI_GTransform.hxx"
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

namespace tigl
{

// Constructor
CCPACSGenericSystem::CCPACSGenericSystem(CCPACSConfiguration* config)
    : CTiglRelativelyPositionedComponent(NULL, &transformation, &symmetryAxis), transformation(config ? &config->GetUIDManager() : NULL), configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSGenericSystem::~CCPACSGenericSystem()
{
    Cleanup();
}

const std::string& CCPACSGenericSystem::GetUID() const {
    return uid;
}

void CCPACSGenericSystem::SetUID(const std::string& uid) {
    this->uid = uid;
}

std::string CCPACSGenericSystem::GetDefaultedUID() const {
    return uid;
}

// Invalidates internal state
void CCPACSGenericSystem::Invalidate()
{
    invalidated = true;
}

// Cleanup routine
void CCPACSGenericSystem::Cleanup()
{
    name = "";
    transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Update internal generic system data
void CCPACSGenericSystem::Update()
{
    if (!invalidated) {
        return;
    }

    transformation.updateMatrix();
    invalidated = false;
}

// Read CPACS generic system element
void CCPACSGenericSystem::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& genericSysXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = genericSysXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get subelement "geometricBaseType"
    char* ptrBaseType = NULL;
    tempString    = genericSysXPath + "/geometricBaseType";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrBaseType) == SUCCESS) {
        geometricBaseType = ptrBaseType;

        // check validity of type
        if (geometricBaseType != "cylinder" && geometricBaseType != "sphere" && geometricBaseType != "cone" && geometricBaseType != "cube") {
            throw CTiglError("Invalid geometry base type: " + geometricBaseType, TIGL_XML_ERROR);
        }
    }

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(genericSysXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get Transformation
    transformation.ReadCPACS(tixiHandle, genericSysXPath);

    // Register ourself at the unique id manager
    if (configuration) {
        configuration->GetUIDManager().AddGeometricComponent(ptrUID, this);
    }

    // Get symmetry axis attribute
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(genericSysXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(stringToTiglSymmetryAxis(ptrSym));
    }

    Update();
}

// Returns the name of the generic system
const std::string& CCPACSGenericSystem::GetName() const
{
    return name;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSGenericSystem::GetConfiguration() const
{
    return *configuration;
}

// build loft
PNamedShape CCPACSGenericSystem::BuildLoft()
{
    TopoDS_Shape sysShape;
    if (geometricBaseType == "cylinder") {
        BRepPrimAPI_MakeCylinder cyl(0.5, 1);
        gp_Vec vec(0, 0, -0.5);
        gp_Trsf trsf = gp_Trsf();
        trsf.SetTranslation(vec);
        TopLoc_Location loc(trsf);
        sysShape = cyl.Shape();
        sysShape.Location(loc);
    }
    else if (geometricBaseType == "sphere") {
        BRepPrimAPI_MakeSphere sph(0.5);
        sysShape = sph.Shape();
    }
    else if (geometricBaseType == "cone") {
        BRepPrimAPI_MakeCone cone(0.5, 0, 1);
        gp_Vec vec(0, 0, -0.5);
        gp_Trsf trsf = gp_Trsf();
        trsf.SetTranslation(vec);
        TopLoc_Location loc(trsf);
        sysShape = cone.Shape();
        sysShape.Location(loc);
    }
    else if (geometricBaseType == "cube") {
        gp_Pnt p1(-0.5, -0.5, -0.5);
        gp_Pnt p2(0.5, 0.5, 0.5);
        BRepPrimAPI_MakeBox cube(p1, p2);
        sysShape = cube.Shape();
    }

    sysShape = GetTransformationMatrix().Transform(sysShape);
    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    PNamedShape loft(new CNamedShape(sysShape, loftName.c_str(), loftShortName.c_str()));

    return loft;
}

// get short name for loft
std::string CCPACSGenericSystem::GetShortShapeName()
{
    unsigned int gsindex = 0;
    for (int i = 1; i <= GetConfiguration().GetGenericSystemCount(); ++i) {
        tigl::CCPACSGenericSystem& gs = GetConfiguration().GetGenericSystem(i);
        if (GetUID() == gs.GetUID()) {
            gsindex = i;
            std::stringstream shortName;
            shortName << "GS" << gsindex;
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

// Sets the Transformation object
void CCPACSGenericSystem::Translate(CTiglPoint trans)
{
    CTiglRelativelyPositionedComponent::Translate(trans);
    invalidated = true;
    Update();
}

} // end namespace tigl
