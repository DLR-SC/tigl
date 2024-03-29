/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-09-03 Martin Siggel <Martin.Siggel@dlr.de>
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

#define _USE_MATH_DEFINES

#include "CCPACSFarField.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

#include "TixiHelper.h"

#include <string>
#include <cmath>

#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

namespace tigl
{

CCPACSFarField::CCPACSFarField()
    : m_referenceLength(0)
    , m_multiplier(0)
{
    init();
}

void CCPACSFarField::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    init();

    // read element type
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/type")) {
        m_type = stringToTiglFarFieldType(tixi::TixiGetElement<std::string>(tixiHandle, xpath + "/type"));
    }
    else {
        LOG(ERROR) << "Required element type is missing at xpath " << xpath;
    }

    // read element referenceLength
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/referenceLength")) {
        m_referenceLength = tixi::TixiGetElement<double>(tixiHandle, xpath + "/referenceLength");
    }
    else {
        LOG(ERROR) << "Required element referenceLength is missing at xpath " << xpath;
    }

    // read element multiplier
    if (tixi::TixiCheckElement(tixiHandle, xpath + "/multiplier")) {
        m_multiplier = tixi::TixiGetElement<double>(tixiHandle, xpath + "/multiplier");
    }
    else {
        LOG(ERROR) << "Required element multiplier is missing at xpath " << xpath;
    }

}

void CCPACSFarField::WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const
{
    const std::vector<std::string> childElemOrder = { "type", "referenceLength", "multiplier" };

    // write element type
    tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/type", childElemOrder);
    tixi::TixiSaveElement(tixiHandle, xpath + "/type", TiglFarFieldTypeToString(m_type));

    // write element referenceLength
    tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/referenceLength", childElemOrder);
    tixi::TixiSaveElement(tixiHandle, xpath + "/referenceLength", m_referenceLength);

    // write element multiplier
    tixi::TixiCreateSequenceElementIfNotExists(tixiHandle, xpath + "/multiplier", childElemOrder);
    tixi::TixiSaveElement(tixiHandle, xpath + "/multiplier", m_multiplier);

}

const TiglFarFieldType& CCPACSFarField::GetType() const
{
    return m_type;
}

void CCPACSFarField::SetType(const TiglFarFieldType& value)
{
    m_type = value;
}

const double& CCPACSFarField::GetReferenceLength() const
{
    return m_referenceLength;
}

void CCPACSFarField::SetReferenceLength(const double& value)
{
    m_referenceLength = value;
}

const double& CCPACSFarField::GetMultiplier() const
{
    return m_multiplier;
}

void CCPACSFarField::SetMultiplier(const double& value)
{
    m_multiplier = value;
}

void CCPACSFarField::init()
{
    SetType(NONE);
    loft.clear();
}

std::string CCPACSFarField::GetDefaultedUID() const {
    return "FarField";
}

PNamedShape CCPACSFarField::BuildLoft() const
{
    const double fieldSize = m_referenceLength * m_multiplier;

    TopoDS_Shape shape;
    gp_Pnt center(0,0,0);

    switch (GetType()) {
    case FULL_SPHERE:
        shape = BRepPrimAPI_MakeSphere(center, fieldSize).Shape();
        break;
    case FULL_CUBE:
        shape = BRepPrimAPI_MakeBox(gp_Pnt(center.X()-fieldSize, center.Y()-fieldSize, center.Z()-fieldSize),
                                    fieldSize*2., fieldSize*2., fieldSize*2.).Shape();
        break;
    case HALF_CUBE:
        shape = BRepPrimAPI_MakeBox(gp_Pnt(center.X()-fieldSize, center.Y(), center.Z()-fieldSize),
                                    fieldSize*2., fieldSize, fieldSize*2.).Shape();
        break;
    case HALF_SPHERE:
        shape = BRepPrimAPI_MakeSphere(gp_Ax2(center, gp_Dir(0,1,0)), fieldSize, 0., M_PI_2).Shape();
        break;
    default:
        shape.Nullify();
    }

    // set names
    std::string loftName = GetDefaultedUID();
    std::string loftShortName = "FF";

    PNamedShape loft(new CNamedShape(shape, loftName.c_str(), loftShortName.c_str()));

    // rename the face trait, which represents the symmetry plane
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(shape,   TopAbs_FACE, map);
    for (int iface = 1; iface <= map.Extent(); ++iface){
        TopoDS_Face face = TopoDS::Face(map(iface));
        gp_Pnt p = GetCentralFacePoint(face);
        if (fabs(p.Y()) < Precision::Confusion()) {
            CFaceTraits traits = loft->GetFaceTraits(iface-1);
            traits.SetName("Symmetry");
            loft->SetFaceTraits(iface-1, traits);
        }
    }
    return loft;
}

TiglGeometricComponentType CCPACSFarField::GetComponentType() const
{
    return TIGL_COMPONENT_FARFIELD;
}

TiglGeometricComponentIntent CCPACSFarField::GetComponentIntent() const
{
    return TIGL_INTENT_LOGICAL;
}


} // namespace tigl

