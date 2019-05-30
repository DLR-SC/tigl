/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-12-04 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleProfile.h"
#include "CCPACSNacelleSection.h"
#include "CCPACSRotationCurve.h"
#include "CTiglUIDManager.h"
#include "Debugging.h"

#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "BRepPrimAPI_MakeRevol.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "Geom_SurfaceOfRevolution.hxx"
#include "BRep_Builder.hxx"

#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"

namespace tigl {

TopoDS_Wire CCPACSRotationCurve::GetCurve() const
{
    CCPACSNacelleProfile& profile = m_uidMgr->ResolveObject<CCPACSNacelleProfile>(GetCurveProfileUID());
    profile.SetPointListAlgoType(CCPACSNacelleProfile::Simple);


    // apply transform of reference section
    CCPACSNacelleSection& section = m_uidMgr->ResolveObject<CCPACSNacelleSection>(GetReferenceSectionUID());
    TopoDS_Wire wire = CutCurveAtZetas(profile.GetWire());
    TopoDS_Shape transformedShape(wire);

    CTiglTransformation trafo = section.GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);

    return TopoDS::Wire(transformedShape);
}

TopoDS_Face CCPACSRotationCurve::GetRotationSurface(gp_Pnt origin, axis dir) const
{
    gp_Vec axis_vec;
    if      ( dir == x ) { axis_vec = gp_Vec(1., 0., 0.); }
    else if ( dir == y ) { axis_vec = gp_Vec(0., 1., 0.); }
    else                 { axis_vec = gp_Vec(0., 0., 1.); }

    gp_Ax1 ax = gp_Ax1(origin, axis_vec);

    TopoDS_Wire wire = GetCurve();

    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(wire, TopAbs_EDGE, map);
    if ( map.Extent() ==0 || map.Extent() > 1 ) {
        CTiglError("CCPACSRotationCurve::GetRotationSurface: Rotation Curve is currently only supported for a single edge.\n", TIGL_ERROR);
    }
    TopoDS_Edge edge = TopoDS::Edge(map(1));

#ifdef DEBUG
    dumpShape(edge, "debugShapes", "rotationalcurve");
#endif


    return TopoDS::Face(BRepPrimAPI_MakeRevol(edge, ax));
}

TopoDS_Wire CCPACSRotationCurve::CutCurveAtZetas(const TopoDS_Wire wire) const
{

    double zeta1 = GetStartZeta();
    double zeta2 = GetEndZeta();

    if ( zeta1 > 0 || zeta2 > 0 || zeta1 < -1 || zeta2 < -1 ) {
        throw CTiglError("CCPACSRotationCurve: StartZetaBlending and EndZetaBlending must be between 0 and -1.");
    }

    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(wire, TopAbs_EDGE, map);
    if ( map.Extent() ==0 || map.Extent() > 1 ) {
        CTiglError("CCPACSRotationCurve::CutCurveAtZetaBlending: Rotation Curve is currently only supported for a single edge.\n", TIGL_ERROR);
    }
    TopoDS_Edge edge = TopoDS::Edge(map(1));

    double umin, umax;
    double par1 = -1;
    double par2 = -1;
    Handle_Geom_Curve curve = BRep_Tool::Curve(edge, umin, umax);
    GeomAdaptor_Curve adaptorCurve(curve, umin, umax);
    Standard_Real len =  GCPnts_AbscissaPoint::Length( adaptorCurve, umin, umax );
    if (len < Precision::Confusion()) {
        throw tigl::CTiglError("CutCurveAtZetaBlending: Unable to cut rotation curve of zero length for CCPACSNacelleCowl", TIGL_MATH_ERROR);
    }
    // cut at startZetaBlending
    if ( zeta1 > -1. ) {
        GCPnts_AbscissaPoint algo1(adaptorCurve, len*(zeta1 + 1.), umin);
        if (algo1.IsDone()) {
            par1 = algo1.Parameter();
        }
        else {
            throw tigl::CTiglError("CutCurveAtZetaBlending: Unable to cut rotation curve for CCPACSNacelleCowl", TIGL_ERROR);
        }
    }
    else {
        par1 = umin;
    }
    // cut at endZetaBlending
    if ( zeta2 < 0 ) {
        GCPnts_AbscissaPoint algo2(adaptorCurve, len*(zeta2 + 1.), umin);
        if (algo2.IsDone()) {
            par2 = algo2.Parameter();
        }
        else {
            throw tigl::CTiglError("CutCurveAtZetaBlending: Unable to cut rotation curve for CCPACSNacelleCowl", TIGL_ERROR);
        }
    }
    else {
        par2 = umax;
    }

    // return trimmed curve
    curve = new Geom_TrimmedCurve(curve, par1,  par2);
    BRepBuilderAPI_MakeWire wireBuilder;
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(curve));
    return wireBuilder.Wire();
}

}
