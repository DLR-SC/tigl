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

#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "BRepPrimAPI_MakeRevol.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "Geom_SurfaceOfRevolution.hxx"
#include "BRep_Builder.hxx"

namespace tigl {

TopoDS_Wire CCPACSRotationCurve::GetCurve() const
{
    CCPACSNacelleProfile& profile = m_uidMgr->ResolveObject<CCPACSNacelleProfile>(GetCurveProfileUID());
    profile.SetPointListAlgoType(CCPACSNacelleProfile::Simple);


    // apply transform of reference section
    CCPACSNacelleSection& section = m_uidMgr->ResolveObject<CCPACSNacelleSection>(GetReferenceSectionUID());
    TopoDS_Shape transformedShape(profile.GetWire());
    CTiglTransformation trafo = section.GetTransformationMatrix();
    transformedShape = trafo.Transform(transformedShape);

    return TopoDS::Wire(transformedShape);
}

TopoDS_Face CCPACSRotationCurve::GetRotationSurface(axis dir) const
{


    gp_Vec axis_vec;
    if      ( dir == x ) { axis_vec = gp_Vec(1., 0., 0.); }
    else if ( dir == y ) { axis_vec = gp_Vec(0., 1., 0.); }
    else                 { axis_vec = gp_Vec(0., 0., 1.); }

    gp_Ax1 ax = gp_Ax1(gp_Pnt(0., 0., 0.), axis_vec);

    TopoDS_Wire wire = GetCurve();

    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(wire, TopAbs_EDGE, map);
    if ( map.Extent() ==0 || map.Extent() > 1 ) {
        CTiglError("CCPACSRotationCurve::GetRotationSurface: Rotation Curve is currently only supported for a single edge.\n", TIGL_ERROR);
    }
    TopoDS_Edge edge = TopoDS::Edge(map(1));

#ifdef DEBUG
    BRepTools::Write(edge,"D:/tmp/rotationalcurve.brep");
#endif


    return TopoDS::Face(BRepPrimAPI_MakeRevol(edge, ax));
}

}
