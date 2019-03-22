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

#include "tiglcommonfunctions.h"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"

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

const std::string& tigl::CTiglFuselageSectionElement::GetSectionUID() const
{
    return section->GetUID();
}

const std::string& tigl::CTiglFuselageSectionElement::GetSectionElementUID() const
{
    return element->GetUID();
}

const std::string& tigl::CTiglFuselageSectionElement::GetProfileUID() const
{
    return element->GetProfileUID();
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
tigl::CTiglTransformation tigl::CTiglFuselageSectionElement::GetSectionElementTransformation() const
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

void tigl::CTiglFuselageSectionElement::SetOrigin(const CTiglPoint& newO, TiglCoordinateSystem referenceCS)
{

    CTiglTransformation newE = GetElementTrasformationToTranslatePoint(newO, GetOrigin(referenceCS), referenceCS);

    // set the new transformation matrix in the element
    CCPACSTransformation& storedTransformation = element->GetTransformation();
    storedTransformation.setTransformationMatrix(newE);
    fuselage->Invalidate();
}

void tigl::CTiglFuselageSectionElement::SetCenter(const tigl::CTiglPoint& newCenter, TiglCoordinateSystem referenceCS)
{

    CTiglTransformation newE = GetElementTrasformationToTranslatePoint(newCenter, GetCenter(referenceCS), referenceCS);

    // set the new transformation matrix in the element
    CCPACSTransformation& storedTransformation = element->GetTransformation();
    storedTransformation.setTransformationMatrix(newE);
    fuselage->Invalidate();
}


void tigl::CTiglFuselageSectionElement::ScaleCircumference(double scaleFactor,  TiglCoordinateSystem referenceCS)
{

    if( !( referenceCS == GLOBAL_COORDINATE_SYSTEM || referenceCS == FUSELAGE_COORDINATE_SYSTEM) )  {
        throw tigl::CTiglError("CTiglFuselageSectionElement::ScaleCircumference: Invalid coordinate system");
    }

    CTiglTransformation newE = GetElementTransformationToScaleCircumference(scaleFactor);

    CCPACSTransformation& storedTransformation = element->GetTransformation();
    storedTransformation.setTransformationMatrix(newE);

    fuselage->Invalidate();
}