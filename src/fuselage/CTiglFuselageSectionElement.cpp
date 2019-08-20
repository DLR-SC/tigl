/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglFuselageSectionElement.h"
#include "CCPACSFuselageSectionElement.h"
#include "CCPACSFuselageSectionElements.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSFuselageSections.h"
#include "CCPACSFuselage.h"
#include "CCPACSConfiguration.h"
#include "tiglmathfunctions.h"

#include "tiglcommonfunctions.h"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepBndLib.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "Bnd_Box.hxx"

tigl::CTiglFuselageSectionElement::CTiglFuselageSectionElement()
    : CTiglSectionElement()
{
    element  = nullptr;
    section  = nullptr;
    fuselage = nullptr;
}

tigl::CTiglFuselageSectionElement::CTiglFuselageSectionElement(tigl::CCPACSFuselageSectionElement* inElement)
    : CTiglSectionElement()
{
    element  = inElement;
    section  = element->GetParent()->GetParent();
    fuselage = section->GetParent()->GetParent();
}


bool tigl::CTiglFuselageSectionElement::IsValid() const
{
    if (element != nullptr && section != nullptr && fuselage != nullptr) {
        return true;
    }
    return false;
}

std::string tigl::CTiglFuselageSectionElement::GetSectionUID() const
{
    return section ? section->GetUID() : "";
}

std::string tigl::CTiglFuselageSectionElement::GetSectionElementUID() const
{
    return element ? element->GetUID() : "";
}

std::string tigl::CTiglFuselageSectionElement::GetProfileUID() const
{
    return element ? element->GetProfileUID() : "";
}

// Returns the fuselage profile referenced by this connection
tigl::CCPACSFuselageProfile& tigl::CTiglFuselageSectionElement::GetProfile()
{

    CCPACSConfiguration& config = fuselage->GetConfiguration();
    return (config.GetFuselageProfile(GetProfileUID()));
}

const tigl::CCPACSFuselageProfile& tigl::CTiglFuselageSectionElement::GetProfile() const
{
    return const_cast<CTiglFuselageSectionElement&>(*this).GetProfile();
}

// Returns the positioning transformation for the referenced section
tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetPositioningTransformation() const
{
    boost::optional<CTiglTransformation> transformation = fuselage->GetPositioningTransformation(section->GetUID());
    if (transformation) {
        return transformation.value();
    }
    else {
        return CTiglTransformation();
    }
}

// Returns the section matrix referenced by this connection
tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetSectionTransformation() const
{
    return section->GetTransformation().getTransformationMatrix();
}

// Returns the section element matrix referenced by this connection
tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetElementTransformation() const
{
    return element->GetTransformation().getTransformationMatrix();
}

tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetParentTransformation() const
{
    return GetFuselageTransformation();
}

tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetFuselageTransformation() const
{

    return fuselage->GetTransformation().getTransformationMatrix();
}

TopoDS_Wire tigl::CTiglFuselageSectionElement::GetWire(TiglCoordinateSystem referenceCS) const
{
    const CCPACSFuselageProfile& profile = GetProfile();
    TopoDS_Wire wire                     = profile.GetWire(true);

    TopoDS_Shape transformedWire = GetTotalTransformation(referenceCS).Transform(wire);

    // Cast shapes to wires, see OpenCascade documentation
    if (transformedWire.ShapeType() != TopAbs_WIRE) {
        throw tigl::CTiglError("Wrong shape type in CCPACSFuselageSegment::transformProfileWire", TIGL_ERROR);
    }

    return TopoDS::Wire(transformedWire);
}


tigl::CTiglPoint tigl::CTiglFuselageSectionElement::GetNormal(TiglCoordinateSystem referenceCS) const
{
    // We assume that the original profile is in the YZ plan.
    // -> So the normal of the final section can be computed by the cross product of the transformed basis vector
    // Remark: do not forget that the CTiglTransformation is augmented, so we need to compute the new Y andZ vector
    // using the origin

    CTiglPoint yPoint = CTiglPoint(0, 1, 0);
    CTiglPoint zPoint = CTiglPoint(0, 0, 1);
    CTiglPoint origin = CTiglPoint(0, 0, 0);

    CTiglTransformation M = GetTotalTransformation(referenceCS);
    CTiglPoint yVec = (M * yPoint) - (M * origin);
    CTiglPoint zVec = (M * zPoint) - (M * origin);

    CTiglPoint normal = CTiglPoint::cross_prod(yVec, zVec);
    normal.normalize();
    return normal;
}

tigl::CCPACSTransformation& tigl::CTiglFuselageSectionElement::GetElementCCPACSTransformation()
{
    return element->GetTransformation();
}

tigl::CCPACSTransformation& tigl::CTiglFuselageSectionElement::GetSectionCCPACSTransformation()
{
    return section->GetTransformation();
}

void tigl::CTiglFuselageSectionElement::InvalidateParent()
{
    fuselage->Invalidate();
}

tigl::CTiglPoint tigl::CTiglFuselageSectionElement::GetStdDirForProfileUnitZ(TiglCoordinateSystem referenceCS)  const
{
    CTiglPoint normal = GetNormal(referenceCS);
    CTiglPoint stdUZDir;
    if (isNear(normal.x, 0) && isNear(normal.z,0)) {
        // in this case the profile is on the XZ plane -> we put the unit z parallel to z
        stdUZDir = CTiglPoint(0,0,1);
    }
    else if (isNear(normal.x, 0)) {
        // in this case we can not put uZ on the line defined by (x,0,1)
        stdUZDir = CTiglPoint(1,0,0);
    }
    else {
        stdUZDir = CTiglPoint( -normal.z/normal.x,0,1);
    }
    stdUZDir.normalize();
    return stdUZDir;
}

tigl::CCPACSPositionings& tigl::CTiglFuselageSectionElement::GetPositionings()
{
    return fuselage->GetPositionings(CreateIfNotExistsTag());
}
