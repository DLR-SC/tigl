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

namespace tigl
{


CTiglWingBuilder::CTiglWingBuilder(const CCPACSWing& wing)
    : _wing(wing)
{
}

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
    SetFaceTraits(_wing.GetGuideCurveStartParameters(), _wing.GetUID(), loft, hasBluntTE);
    return loft;
}


CTiglWingBuilder::operator PNamedShape()
{
    return BuildShape();
}
// Set the name of each wing face
void CTiglWingBuilder::SetFaceTraits (const std::vector<double>& guideCurveParams, const std::string& shapeUID, PNamedShape shape, bool hasBluntTE)
{
    auto params = guideCurveParams;
    assert(std::is_sorted(std::begin(params), std::end(params)));

    bool hasGuideCurves = params.size() > 0.;

    size_t nFacesPerSegment = 2; // Without trailing edge
    size_t idx_leading_edge = 1;
    if (hasGuideCurves) {
        double tolerance = 1e-3;
        if (std::abs(params.front() + 1.) > tolerance) {
            params.insert(params.begin(), -1.);
        }

        if (std::abs(params.back() - 1.) > tolerance) {
            params.push_back(1.);
        }

        // find leading edge curve
        idx_leading_edge = FindIndex(params.cbegin(), params.cend(), [tolerance] (double val) {
            return fabs(val) < tolerance;
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

    unsigned int nFaces = GetNumberOfFaces(shape->Shape());

    for (unsigned int i = 0; i < nFaces; i++) {
        shape->FaceTraits(i).SetComponentUID(shapeUID);
    }

    if ((nFaces - 2) % nFacesPerSegment != 0) {
        LOG(ERROR) << "CCPACSWingBuilder: Unable to determine wing face names from wing loft.";
        return;
    }

    // assign "Top" and "Bottom" to face traits
    for (unsigned int i = 0; i < nFaces-2; i++) {
        CFaceTraits traits = shape->GetFaceTraits(i);
        traits.SetName(names[i%names.size()]);
        shape->SetFaceTraits(i, traits);
    }

    // assign "Inside" and "Outside" to face traits
    for (unsigned int i = nFaces-2; i < nFaces; i++) {
        CFaceTraits traits = shape->GetFaceTraits(i);
        traits.SetName(endnames[i-nFaces+2]);
        shape->SetFaceTraits(i, traits);
    }
}

} //namespace tigl
