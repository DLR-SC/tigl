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

#include "BRepTools.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRep_Tool.hxx"

namespace {
void RemoveBlendingPart(TopoDS_Edge const lowerEdge,
                        double zeta1,
                        double zeta2,
                        TopoDS_Edge& edge1,
                        TopoDS_Edge& edge2);

std::vector<std::pair<double,TopoDS_Wire>> connectNacelleGuideCurves(std::vector<std::pair<double,TopoDS_Wire>> const&,
                                                                     double tol);
}

namespace tigl
{

CCPACSNacelleCowl::CCPACSNacelleCowl(CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleCowl(uidMgr)
    , CTiglAbstractGeometricComponent()
{}

std::string CCPACSNacelleCowl::GetDefaultedUID() const
{
    return generated::CPACSNacelleCowl::GetUID();
}

TopoDS_Shape CCPACSNacelleCowl::BuildOuterShape() const
{
    CTiglMakeLoft lofter;
    lofter.setMakeSmooth(true);
    lofter.setMakeSolid(false);

    double startZetaBlending = m_rotationCurve.GetStartZetaBlending();
    double endZetaBlending   = m_rotationCurve.GetEndZetaBlending();

    // get profile curves
    std::vector<TopoDS_Wire> profiles;
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

        profiles.push_back(profileWire);
    }

    for ( size_t i = 0; i<profiles.size(); ++i) {

#ifdef DEBUG
        std::stringstream ss;
        ss << "D:/tmp/nacelleProfile_"<<i<<".brep";
        BRepTools::Write(profiles[i], ss.str().c_str());
#endif

        lofter.addProfiles(profiles[i]);
    }

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
    for(size_t i = 1; i <= m_guideCurves.GetGuideCurveCount(); ++i ) {
        const CCPACSNacelleGuideCurve& guide = m_guideCurves.GetGuideCurve(i);

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
    std::vector<std::pair<double,TopoDS_Wire>> connectedZetaGuides = connectNacelleGuideCurves(zetaGuides, Precision::Confusion());

    for(size_t i=0; i<connectedZetaGuides.size(); ++i) {
#ifdef DEBUG
        std::stringstream ss;
        ss << "D:/tmp/nacelleGuide_"<<i<<".brep";
        BRepTools::Write(connectedZetaGuides[i].second, ss.str().c_str());
#endif
        lofter.addGuides(connectedZetaGuides[i].second);
    }

    return  lofter.Shape();
}

PNamedShape CCPACSNacelleCowl::BuildLoft() const
{
    // get outer shape
    TopoDS_Shape outerShape = BuildOuterShape();

#ifdef DEBUG
    BRepTools::Write(outerShape,"D:/tmp/outerShape.brep");
#endif

    // get rotation curve and generate rotationally symmetric interior
    TopoDS_Face innerShape = m_rotationCurve.GetRotationSurface();

#ifdef DEBUG
    BRepTools::Write(innerShape,"D:/tmp/innerShape.brep");
#endif

    // blend the surfaces

    return PNamedShape();
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
    double par1, par2;
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
    GCPnts_AbscissaPoint algo2(adaptorCurve, len*(zeta2 + 1.), umin);
    if (algo2.IsDone()) {
        par2 = algo2.Parameter();
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
};

} //anonymous namespace
