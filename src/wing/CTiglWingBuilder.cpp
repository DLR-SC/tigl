/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-20 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglWingBuilder.h"

#include "CTiglMakeLoft.h"
#include "CCPACSWingSegment.h"
#include "CTiglLogging.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"

#include <TopExp.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepLib.hxx>
#include <Geom_Plane.hxx>

#include <cassert>
#include <algorithm>

#define NO_EXPLICIT_TE_MODELING

namespace
{
template <typename ForwardIter, typename Compare>
size_t find_index(ForwardIter begin, ForwardIter end, Compare comp)
{
    const auto it = std::find_if(begin, end, comp);
    if (it != end) {
        return std::distance(begin, it);
    }
    else {
        return std::distance(begin, end);
    }
}

template <typename ArrayLike, typename ValueType>
bool contains(const ArrayLike& array, ValueType val)
{
    auto idx = find_index(std::cbegin(array), std::cend(array), [val](const auto& cval) {
        return fabs(cval - val) < 1e-3;
    });

    return idx < array.size();
}

} // namespace

namespace tigl
{

Standard_Boolean CreateSideCap(const TopoDS_Wire& W,
                               const Standard_Real presPln,
                               TopoDS_Face& theFace);

CTiglWingBuilder::CTiglWingBuilder(const CCPACSWing& wing)
    : _wing(wing)
{
}

#ifndef NO_EXPLICIT_TE_MODELING
PNamedShape CTiglWingBuilder::BuildShape()
{
    const CCPACSWingSegments& segments = _wing.m_segments;

    // check whether we have a blunt TE or not
    const CCPACSWingProfile& innerProfile = segments.GetSegment(1).GetInnerConnection().GetProfile();

    TopoDS_Compound guideCurves = _wing.GetGuideCurveWires();

    std::vector<TopoDS_Wire> guides;
    for (TopoDS_Iterator anIter(guideCurves); anIter.More(); anIter.Next()) {
        TopoDS_Wire aSh = TopoDS::Wire(anIter.Value());
        guides.push_back(aSh);
    }

    // we assume, that all profiles of one wing are either blunt or not
    // this is checked during cpacs loading of each wing segment
    bool hasBluntTE = innerProfile.HasBluntTE();
    bool hasGuideCurves = guides.size() > 0;


    CTiglMakeLoft lofter;
    lofter.setMakeSolid(false);

    std::vector<gp_Pnt> upperTEPoints, lowerTEPoints;
    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        const TopoDS_Wire& startWire = segments.GetSegment(i).GetInnerWire();

        // extract trailing edge, in case of a blunt TE
        // we assume, that the TE is the last edge of the wire
        if (hasBluntTE) {
            BRepBuilderAPI_MakeWire wireMaker;
            TopTools_IndexedMapOfShape edgeMap;
            TopExp::MapShapes(startWire, TopAbs_EDGE, edgeMap);
            for (int i = 1; i <= edgeMap.Extent(); ++i) {
                const TopoDS_Edge& e = TopoDS::Edge(edgeMap(i));
                if (i < edgeMap.Extent()) {
                    wireMaker.Add(e);
                }
            }
            TopoDS_Wire aeroProfile = wireMaker.Wire();
            lofter.addProfiles(aeroProfile);
            upperTEPoints.push_back(GetLastPoint(aeroProfile));
            lowerTEPoints.push_back(GetFirstPoint(aeroProfile));
        }
        else {
            lofter.addProfiles(startWire);
        }
    }

    TopoDS_Wire endWire =  segments.GetSegment(segments.GetSegmentCount()).GetOuterWire();
    if (hasBluntTE) {
        // extract the wire without the trailing edge

        BRepBuilderAPI_MakeWire wireMaker;
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(endWire, TopAbs_EDGE, edgeMap);
        for (int i = 1; i <= edgeMap.Extent(); ++i) {
            const TopoDS_Edge& e = TopoDS::Edge(edgeMap(i));
            if (i < edgeMap.Extent()) {
                wireMaker.Add(e);
            }
        }
        TopoDS_Wire aeroProfile = wireMaker.Wire();
        lofter.addProfiles(aeroProfile);
        upperTEPoints.push_back(GetLastPoint(aeroProfile));
        lowerTEPoints.push_back(GetFirstPoint(aeroProfile));
    }
    else {
        lofter.addProfiles(endWire);
    }

    // add guide curves
    lofter.addGuides(guideCurves);

    TopoDS_Shape aeroShape = lofter.Shape();
    BRepBuilderAPI_Sewing sewingAlgo;
    sewingAlgo.Add(aeroShape);

    // If blunt trailing edge is available, model it explicitly
    if (hasBluntTE) {

        TopoDS_Wire upperTE, lowerTE;
        if (hasGuideCurves) {
            // The guide curves are sorted in the order of the relative starting coordinate
            // at the innermost section
            upperTE = guides.back();
            lowerTE = guides.front();
        }
        else {
            // model the edges by taking the first and last profile points

            assert(lowerTEPoints.size() == upperTEPoints.size());
            assert(lowerTEPoints.size() > 1);

            // build upper edge
            BRepBuilderAPI_MakeWire upperWireMaker, lowerWireMaker;
            for (unsigned int i = 0; i < upperTEPoints.size() - 1; ++i) {
                upperWireMaker.Add(BRepBuilderAPI_MakeEdge(upperTEPoints.at(i), upperTEPoints.at(i+1)));
                lowerWireMaker.Add(BRepBuilderAPI_MakeEdge(lowerTEPoints.at(i), lowerTEPoints.at(i+1)));
            }
            upperTE = upperWireMaker.Wire();
            lowerTE = lowerWireMaker.Wire();
        }

        // the TE is build using a ruled loft
        BRepOffsetAPI_ThruSections trailingEdgeBuilder(Standard_False, Standard_True);
        trailingEdgeBuilder.AddWire(upperTE);
        trailingEdgeBuilder.AddWire(lowerTE);
        trailingEdgeBuilder.Build();
        TopoDS_Shape trailingEdge = trailingEdgeBuilder.Shape();
        sewingAlgo.Add(trailingEdge);

    } // hasBluntTE


    // get the profiles
    TopoDS_Wire innerWire = segments.GetSegment(1).GetInnerWire();
    TopoDS_Wire outerWire = segments.GetSegment(segments.GetSegmentCount()).GetOuterWire();

    TopoDS_Face innerFace, outerFace;
    CreateSideCap(innerWire, 1e-6, innerFace);
    CreateSideCap(outerWire, 1e-6, outerFace);

    sewingAlgo.Add(innerFace);
    sewingAlgo.Add(outerFace);

    sewingAlgo.Perform();
    TopoDS_Shape shellClosed  = sewingAlgo.SewedShape();
    if (shellClosed.ShapeType() != TopAbs_SHELL)
        throw CTiglError("Expected sewing algo to construct a shell when building wing loft");
    shellClosed.Closed(Standard_True);

    // make solid from shell
    TopoDS_Solid solid;
    BRep_Builder solidMaker;
    solidMaker.MakeSolid(solid);
    solidMaker.Add(solid, shellClosed);

    // verify the orientation the solid
    BRepClass3d_SolidClassifier clas3d(solid);
    clas3d.PerformInfinitePoint(Precision::Confusion());
    if (clas3d.State() == TopAbs_IN) {
     solidMaker.MakeSolid(solid);
        TopoDS_Shape aLocalShape = shellClosed.Reversed();
        solidMaker.Add(solid, TopoDS::Shell(aLocalShape));
    }

    solid.Closed(Standard_True);
    BRepLib::EncodeRegularity(solid);

    std::string loftName = _wing.GetUID();
    std::string loftShortName = _wing.GetShortShapeName();
    PNamedShape loft(new CNamedShape(solid, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(_wing.GetUID(), loft, hasBluntTE);

    return loft;
}
#else
PNamedShape CTiglWingBuilder::BuildShape()
{
    const CCPACSWingSegments& segments = _wing.GetSegments();
    const CCPACSWingProfile& innerProfile = segments.GetSegment(1).GetInnerConnection().GetProfile();

    // we assume, that all profiles of one wing are either blunt or not
    // this is checked during cpacs loading of each wing segment
    bool hasBluntTE = innerProfile.HasBluntTE();

    CTiglMakeLoft lofter;
    lofter.setMakeSolid(true);

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        const TopoDS_Shape& startWire = segments.GetSegment(i).GetInnerWire();
        lofter.addProfiles(startWire);
    }

    TopoDS_Wire endWire =  segments.GetSegment(segments.GetSegmentCount()).GetOuterWire();
    lofter.addProfiles(endWire);

    // add guide curves
    lofter.addGuides(_wing.GetGuideCurveWires());

    TopoDS_Shape loftShape = lofter.Shape();
    std::string loftName = _wing.GetUID();
    std::string loftShortName = _wing.GetShortShapeName();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str(), loftShortName.c_str()));
    SetFaceTraits(_wing.GetUID(), loft, hasBluntTE);
    return loft;
}
#endif



CTiglWingBuilder::operator PNamedShape()
{
    return BuildShape();
}
// Set the name of each wing face
void CTiglWingBuilder::SetFaceTraits (const std::string& wingUID, PNamedShape loft, bool hasBluntTE)
{
    unsigned int nSegments = _wing.GetSegmentCount();

    auto params = _wing.GetGuideCurveStartParameters();
    assert(std::is_sorted(std::begin(params), std::end(params)));

    bool hasGuideCurves = params.size() > 0.;
    LOG(ERROR) << "HasGuideCurves " << hasGuideCurves;

    size_t nFacesPerSegment = 2; // Without trailing edge
    size_t idx_leading_edge = 1;
    if (hasGuideCurves) {
        if (!contains(params, -1.)) {
            params.insert(params.begin(), -1.);
        }

        if (!contains(params, 1.)) {
            params.push_back(1.);
        }

        // find leading edge curve
        idx_leading_edge = find_index(params.cbegin(), params.cend(), [] (double val) {
            return fabs(val) < 1e-3;
        });

        if (idx_leading_edge == params.size()) {
            // no guide curve at leading edge
            LOG(ERROR) << "No guide curve at leading edge defined";
            return;
        }
        nFacesPerSegment = params.size() - 1;
    }

    // designated names of the faces
    std::vector<std::string> names;
    for (size_t i = 0; i < idx_leading_edge; ++i) {
        names.push_back("Bottom");
    }

    for (size_t i = idx_leading_edge; i < nFacesPerSegment; ++i) {
        names.push_back("Top");
    }

    if (hasBluntTE) {
        names.push_back("TrailingEdge");
        nFacesPerSegment += 1;
    }

    std::vector<std::string> endnames(2);
    endnames[0]="Inside";
    endnames[1]="Outside";

    unsigned int nFaces = GetNumberOfFaces(loft->Shape());

    for (unsigned int i = 0; i < nFaces; i++) {
        loft->FaceTraits(i).SetComponentUID(wingUID);
    }

    if (nFacesPerSegment*nSegments + 2 != nFaces) {
        LOG(ERROR) << "CCPACSWingBuilder: Unable to determine wing face names from wing loft.";
        return;
    }

#ifndef NO_EXPLICIT_TE_MODELING

    unsigned int nTEFaces = hasBluntTE ? nSegments : 0;
    unsigned int nAeroFaces = nFaces - nTEFaces - 2;

    // assign "Top" and "Bottom" to face traits
    for (unsigned int i = 0; i < nAeroFaces; i++) {
        CFaceTraits traits = loft->GetFaceTraits(i);
        traits.SetName(names[i%2].c_str());
        loft->SetFaceTraits(i, traits);
    }

    // assign TE to face traits
    for (unsigned int i = nAeroFaces; i < nAeroFaces + nTEFaces; ++i) {
        CFaceTraits traits = loft->GetFaceTraits(i);
        traits.SetName(names[2].c_str());
        loft->SetFaceTraits(i, traits);
    }
#else
    // assign "Top" and "Bottom" to face traits
    for (unsigned int i = 0; i < nFaces-2; i++) {
        CFaceTraits traits = loft->GetFaceTraits(i);
        traits.SetName(names[i%names.size()]);
        loft->SetFaceTraits(i, traits);
    }
#endif

    // assign "Inside" and "Outside" to face traits
    for (unsigned int i = nFaces-2; i < nFaces; i++) {
        CFaceTraits traits = loft->GetFaceTraits(i);
        traits.SetName(endnames[i-nFaces+2]);
        loft->SetFaceTraits(i, traits);
    }
}

// creates the inside and outside cap of the wing
Standard_Boolean CreateSideCap(const TopoDS_Wire& W,
                               const Standard_Real presPln,
                               TopoDS_Face& theFace)
{
    Standard_Boolean isDegen = Standard_True;
    TopoDS_Iterator iter(W);
    for (; iter.More(); iter.Next())
    {
        const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
        if (!BRep_Tool::Degenerated(anEdge))
            isDegen = Standard_False;
    }
    if (isDegen)
        return Standard_True;

    Standard_Boolean Ok = Standard_False;
    if (!W.IsNull()) {
        BRepBuilderAPI_FindPlane Searcher( W, presPln );
        if (Searcher.Found()) {
            theFace = BRepBuilderAPI_MakeFace(Searcher.Plane(), W);
            Ok = Standard_True;
        }
        else {
            // try to find another surface
            BRepBuilderAPI_MakeFace MF( W );
            if (MF.IsDone())
            {
                theFace = MF.Face();
                Ok = Standard_True;
            }
        }
    }

    return Ok;
}

} //namespace tigl
