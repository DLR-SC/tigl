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

#include "CCPACSNacelleCowl.h"
#include "CCPACSNacelleSection.h"
#include "generated/CPACSNacelleGuideCurve.h"
#include "CTiglNacelleGuideCurveBuilder.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"
#include "Debugging.h"

#include "BRepTools.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepClass3d_SolidClassifier.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRep_Tool.hxx"

#include "TopTools_IndexedMapOfShape.hxx"
#include "TopExp.hxx"
#include "CWireToCurve.h"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "GeomFill.hxx"
#include "BRepPrimAPI_MakeRevol.hxx"


namespace {
void RemoveBlendingPart(TopoDS_Edge const lowerEdge,
                        double zeta1,
                        double zeta2,
                        TopoDS_Edge& edge1,
                        TopoDS_Edge& edge2);

std::vector<std::pair<double,TopoDS_Wire>> connectNacelleGuideCurves(std::vector<std::pair<double,TopoDS_Wire>> const&,
                                                                     double tol);

Handle(Geom_Curve) GetBoundaryCurveByIndex(TopoDS_Shape& face, int edgeIndex);

}

namespace tigl
{

CCPACSNacelleCowl::CCPACSNacelleCowl(CCPACSEngineNacelle* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleCowl(parent, uidMgr)
    , wireCache(*this, &CCPACSNacelleCowl::BuildOuterShapeWires)
{}

void CCPACSNacelleCowl::BuildOuterShapeWires(WireCache& cache) const
{
    double startZetaBlending = m_rotationCurve.GetStartZetaBlending();
    double endZetaBlending   = m_rotationCurve.GetEndZetaBlending();

    // build profile wires
    for(size_t i = 1; i <= m_sections.GetSectionCount(); ++i ) {

        CCPACSNacelleSection& section = m_sections.GetSection(i);

        BRepBuilderAPI_MakeWire builder;
        TopoDS_Edge lowerEdge = section.GetTransformedLowerWire();
        TopoDS_Edge lower1, lower2;

        // remove blending part for rotationally symmetric interior
        RemoveBlendingPart(lowerEdge, startZetaBlending, endZetaBlending, lower1, lower2);

        builder.Add(lower2);
        TopoDS_Edge upperEdge = section.GetTransformedUpperWire();
        builder.Add(upperEdge);
        if (section.GetProfile().HasBluntTE()) {
            TopoDS_Edge trailingEdge = section.GetTransformedTrailingEdge();
            builder.Add(trailingEdge);
        }
        builder.Add(lower1);
        TopoDS_Wire profileWire = builder.Wire();

        cache.profiles.push_back(profileWire);
    }

    if ( cache.profiles.size() == 1 ) {
        // there is only one profile. No need to build guide curves, nacelle will
        // be a rotation surface
        return;
    }

    // build guide curve wires
    // guide curves for some zeta values MUST be present, even if not defined via CPACS
    double requiredZeta[5] =      {endZetaBlending, 0, 1, -1, startZetaBlending};
    bool   buildRequiredZeta[5] = {true, true, true, true, true};
    if ( m_sections.GetSectionCount()>0 && !m_sections.GetSection(1).GetProfile().HasBluntTE() ) {
        // upper trailing edge guide curve not needed for sharp profiles
        // Here it is assumed that either ALL profiles are sharp or ALL profiles are blunt
        buildRequiredZeta[2] = false;
    }

    // get CPACS guide curves
    std::vector<std::pair<double,TopoDS_Wire>> zetaGuides;
    if ( m_guideCurves ) {
        for(size_t i = 1; i <= m_guideCurves->GetGuideCurveCount(); ++i ) {
            const CCPACSNacelleGuideCurve& guide = m_guideCurves->GetGuideCurve(i);

            // check if it is one of the required curves
            for (int i = 0; i<5; ++i) {
                if(    fabs(guide.GetFromZeta() - requiredZeta[i]) < Precision::Confusion()
                    && fabs(guide.GetToZeta()   - requiredZeta[i]) < Precision::Confusion() ) {
                    // if the required guide curve is already defined via CPACS, it must not explicitly be built.
                    buildRequiredZeta[i] = false; }
            }

            CTiglNacelleGuideCurveBuilder gcbuilder(guide);
            std::pair<double,TopoDS_Wire> zetaGuidePair(guide.GetFromZeta(), gcbuilder.GetWire());
            zetaGuides.push_back(zetaGuidePair);
        }
    }

    // explicitly built the guide curves for the required zeta values, that are not defined in CPACS
    for (int i = 0; i<5; i++) {
        if ( buildRequiredZeta[i] ) {
            for ( size_t j = 1; j <= m_sections.GetSectionCount(); ++j ) {
                NacelleGuideCurveParameters params;
                params.fromZeta    = requiredZeta[i];
                params.toZeta      = requiredZeta[i];
                params.fromSection = &m_sections.GetSection(j);

                size_t k = j+1;
                if ( k > m_sections.GetSectionCount() ) {
                    k = 1;
                }
                params.toSection = &m_sections.GetSection(k);
                CTiglNacelleGuideCurveBuilder gcbuilder(params);
                std::pair<double,TopoDS_Wire> zetaGuidePair(params.fromZeta, gcbuilder.GetWire());
                zetaGuides.push_back(zetaGuidePair);
            }
        }
    }
    cache.guideCurves = connectNacelleGuideCurves(zetaGuides, Precision::Confusion());

}

TopoDS_Shape CCPACSNacelleCowl::BuildOuterShape() const
{
    if ( wireCache->profiles.size()< 1 ) {
        throw CTiglError("CCPACSNacelleCowl::BuildOuterShape: Unable to create nacelle geometry for nacelle without any profile curves.");
    }

    if ( wireCache->profiles.size() == 1 ) {
        // there is only one profile. nacelle will be rotation curve.
        gp_Ax1 ax = gp_Ax1(gp_Pnt(0., 0., 0.), gp_Vec(1., 0., 0.));

        TopoDS_Shell shell;
        BRep_Builder shellBuilder;
        shellBuilder.MakeShell(shell);

        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(wireCache->profiles[0], TopAbs_EDGE, edgeMap);
        for (int e = 1; e <= edgeMap.Extent(); e++) {
            TopoDS_Edge edge = TopoDS::Edge(edgeMap(e));
            TopoDS_Face current = TopoDS::Face(BRepPrimAPI_MakeRevol(edge, ax));
            shellBuilder.Add(shell, current);
        }
        return shell;
    }

    CTiglMakeLoft lofter;
    lofter.setMakeSmooth(true);
    lofter.setMakeSolid(false);

    // get profile curves
    for ( size_t i = 0; i<wireCache->profiles.size(); ++i) {

#ifdef DEBUG
        dumpShape(wireCache->profiles[i], "debugShapes", "nacelleProfile", i);
#endif

        lofter.addProfiles(wireCache->profiles[i]);
    }

    // get guide curves
    for(size_t i=0; i<wireCache->guideCurves.size(); ++i) {
#ifdef DEBUG
        dumpShape(wireCache->guideCurves[i].second, "debugShapes", "nacelleGuide", i);
#endif
        lofter.addGuides(wireCache->guideCurves[i].second);
    }

    return  lofter.Shape();
}

PNamedShape CCPACSNacelleCowl::BuildLoft() const
{
    TopoDS_Shell shell;
    BRep_Builder shellBuilder;
    shellBuilder.MakeShell(shell);

    // get nacelle origin
    gp_Pnt origin(0., 0., 0.);

    // get shapes of nacelle cowls
    TopoDS_Shape outerShape    = BuildOuterShape();
    TopoDS_Face  innerShape    = m_rotationCurve.GetRotationSurface(origin);

#ifdef DEBUG
    dumpShape(outerShape, "debugShapes", "outerShape");
    dumpShape(innerShape, "debugShapes", "innerShape");
#endif

    TopoDS_Face  blendingSurf1 = GetStartZetaBlendingSurface(innerShape, outerShape);
    TopoDS_Face  blendingSurf2 = GetEndZetaBlendingSurface(innerShape, outerShape);

#ifdef DEBUG
    dumpShape(blendingSurf1, "debugShapes", "blendingSurf1");
    dumpShape(blendingSurf1, "debugShapes", "blendingSurf2");
#endif

    // add subshapes of outerShape to shell
    for(TopExp_Explorer e(outerShape, TopAbs_FACE); e.More(); e.Next()) {
        shellBuilder.Add(shell, TopoDS::Face( e.Current() ) );
    }

    // add blending surfaces
    shellBuilder.Add(shell, blendingSurf1);
    shellBuilder.Add(shell, blendingSurf2);

    // add rotationally symmetric interior
    shellBuilder.Add(shell, innerShape);

    // create a solid from the shell
    BRep_Builder solidBuilder;
    TopoDS_Solid solid;
    solidBuilder.MakeSolid(solid);
    try {
        solidBuilder.Add(solid, shell);
    }
    catch ( ... ) {
        throw CTiglError("Cannot make a solid out of the shell. Is the base type correct?", TIGL_ERROR);
    }

    // verify the orientation of the solid
    BRepClass3d_SolidClassifier clas3d(solid);
    clas3d.PerformInfinitePoint(Precision::Confusion());
    if (clas3d.State() == TopAbs_IN) {
        solidBuilder.MakeSolid(solid);
        TopoDS_Shape aLocalShape = shell.Reversed();
        solidBuilder.Add(solid, TopoDS::Shell(aLocalShape));
    }

    PNamedShape nacelleShape(new CNamedShape(solid, GetUID().c_str()));
    return nacelleShape;
}

TopoDS_Face CCPACSNacelleCowl::GetStartZetaBlendingSurface(TopoDS_Face& innerShape, TopoDS_Shape& outerShape) const
{
    // get curve on outer shape
    Handle(Geom_Curve) curve1;
    if ( wireCache->profiles.size()==1 ){
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(outerShape, TopAbs_EDGE, map);
        assert(map.Extent()-1>0);
        curve1 = GetBoundaryCurveByIndex(outerShape, map.Extent()-1);
    }
    else {
        double zeta   = m_rotationCurve.GetStartZetaBlending();
        curve1 = GetGuideCurve(zeta);
    }

    // get curve on inner shape
    Handle(Geom_Curve) curve2 = GetBoundaryCurveByIndex(innerShape, 1);

    // create the filling
    Handle(Geom_Surface) surface = GeomFill::Surface(curve1, curve2);
    BRepBuilderAPI_MakeFace faceMaker(surface, 1e-10);

    return faceMaker.Face();
}

TopoDS_Face CCPACSNacelleCowl::GetEndZetaBlendingSurface(TopoDS_Face& innerShape, TopoDS_Shape& outerShape) const
{
    // get curve on outer shape
    Handle(Geom_Curve) curve1;
    if ( wireCache->profiles.size()==1 ){
        curve1 = GetBoundaryCurveByIndex(outerShape, 1);
    }
    else {
        double zeta   = m_rotationCurve.GetEndZetaBlending();
        curve1 = GetGuideCurve(zeta);
    }

    // get curve on inner shape
    Handle(Geom_Curve) curve2 = GetBoundaryCurveByIndex(innerShape, 2);

    // create the filling
    Handle(Geom_Surface) surface = GeomFill::Surface(curve1, curve2);
    BRepBuilderAPI_MakeFace faceMaker(surface, 1e-10);

    return faceMaker.Face();
}

Handle(Geom_Curve) CCPACSNacelleCowl::GetGuideCurve(double zeta) const
{
    size_t i = 0;
    while (    i<wireCache->guideCurves.size()
            && fabs(wireCache->guideCurves[i].first - zeta) > Precision::Confusion() ) {
        ++i;
    }
    if ( i == wireCache->guideCurves.size() ) {
        throw CTiglError("Something went wrong: There is no guide curve at endZetaBlending parameter!");
    }

    return CWireToCurve(wireCache->guideCurves[i].second).curve();
}

} //namespace tigl

namespace {

// cut edge into three parts at zeta1, zeta2 and output everything but the middle part
void RemoveBlendingPart(TopoDS_Edge const lowerEdge,
                        double zeta1,
                        double zeta2,
                        TopoDS_Edge& edge1,
                        TopoDS_Edge& edge2)
{
    if ( zeta1 < -1 || zeta1 > 0. || zeta2 < -1 || zeta2 > 0. ){
        tigl::CTiglError("RemoveBlendingPart: Cannot trim lower profile curve for CCPACSNacelleCowl. startZetaBlending and endZetaBlending must be between -1 and 0.\n",TIGL_MATH_ERROR);
    }

    double umin, umax;
    double par1 = -1.;
    double par2 = 0.;
    Handle_Geom_Curve curve = BRep_Tool::Curve(lowerEdge, umin, umax);
    GeomAdaptor_Curve adaptorCurve(curve, umin, umax);
    Standard_Real len =  GCPnts_AbscissaPoint::Length( adaptorCurve, umin, umax );
    if (len < Precision::Confusion()) {
        throw tigl::CTiglError("RemoveBlendingPart: Unable to cut lower profile of zero length for CCPACSNacelleCowl", TIGL_MATH_ERROR);
    }
    GCPnts_AbscissaPoint algo1(adaptorCurve, len*(zeta1 + 1.), umin);
    if (algo1.IsDone()) {
        par1 = algo1.Parameter();
    }
    else {
        throw tigl::CTiglError("RemoveBlendingPart: Unable to cut lower profile for CCPACSNacelleCowl", TIGL_ERROR);
    }
    GCPnts_AbscissaPoint algo2(adaptorCurve, len*(zeta2 + 1.), umin);
    if (algo2.IsDone()) {
        par2 = algo2.Parameter();
    }
    else {
        throw tigl::CTiglError("RemoveBlendingPart: Unable to cut lower profile for CCPACSNacelleCowl", TIGL_ERROR);
    }
    curve = new Geom_TrimmedCurve(curve, umin,  par1);
    edge1 = BRepBuilderAPI_MakeEdge(curve);

    curve = new Geom_TrimmedCurve(curve, par2,  umax);
    edge2 = BRepBuilderAPI_MakeEdge(curve);
}

std::vector<std::pair<double,TopoDS_Wire>> connectNacelleGuideCurves(std::vector<std::pair<double,TopoDS_Wire>> const& zetaGuides,
                                                                     double tol)
{
    // connect guide curves of same zeta parameters
    std::vector<std::pair<double,TopoDS_Wire>> connectedZetaWires;

    std::vector<bool> used(zetaGuides.size(), false);
    for(size_t i = 0; i<zetaGuides.size(); i++) {
        if( !used[i] ) {
            BRepBuilderAPI_MakeWire builder;
            builder.Add(zetaGuides[i].second);
            for(size_t j=0; j != zetaGuides.size(); j++) {
                if (j != i && !used[j] && fabs(zetaGuides[j].first-zetaGuides[i].first) < tol ) {
                    builder.Add(zetaGuides[j].second);
                    used[j] = true;
                }
            }
            used[i] = true;
            std::pair<double,TopoDS_Wire> current(zetaGuides[i].first, builder.Wire());
            connectedZetaWires.push_back(current);
        }
    }

    return connectedZetaWires;
}

Handle(Geom_Curve) GetBoundaryCurveByIndex(TopoDS_Shape& face, int edgeIndex)
{
    TopTools_IndexedMapOfShape map;
    TopExp::MapShapes(face, TopAbs_EDGE, map);
    Standard_Real umin, umax;

    return BRep_Tool::Curve(TopoDS::Edge(map(edgeIndex)), umin, umax);
}

} //anonymous namespace
