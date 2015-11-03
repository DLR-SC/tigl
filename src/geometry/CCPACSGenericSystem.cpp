/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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

#include "CCPACSGenericSystem.h"
#include "CCPACSConfiguration.h"
#include "CTiglError.h"
#include "tiglcommonfunctions.h"

#include "BRepBuilderAPI_GTransform.hxx"
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

namespace tigl
{

namespace
{
    inline double max(double a, double b)
    {
        return a > b? a : b;
    }
}


// Constructor
CCPACSGenericSystem::CCPACSGenericSystem(CCPACSConfiguration* config)
    : configuration(config)
{
    Cleanup();
}

// Destructor
CCPACSGenericSystem::~CCPACSGenericSystem(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSGenericSystem::Invalidate(void)
{
    invalidated = true;
}

// Cleanup routine
void CCPACSGenericSystem::Cleanup(void)
{
    name = "";
    transformation.reset();

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Update internal generic system data
void CCPACSGenericSystem::Update(void)
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
    configuration->GetUIDManager().AddUID(ptrUID, this);

    // Get symmetry axis attribute
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(genericSysXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }

    Update();
}

// Returns the name of the generic system
const std::string& CCPACSGenericSystem::GetName(void) const
{
    return name;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSGenericSystem::GetConfiguration(void) const
{
    return *configuration;
}

// build loft
PNamedShape CCPACSGenericSystem::BuildLoft()
{
    TopoDS_Shape sysShape;
    if (geometricBaseType == "cylinder") {
        BRepPrimAPI_MakeCylinder cyl(0.5, 1);
        sysShape = cyl.Shape();
    }
    else if (geometricBaseType == "sphere") {
        BRepPrimAPI_MakeSphere sph(0.5);
        sysShape = sph.Shape();
    }
    else if (geometricBaseType == "cone") {
        BRepPrimAPI_MakeCone cone(0, 0.5, 1);
        sysShape = cone.Shape();
    }
    else if (geometricBaseType == "prism") {
        BRepBuilderAPI_MakePolygon polygon;
        polygon.Add(gp_Pnt(-0.5, -0.5, 0));
        polygon.Add(gp_Pnt(0.5, -0.5, 0));
        polygon.Add(gp_Pnt(0.5, 0.5, 0));
        polygon.Add(gp_Pnt(-0.5, 0.5, 0));
        polygon.Close();
        TopoDS_Wire wire = polygon.Wire();
        TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);
        gp_Vec v(0, 0, 1);
        BRepPrimAPI_MakePrism prism(face, v);
        sysShape = prism.Shape();
    }


    sysShape = GetTransformation().Transform(sysShape);
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

// Get the Transformation object (general interface implementation)
CTiglTransformation CCPACSGenericSystem::GetTransformation(void)
{
    return transformation.getTransformationMatrix();
}

// Sets the Transformation object
void CCPACSGenericSystem::Translate(CTiglPoint trans)
{
    CTiglAbstractGeometricComponent::Translate(trans);
    invalidated = true;
    Update();
}

// sets the symmetry plane for all childs, segments and component segments
void CCPACSGenericSystem::SetSymmetryAxis(const std::string& axis)
{
    CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);
}

} // end namespace tigl
