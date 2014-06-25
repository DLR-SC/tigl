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

#include "CCPACSFarField.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"

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
{
    init();
}

CCPACSFarField::~CCPACSFarField() {}

void CCPACSFarField::init()
{
    fieldType = NONE;
    fieldSize = 0.;
    loft.reset();
    SetUID("FarField");
}

TiglFarFieldType CCPACSFarField::GetFieldType()
{
    return fieldType;
}

void CCPACSFarField::ReadCPACS(TixiDocumentHandle tixiHandle)
{
    init();

    std::string prefix = "/cpacs/toolspecific/cFD/farField";
    if (tixiCheckElement(tixiHandle, prefix.c_str()) != SUCCESS) {
        LOG(INFO) << "No far-field defined.";
        fieldType = NONE;
        return;
    }

    // get field type
    std::string typePath = prefix + "/type";
    char * tmpstr = NULL;
    if (tixiGetTextElement(tixiHandle, typePath.c_str(), &tmpstr) != SUCCESS) {
        fieldType = NONE;
        return;
    }
    else {
        if (strcmp(tmpstr, "halfSphere") == 0){
            fieldType = HALF_SPHERE;
        }
        else if (strcmp(tmpstr, "fullSphere") == 0){
            fieldType = FULL_SPHERE;
        }
        else if (strcmp(tmpstr, "halfCube") == 0){
            fieldType = HALF_CUBE;
        }
        else if (strcmp(tmpstr, "fullCube") == 0){
            fieldType = FULL_CUBE;
        }
        else {
            fieldType = NONE;
            return;
        }
    }

    // get reference length
    std::string refLenPath = prefix + "/referenceLength";
    if (tixiGetDoubleElement(tixiHandle, refLenPath.c_str(), &fieldSize) != SUCCESS) {
        fieldSize = 0.;
        throw tigl::CTiglError("No reference length defined for far-field!");
    }

    // get multiplier
    std::string multiplierPath = prefix + "/multiplier";
    double multiplier = 1.;
    if (tixiGetDoubleElement(tixiHandle, multiplierPath.c_str(), &multiplier) != SUCCESS) {
        fieldSize = 0.;
        throw tigl::CTiglError("No multiplier defined for far-field!");
    }
    else {
        fieldSize *= multiplier;
    }
}

PNamedShape CCPACSFarField::BuildLoft(void)
{
    TopoDS_Shape shape;
    shape.Nullify();
    gp_Pnt center(0,0,0);

    switch (fieldType) {
    case NONE:
        shape.Nullify();
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
    std::string loftName = GetUID();
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

TiglGeometricComponentType CCPACSFarField::GetComponentType(void)
{
    return TIGL_COMPONENT_LOGICAL;
}

} // namespace tigl

