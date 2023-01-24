/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-05-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglMakeLoft.h"
#include "tigl_config.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"
#include "Debugging.h"

#include "CTiglBSplineAlgorithms.h"
#include "CTiglCurvesToSurface.h"
#include "CTiglInterpolateCurveNetwork.h"
#include "CTiglPatchShell.h"
#include "Debugging.h"
#include "to_string.h"

#include "contrib/MakePatches.hxx"
#include "geometry/CTiglTopoAlgorithms.h"
#include "geometry/CTiglBSplineAlgorithms.h"
#include "system/CTiglError.h"


#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp.hxx>
#include <BRep_Builder.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepAlgo.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom_Plane.hxx>
#include <StdFail_NotDone.hxx>
#include <Precision.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GeomConvert.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRep_Tool.hxx>

namespace {
    TopoDS_Shape ResortFaces(TopoDS_Shape const& shape, int nu, int nv, bool umajor2vmajor = true);
}

CTiglMakeLoft::CTiglMakeLoft(double tolerance, double sameKnotTolerance)
    : _hasPerformed{false}
    , _makeSolid{true}
#ifdef TIGL_GORDON_SURFACE
    , _use_gordon_surface_algorithm(true)
#else
    , _use_gordon_surface_algorithm(false)
#endif
    , _myTolerance{tolerance}
    , _mySameKnotTolerance{sameKnotTolerance}
{
    _result.Nullify();
}

CTiglMakeLoft::CTiglMakeLoft(const TopoDS_Shape& profiles, const TopoDS_Shape& guides, double tolerance, double sameKnotTolerance)
    : CTiglMakeLoft(tolerance, sameKnotTolerance)
{
    addProfiles(profiles);
    addGuides(guides);
}

void CTiglMakeLoft::addProfiles(const TopoDS_Shape &profiles)
{
    if (profiles.ShapeType() == TopAbs_COMPOUND) {
        for (TopoDS_Iterator anIter(profiles); anIter.More(); anIter.Next()) {
            TopoDS_Wire aSh = TopoDS::Wire(anIter.Value());
            this->profiles.push_back(aSh);
        }
    }
    else {
        this->profiles.push_back(TopoDS::Wire(profiles));
    }
}

void CTiglMakeLoft::addGuides(const TopoDS_Shape &guides)
{
    if (guides.ShapeType() == TopAbs_COMPOUND) {
        for (TopoDS_Iterator anIter(guides); anIter.More(); anIter.Next()) {
            TopoDS_Wire aSh = TopoDS::Wire(anIter.Value());
            this->guides.push_back(aSh);
        }
    }
    else {
        this->guides.push_back(TopoDS::Wire(guides));
    }
}

TopoDS_Shape &CTiglMakeLoft::Shape()
{
    Perform();
    
    return _result;
}

CTiglMakeLoft::operator TopoDS_Shape &()
{
    return Shape();
}

void CTiglMakeLoft::Perform() 
{
    if (_hasPerformed) {
        return;
    }
    
    DEBUG_SCOPE(debug);
    for (size_t i = 0; i < profiles.size(); ++i) {
        debug.addShape(profiles[i], "profile_" + tigl::std_to_string(i));
    }

    if (guides.size() > 0) {
        for (size_t i = 0; i < guides.size(); ++i) {
            debug.addShape(guides[i], "guide_" + tigl::std_to_string(i));
        }

        // to the loft with guides
        makeLoftWithGuides();
    }
    else {
        makeLoftWithoutGuides();
    }
    _hasPerformed = true;
}

void CTiglMakeLoft::useGordonSurfaceAlgorithm(bool enabled) {
    _use_gordon_surface_algorithm = enabled;
}

void CTiglMakeLoft::setMakeSolid(bool enabled)
{
    _makeSolid = enabled;
}

void CTiglMakeLoft::setMakeSmooth(bool enabled)
{
    _makeSmooth = enabled;
}

/**
 * @brief Builds the loft using profiles and guide curves
 */
void CTiglMakeLoft::makeLoftWithGuides()
{
    if (_use_gordon_surface_algorithm) {
        makeLoftGordon();
    }
    else {
        makeLoftCoons();
    }
}



/**
 * @brief Builds the loft using profiles and guide curves using Coons patches
 */
void CTiglMakeLoft::makeLoftGordon()
{
    std::vector<Handle(Geom_Curve)> guide_curves;
    for (auto const& guide : guides) {
        // TODO: Currently only considering first edge of wire, regardless of the
        // actual number of edges
        guide_curves.push_back(GetBSplineCurve(GetEdge(guide, 0)));
    }

    // get the profile curves
    std::vector<Handle(Geom_Curve)> profiles_curves;
    for (auto& profile : profiles){

        // remove leading edge split in profile
        if (GetNumberOfEdges(profile) > 1) {
            profile =  BRepAlgo::ConcatenateWire(profile,GeomAbs_C1);
        }

        // TODO: Currently only considering first edge of wire, regardless of the
        // actual number of edges
        profiles_curves.push_back(GetBSplineCurve(GetEdge(profile, 0)));
    }


    auto interpolator = tigl::CTiglInterpolateCurveNetwork(
                profiles_curves,
                guide_curves,
                1e-8 //TODO: spatial tolerance
    );
    auto surface = interpolator.Surface();

    auto face_maker = BRepBuilderAPI_MakeFace(surface, 1e-8); //TODO sensible tolerance
    _result = face_maker.Face();

    // make surface to face(s). We likely needSurface to trim along profiles and guides.

    CloseShape();
}

/**
 * @brief Builds the loft using profiles and guide curves using Coons patches
 */
void CTiglMakeLoft::makeLoftCoons()
{
    BRep_Builder b;
    TopoDS_Compound cprof, cguid;
    b.MakeCompound(cprof);
    b.MakeCompound(cguid);
    
    for (unsigned int i = 0; i < profiles.size(); ++i) {
        TopoDS_Wire& profile =  profiles[i];
        
        // remove leading edge split in profile
        if (GetNumberOfEdges(profile) > 1) {
            profile =  BRepAlgo::ConcatenateWire(profile,GeomAbs_C1);
        }
        b.Add(cprof, profile);
    }
    for (unsigned int i = 0; i < guides.size(); ++i) {
        TopoDS_Wire& guide =  guides[i];
        b.Add(cguid, guide);
    }
    
#ifdef DEBUG
    static int iLoft = 0;
    tigl::dumpShape(cprof, "debugShapes", "profiles", iLoft);
    tigl::dumpShape(cguid, "debugShapes", "guides", iLoft);
    iLoft++;
#endif
    
    MakePatches SurfMaker(cguid, cprof);
    // Don't sew yet. We do it later in solid creation
#ifdef HAVE_OCE_COONS_PATCHED
    GeomFill_FillingStyle style = GeomFill_CoonsC2Style;

    char* c_cont = getenv("TIGL_COONS_CONTINUITY");
    if (c_cont) {
        if (strcmp(c_cont, "0") == 0) {
            style = GeomFill_StretchStyle;
        }
        else if (strcmp(c_cont, "1") == 0) {
            style = GeomFill_CoonsStyle;
        }
        else if (strcmp(c_cont, "2") == 0) {
            style = GeomFill_CoonsC2Style;
        }

        static bool has_informed = false;
        if (!has_informed) {
            std::string s_style;
            switch(style) {
            case GeomFill_StretchStyle:
                s_style = "Stretch style (C0)";
                break;
            case GeomFill_CoonsStyle:
                s_style = "Coons style (C1)";
                break;
            case GeomFill_CoonsC2Style:
            default:
                s_style = "Coons C2 style (C2)";
            }
            LOG(WARNING) << "Using user-defined surface modelling: " << s_style;
            has_informed = true;
        }

    }
#else
    GeomFill_FillingStyle style = GeomFill_CoonsStyle;
#endif
    SurfMaker.Perform(_myTolerance, _mySameKnotTolerance, style, Standard_True);
    _result = SurfMaker.Patches();
    if (SurfMaker.GetStatus() > 0) {
        LOG(ERROR) << "Could not create loft with guide curves. " << "Error code = " << SurfMaker.GetStatus();
        return;
    }
    
#ifdef DEBUG
    // store intermediate result
    std::stringstream spatches;
    spatches << "patches" << iLoft << ".brep";
    BRepTools::Write(_result, spatches.str().c_str());
#endif
    CloseShape();
}

void CTiglMakeLoft::makeLoftWithoutGuides()
{
    // check number of edges are all same
    std::vector<unsigned int> edgeCountPerProfile;
    for (const auto& profile : profiles) {
        edgeCountPerProfile.push_back(GetNumberOfEdges(profile));
    }
    if (!AllSame(edgeCountPerProfile.begin(), edgeCountPerProfile.end())) {
        throw tigl::CTiglError("Number of edges not equal in CTiglMakeLoft");
    }

    TopoDS_Shell faces;
    BRep_Builder builder;
    builder.MakeShell(faces);

    // get number of edges per profile wire
    // --> should be the same for all profiles
    TopTools_IndexedMapOfShape firstProfileMap;
    TopExp::MapShapes(profiles[0], TopAbs_EDGE, firstProfileMap);
    int const nEdgesPerProfile = firstProfileMap.Extent();

    // skin the surface edge by adge
    // CAUTION: Here it is assumed that the edges are ordered
    // in the same way along each profile (e.g. lower edge,
    // upper edge, trailing edge for a wing)
    for ( int iE = 1; iE <= nEdgesPerProfile; ++iE ) {

        // get the curves
        std::vector<Handle(Geom_Curve)> profileCurves;
        profileCurves.reserve(profiles.size());
        for (unsigned iP=0; iP<profiles.size(); ++iP ) {

            TopTools_IndexedMapOfShape profileMap;
            TopExp::MapShapes(profiles[iP], TopAbs_EDGE, profileMap);
            assert( profileMap.Extent() >= iE );

            TopoDS_Edge edge = TopoDS::Edge(profileMap(iE));
            profileCurves.push_back(GetBSplineCurve(edge));
        }

        // skin the curves
        tigl::CTiglCurvesToSurface surfaceSkinner(profileCurves, vparams);
        if (!_makeSmooth) {
            surfaceSkinner.SetMaxDegree(1);
        }
        Handle(Geom_BSplineSurface) surface = surfaceSkinner.Surface();

        // remember the profile parameters used for the skinning
        if (vparams.size()==0) {
            vparams = surfaceSkinner.GetParameters();
        }

        builder.Add(faces, BRepBuilderAPI_MakeFace(surface, 1e-6).Face());
    }
    _result = tigl::CTiglTopoAlgorithms::CutShellAtUVParameters(faces, {}, vparams);

    // make sure the order is the same as for the COONS Patch algorithm
    _result = ResortFaces(_result, nEdgesPerProfile, static_cast<int>(vparams.size()-1));
    _result = tigl::CTiglTopoAlgorithms::CutShellAtKinks(_result);
    CloseShape();
}

void CTiglMakeLoft::CloseShape()
{
    tigl::CTiglPatchShell patcher(_result, _myTolerance);
    Standard_Boolean vClosed = (profiles[0].IsSame(profiles.back()));
    if ( !vClosed && _makeSolid ) {
        patcher.AddSideCap(TopoDS::Wire(profiles[0]));
        patcher.AddSideCap(TopoDS::Wire(profiles.back()));
    }
    patcher.SetMakeSolid(_makeSolid);
    _result = patcher.PatchedShape();
}

namespace
{

    TopoDS_Shape ResortFaces(TopoDS_Shape const& shape, int nu, int nv, bool umajor2vmajor)
    {
        nu = (nu < 1)? 1 : nu;
        nv = (nv < 1)? 1 : nv;

        // map of in faces
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape, TopAbs_FACE, map);
        int n = map.Extent();

        assert(nu*nv == n);

        TopoDS_Shell sorted;
        BRep_Builder B;
        B.MakeShell(sorted);
        if ( umajor2vmajor ) {
            for(int v = 1; v <= nv; v++) {
                for (int u = 1; u <= nu; u++) {
                    B.Add(sorted,TopoDS::Face(map((u-1)*nv + v)));
                }
            }
        }
        else
        {
            for(int u = 1; u <= nu; u++) {
                for (int v = 1; v <= nv; v++) {
                    B.Add(sorted,TopoDS::Face(map((v-1)*nu + u)));
                }
            }
        }

        return sorted;
    }
} // namespace
