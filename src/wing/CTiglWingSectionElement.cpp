/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglWingSectionElement.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSWingSectionElements.h"
#include "CCPACSWingSection.h"
#include "CPACSWingSections.h"
#include "CCPACSWingSections.h"
#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "tiglmathfunctions.h"

#include "tiglcommonfunctions.h"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepBndLib.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "Bnd_Box.hxx"

tigl::CTiglWingSectionElement::CTiglWingSectionElement()
        : CTiglSectionElement()
{
    element  = nullptr;
    section  = nullptr;
    wing = nullptr;
}

tigl::CTiglWingSectionElement::CTiglWingSectionElement(tigl::CCPACSWingSectionElement* inElement)
    : CTiglSectionElement()
{
    this->SetAssociateElement(inElement);
}

void tigl::CTiglWingSectionElement::SetAssociateElement(tigl::CCPACSWingSectionElement* element)
{
    this->element = element;
    section       = element->GetParent()->GetParent();
    wing          = section->GetParent()->GetParent<CCPACSWing>();
}

std::string tigl::CTiglWingSectionElement::GetSectionUID() const
{
    return section ? section->GetUID() : "" ;
}

std::string tigl::CTiglWingSectionElement::GetSectionElementUID() const
{
    return element ? element->GetUID() : "" ;
}

std::string tigl::CTiglWingSectionElement::GetProfileUID() const
{
    return element ? element->GetAirfoilUID() : "";
}

// Returns the Wing profile referenced by this connection
tigl::CCPACSWingProfile& tigl::CTiglWingSectionElement::GetProfile()
{
    CCPACSConfiguration& config = wing->GetConfiguration();
    return (config.GetWingProfile(GetProfileUID()));
}

const tigl::CCPACSWingProfile& tigl::CTiglWingSectionElement::GetProfile() const
{
    return const_cast<CTiglWingSectionElement&>(*this).GetProfile();
}

// Returns the positioning transformation for the referenced section
tigl::CTiglTransformation tigl::CTiglWingSectionElement::GetPositioningTransformation() const
{
    boost::optional<CTiglTransformation> transformation = wing->GetPositioningTransformation(section->GetUID());
    if (transformation) {
        return transformation.value();
    }
    else {
        return CTiglTransformation();
    }
}

// Returns the section matrix referenced by this connection
tigl::CTiglTransformation tigl::CTiglWingSectionElement::GetSectionTransformation() const
{
    return section->GetTransformation().getTransformationMatrix();
}

// Returns the section element matrix referenced by this connection
tigl::CTiglTransformation tigl::CTiglWingSectionElement::GetElementTransformation() const
{
    return element->GetTransformation().getTransformationMatrix();
}

tigl::CTiglTransformation tigl::CTiglWingSectionElement::GetParentTransformation() const
{
    return GetWingTransformation();
}

tigl::CTiglTransformation tigl::CTiglWingSectionElement::GetWingTransformation() const
{

    return wing->GetTransformation().getTransformationMatrix();
}

TopoDS_Wire tigl::CTiglWingSectionElement::GetWire(TiglCoordinateSystem referenceCS) const
{
    const CCPACSWingProfile& profile = GetProfile();
    TopoDS_Wire wire                     = profile.GetWire();

    TopoDS_Shape transformedWire = GetTotalTransformation(referenceCS).Transform(wire);

    // Cast shapes to wires, see OpenCascade documentation
    if (transformedWire.ShapeType() != TopAbs_WIRE) {
        throw tigl::CTiglError("Wrong shape type in CCPACSWingSegment::transformProfileWire", TIGL_ERROR);
    }

    return TopoDS::Wire(transformedWire);
}


tigl::CTiglPoint tigl::CTiglWingSectionElement::GetNormal(TiglCoordinateSystem referenceCS) const
{
    // We assume that the original profile is in the XZ plan.
    // -> So the normal of the final section can be computed by the cross product of the transformed basis vector
    // Remark: do not forget that the CTiglTransformation is augmented, so we need to compute the new Y andZ vector
    // using the origin

    CTiglPoint xPoint = CTiglPoint(1,0,0);
    CTiglPoint zPoint = CTiglPoint(0,0,1);
    CTiglPoint origin = CTiglPoint(0,0,0);

    CTiglTransformation M = GetTotalTransformation(referenceCS);
    CTiglPoint xVec = (M * xPoint) - (M * origin);
    CTiglPoint zVec = (M * zPoint) - (M * origin);

    CTiglPoint normal = CTiglPoint::cross_prod(xVec, zVec);
    normal.normalize();
    return normal;

}

bool tigl::CTiglWingSectionElement::IsValid() const
{
    if (element != nullptr && section != nullptr && wing != nullptr) {
        return true;
    }
    return false;
}

tigl::CTiglPoint tigl::CTiglWingSectionElement::GetChordPoint(double xsi, TiglCoordinateSystem referenceCS) const
{
    CTiglPoint airfoilChordPoint(GetProfile().GetChordPoint(xsi).XYZ());
    return GetTotalTransformation(referenceCS) * airfoilChordPoint;
}

void tigl::CTiglWingSectionElement::InvalidateParent()
{
    wing->Invalidate();
}

tigl::CCPACSTransformation& tigl::CTiglWingSectionElement::GetElementCCPACSTransformation()
{
    return element->GetTransformation();
}

tigl::CCPACSTransformation& tigl::CTiglWingSectionElement::GetSectionCCPACSTransformation()
{
    return section->GetTransformation();
}

tigl::CTiglPoint tigl::CTiglWingSectionElement::GetStdDirForProfileUnitZ(TiglCoordinateSystem referenceCS) const
{
    CTiglPoint normal = GetNormal(referenceCS);
    CTiglPoint stdUZDir;
    if (isNear(normal.y, 0) && isNear(normal.z,0)) {
        // in this case the profile is on the YZ plane -> we put the unit z parallel to z
        stdUZDir = CTiglPoint(0,0,1);
    }
    else if (isNear(normal.y, 0)) {
        // in this case we can not put uZ on the line defined by (x,0,1)
        stdUZDir = CTiglPoint(0,1,0);
    }
    else {
        stdUZDir = CTiglPoint( 0,-normal.z/normal.y,1);
    }
    stdUZDir.normalize();
    return stdUZDir;
}

tigl::CCPACSPositionings& tigl::CTiglWingSectionElement::GetPositionings()
{
    return wing->GetPositionings(CreateIfNotExistsTag());
}
