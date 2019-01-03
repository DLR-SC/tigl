/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleSection.h"
#include "CCPACSNacelleProfile.h"
#include "CTiglUIDManager.h"
#include "tiglcommonfunctions.h"

namespace tigl {

TIGL_EXPORT CCPACSNacelleSection::CCPACSNacelleSection(CTiglUIDManager* uidMgr)
   : generated::CPACSNacelleSection(uidMgr)
{};

TIGL_EXPORT const CCPACSNacelleProfile& CCPACSNacelleSection::GetProfile() const
{
    return m_uidMgr->ResolveObject<CCPACSNacelleProfile>(m_profileUID);
}

TIGL_EXPORT TopoDS_Wire CCPACSNacelleSection::GetTransformedWire() const
{
    // apply polar transformation
    TopoDS_Shape transformedShape(GetProfile().GetWire());
    CTiglTransformation trafo = GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);
    return TopoDS::Wire(transformedShape);
}

TIGL_EXPORT TopoDS_Edge CCPACSNacelleSection::GetTransformedUpperWire() const
{
    // apply polar transformation
    TopoDS_Shape transformedShape(GetProfile().GetUpperWire());
    CTiglTransformation trafo = GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);
    return TopoDS::Edge(transformedShape);
}

TIGL_EXPORT TopoDS_Edge CCPACSNacelleSection::GetTransformedLowerWire() const
{
    // apply polar transformation
    TopoDS_Shape transformedShape(GetProfile().GetLowerWire());
    CTiglTransformation trafo = GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);
    return TopoDS::Edge(transformedShape);
}

TIGL_EXPORT TopoDS_Edge CCPACSNacelleSection::GetTransformedTrailingEdge() const
{
    // apply polar transformation
    TopoDS_Shape transformedShape(GetProfile().GetTrailingEdge());
    CTiglTransformation trafo = GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);
    return TopoDS::Edge(transformedShape);
}

// TODO: 2D Polar coordinates cannot be used to place things in 3D space. CCPACSTransformationPolar
// should not be used anywhere but here.
TIGL_EXPORT CTiglTransformation CCPACSNacelleSection::GetTransformationMatrix() const
{
    CTiglTransformation out;

    // get r and phi from translation
    double radius = 0;
    double phi    = 0;
    if ( m_transformation.GetTranslation() ) {
        if (m_transformation.GetTranslation()->GetR()) { radius = m_transformation.GetTranslation()->GetR().get();   }
        if (m_transformation.GetTranslation()->GetR()) { phi    = m_transformation.GetTranslation()->GetPhi().get(); }
    }

    // rotate from XY-plane to XZ-plane
    out.AddRotationX(90.);

    // apply scaling
    if ( m_transformation.GetScaling() ) {
        tigl::CTiglPoint scale = m_transformation.GetScaling()->AsPoint();
        out.AddScaling(scale.x, scale.y, scale.z);
    }

    // apply rotation
    if ( m_transformation.GetRotation() ) {
        tigl::CTiglPoint rotation = m_transformation.GetRotation()->AsPoint();
        out.AddRotationX(rotation.x);
        out.AddRotationY(rotation.y);
        out.AddRotationZ(rotation.z);
    }

    // apply translation (and rotate the profile accordingly)
    out.AddRotationX(phi);
    out.AddTranslation(0,-radius*sin(Radians(phi)),radius*cos(Radians(phi)));

    return out;
}

} //namepsace tigl
