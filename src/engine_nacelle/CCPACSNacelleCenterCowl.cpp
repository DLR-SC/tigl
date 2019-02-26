/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-02-26 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleCenterCowl.h"
#include "CCPACSNacelleProfile.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"
#include "CTiglUIDManager.h"
#include "CTiglPatchShell.h"

#include <BRep_Builder.hxx>
#include <Precision.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepTools.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

namespace tigl
{

CCPACSNacelleCenterCowl::CCPACSNacelleCenterCowl(CCPACSEngineNacelle* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleCenterCowl(parent, uidMgr)
{}


PNamedShape CCPACSNacelleCenterCowl::BuildLoft() const
{
    // create rotation curve
    gp_Vec axis_vec = gp_Vec(1., 0., 0.);
    gp_Pnt origin = gp_Pnt(0., 0., 0.);
    gp_Ax1 ax = gp_Ax1(origin, axis_vec);

    TopoDS_Wire wire = GetCurve();

    TopTools_IndexedMapOfShape curvemap;
    TopExp::MapShapes(wire, TopAbs_EDGE, curvemap);
    if ( curvemap.Extent() ==0 || curvemap.Extent() > 1 ) {
        CTiglError("CCPACSRotationCurve::GetRotationSurface: Rotation Curve is currently only supported for a single edge.\n", TIGL_ERROR);
    }
    TopoDS_Edge edge = TopoDS::Edge(curvemap(1));

#ifdef DEBUG
    BRepTools::Write(edge,"nacelleCenterCowlCurve.brep");
#endif

    TopoDS_Face face = TopoDS::Face(BRepPrimAPI_MakeRevol(edge, ax));

    // make solid and add front and back cap, if necessary
    CTiglPatchShell patcher(face);

    TopTools_IndexedMapOfShape facemap;
    TopExp::MapShapes(face, TopAbs_EDGE, facemap);
    if ( facemap.Extent() > 1) {
        TopoDS_Edge edge = TopoDS::Edge(facemap(1));
        if (!BRep_Tool::Degenerated(edge)) {
            BRepBuilderAPI_MakeWire builder = BRepBuilderAPI_MakeWire(edge);
            patcher.AddSideCap(builder.Wire());
        }
    }
    if ( facemap.Extent() > 2) {
        TopoDS_Edge edge = TopoDS::Edge(facemap(2));
        if (!BRep_Tool::Degenerated(edge)) {
            BRepBuilderAPI_MakeWire builder = BRepBuilderAPI_MakeWire(edge);
            patcher.AddSideCap(builder.Wire());
        }
    }
    patcher.SetMakeSolid(true);
    TopoDS_Shape solid = patcher.PatchedShape();

    PNamedShape centerCowlShape(new CNamedShape(solid, GetUID().c_str()));
    return centerCowlShape;
}

TopoDS_Wire CCPACSNacelleCenterCowl::GetCurve() const
{
    CCPACSNacelleProfile& profile = m_uidMgr->ResolveObject<CCPACSNacelleProfile>(GetCurveUID());
    profile.SetPointListAlgoType(CCPACSNacelleProfile::Simple);

    // shift curve by XOffset.
    TopoDS_Shape transformedShape(profile.GetWire());
    CTiglTransformation trafo;
    trafo.AddTranslation(GetXOffset(), 0., 0.);
    transformedShape = trafo.Transform(transformedShape);

    return TopoDS::Wire(transformedShape);
}

}
