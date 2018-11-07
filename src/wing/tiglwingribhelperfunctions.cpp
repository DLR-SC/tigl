/*
* Copyright (C) 2016 Airbus Defence and Space
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

#include "tiglwingribhelperfunctions.h"

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBndLib.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include "CCPACSWing.h"
#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSparPosition.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglWingStructureReference.h"
#include "tiglcommonfunctions.h"
#include "tigletaxsifunctions.h"


namespace tigl
{

TopoDS_Shape ApplyWingTransformation(const CCPACSWingCSStructure& structure, const TopoDS_Shape& shape)
{
    return CTiglWingStructureReference(structure).GetWing().GetTransformationMatrix().Transform(shape);
}

TopoDS_Shape CutShapeWithSpars(const TopoDS_Shape& shape, const CCPACSWingCSStructure& structure)
{
    double precision = 10e-3;
    // create initial bounding box for shape
    Bnd_Box boundingBox;
    BRepBndLib::Add(shape, boundingBox);
    boundingBox.Enlarge(precision);

    // next iterate over all spars and build up a compound of all participating
    // split shapes
    int numSpars = structure.GetSparSegmentCount();
    BRep_Builder builder;
    TopoDS_Compound splitCompound;
    builder.MakeCompound(splitCompound);
    for (int i = 1; i <= numSpars; i++) {
        // get the split geometry from the spars
        const CCPACSWingSparSegment &sparSegment = structure.GetSparSegment(i);

        Bnd_Box checkBox;
        BRepBndLib::Add(sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM), checkBox);
        checkBox.Enlarge(precision);

        if (!boundingBox.IsOut(checkBox)) {
            builder.Add(splitCompound, sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM));
        }
    }
    // finally cut the shape with the compound of all split shapes
    return SplitShape(shape, splitCompound);
}

gp_Pnt GetReferencePoint(const CCPACSWingCSStructure& structure, const std::string& reference, double currentEta)
{
    const CTiglWingStructureReference wsr(structure);
    gp_Pnt referencePnt;
    if (reference == "leadingEdge") {
        referencePnt = wsr.GetLeadingEdgePoint(currentEta);
    }
    else if (reference == "trailingEdge") {
        referencePnt = wsr.GetTrailingEdgePoint(currentEta);
    }
    else {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(reference);
        referencePnt = sparSegment.GetPoint(currentEta);
    }
    return referencePnt;
}

double GetRibReferenceLength(const std::string& reference, const CCPACSWingCSStructure& structure)
{
    const CTiglWingStructureReference wsr(structure);
    double referenceLength;
    if (reference == "leadingEdge") {
        referenceLength = wsr.GetLeadingEdgeLength();
    }
    else if (reference == "trailingEdge") {
        referenceLength = wsr.GetTrailingEdgeLength();
    }
    else {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(reference);
        referenceLength = sparSegment.GetSparLength();
    }
    return referenceLength;
}

// NOTE: method works only for eta==0 or eta==1, eta values in the mid of a spar
// are not checked for coincidence with a section element
bool IsOuterSparPointInSection(const std::string& sparUid, double eta, const CCPACSWingCSStructure& structure)
{
    // check whether spar point lies within a section
    const CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(sparUid);
    int sparPositionIndex;
    if (eta < Precision::Confusion()) {
        sparPositionIndex = 1;
    }
    else if (eta > 1 - Precision::Confusion()) {
        sparPositionIndex = sparSegment.GetSparPositionUIDs().GetSparPositionUIDCount();
    }
    else {
        return false;
    }
    const CCPACSWingSparPosition& pos = sparSegment.GetSparPosition(sparSegment.GetSparPositionUIDs().GetSparPositionUID(sparPositionIndex));
    if (pos.GetInputType() == CCPACSWingSparPosition::ElementUID) {
        return true;
    }
    else if (pos.GetInputType() == CCPACSWingSparPosition::Eta &&
             (pos.GetEta() < Precision::Confusion() || pos.GetEta() > 1 - Precision::Confusion())) {
        return true;
    }
    return false;
}

gp_Vec GetUpVectorWithoutXRotation(const std::string& ribReference, double currentEta, const gp_Pnt& startPnt, 
                                   const std::string& sparPositionUID, const CCPACSWingCSStructure& structure)
{
    const CTiglWingStructureReference wsr(structure);

    // use the midplane normal as up vector
    double midplaneEta, dummy;
    wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
    gp_Vec upVec = wsr.GetMidplaneNormal(midplaneEta);

    // Bug #408: special handling in case the rib is defined at the spar position or start or end point of a spar
    if (!sparPositionUID.empty() || (ribReference != "leadingEdge" && ribReference != "trailingEdge" &&
        (currentEta < Precision::Confusion() || currentEta >(1 - Precision::Confusion())))) {
        // TODO: here it is expected that the up vector of the spars is always (0,0,1) !!!
        upVec = gp_Vec(0, 0, 1);
    }

    return upVec;
}

void ApplyXRotationToUpVector(double angle, gp_Vec& upVec, const gp_Vec& ribDir)
{
    // NOTE: up-vector is already perpendicular to midplane, so rotation angle is decreased by 90 deg
    double xRotation = (angle - 90) * M_PI / 180.0;
    upVec.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(ribDir)), xRotation);
}

double GetBoundingBoxSize(const TopoDS_Shape& shape)
{
    Bnd_Box bbox;
    BRepBndLib::Add(shape, bbox);
    double bboxSize = sqrt(bbox.SquareExtent());
    return bboxSize;
}

TopoDS_Wire CutFaceWithSpar(TopoDS_Shape& ribCutFace, const TopoDS_Shape& sparGeometry,
                            double bboxSize, TopoDS_Wire& changedWire, const TopoDS_Wire& wire2,
                            const gp_Vec& upVec)
{
    // split rib face with spar geometry
    TopoDS_Shape cutEdges = CutShapes(sparGeometry, ribCutFace);
    // remove duplicate edges from result (may occur when two edges overlap in sparGeometry and ribCutFace)
    cutEdges = RemoveDuplicateEdges(cutEdges);
    // get a single wire out of the edges
    TopoDS_Wire cutWire = BuildWireFromEdges(cutEdges);

    // get end vertices of wire
    TopTools_ListOfShape endVertices;
    GetEndVertices(cutWire, endVertices);
    // check if number of end vertices is valid
    if (endVertices.Extent() > 2) {
        LOG(ERROR) << "invalid number of end vertices found!";
        throw CTiglError("invalid number of end vertices found in CCPACSWingRibsDefinition::CutFaceWithSpar!");
    }

    // extend the edges of the end vertices
    TopTools_ListOfShape extendedEdges;
    BRepBuilderAPI_MakeWire makeWire;
    TopTools_ListOfShape edgeList;
    TopExp_Explorer edgeExplorer;
    // Next iterate over all edges, and scale the ones which are connected to the end vertices
    for (edgeExplorer.Init(cutWire, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
        TopoDS_Vertex v1 = TopExp::FirstVertex(edge);
        TopoDS_Vertex v2 = TopExp::LastVertex(edge);
        extendedEdges.Clear();
        // iterate over the end vertices
        TopTools_ListIteratorOfListOfShape endVertexIt;
        for (endVertexIt.Initialize(endVertices); endVertexIt.More(); endVertexIt.Next()) {
            TopoDS_Vertex& vEnd = TopoDS::Vertex(endVertexIt.Value());
            // check if the edge is connected to the end vertex
            if (v1.IsSame(vEnd) || v2.IsSame(vEnd)) {
                // get the inner and outer vertex of the edge
                TopoDS_Vertex vInner = v1;
                if (vInner.IsSame(vEnd)) {
                    vInner = v2;
                }
                gp_Pnt pInner = BRep_Tool::Pnt(vInner);
                gp_Pnt pOuter = BRep_Tool::Pnt(vEnd);
                gp_Vec dir(pInner, pOuter);
                // build extension edge along up vector
                gp_Pnt pExtend;
                if (dir.Dot(upVec) >= 0) {
                    pExtend = pOuter.Translated(bboxSize * upVec);
                }
                else {
                    pExtend = pOuter.Translated(-bboxSize * upVec);
                }

                TopoDS_Vertex vExtend = BRepBuilderAPI_MakeVertex(pExtend);
                extendedEdges.Append(BRepBuilderAPI_MakeEdge(pOuter, pExtend));
            }
        }
        // add original edges and extension edges to resulting wire
        makeWire.Add(edge);
        if (extendedEdges.Extent() > 0) {
            makeWire.Add(extendedEdges);
        }
    }
    changedWire = SortWireEdges(makeWire.Wire());

    // build rib cut face based on new wire
    ribCutFace = BuildFace(changedWire, wire2, upVec);

    // return result wire of intersection from ribCutFace with spar
    return cutWire;
}

gp_Pnt GetRibDefinitionPoint(const std::string& definition, const TopoDS_Face& ribCutFace, const CCPACSWingCSStructure& structure)
{
    const CTiglWingStructureReference wsr(structure);
    gp_Pnt definitionPoint;
    if (definition == "leadingEdge") {
        TopoDS_Wire leadingEdgeLine = wsr.GetLeadingEdgeLine();
        if (!GetIntersectionPoint(ribCutFace, leadingEdgeLine, definitionPoint)) {
            LOG(ERROR) << "Unable to determine rib definition point!";
            throw CTiglError("Unable to determine rib definition point in CCPACSWingRibsDefinition::GetRibDefinitionPoint!");
        }
    }
    else if (definition == "trailingEdge") {
        TopoDS_Wire trailingEdgeLine = wsr.GetTrailingEdgeLine();
        if (!GetIntersectionPoint(ribCutFace, trailingEdgeLine, definitionPoint)) {
            LOG(ERROR) << "Unable to determine rib definition point!";
            throw CTiglError("Unable to determine rib definition point in CCPACSWingRibsDefinition::GetRibDefinitionPoint!");
        }
    }
    else {
        // get spar
        std::string sparUid = definition;
        const CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(sparUid);
        TopoDS_Wire sparMidplaneLine = sparSegment.GetSparMidplaneLine();
        if (!GetIntersectionPoint(ribCutFace, sparMidplaneLine, definitionPoint)) {
            LOG(ERROR) << "Unable to determine rib definition point!";
            throw CTiglError("Unable to determine rib definition point in CCPACSWingRibsDefinition::GetRibDefinitionPoint!");
        }
    }
    return definitionPoint;
}

gp_Pnt GetSparMidplanePoint(const CCPACSWingSparPosition& sparPos, const CCPACSWingCSStructure& structure)
{
    const CTiglWingStructureReference wsr(structure);
    gp_Pnt midplanePoint;
    if (sparPos.GetInputType() == CCPACSWingSparPosition::ElementUID) {
        const CCPACSWingComponentSegment& componentSegment = wsr.GetWingComponentSegment();
        midplanePoint = getSectionElementChordlinePoint(componentSegment, sparPos.GetElementUID(), sparPos.GetXsi());
    }
    else if (sparPos.GetInputType() == CCPACSWingSparPosition::Eta) {
        midplanePoint = wsr.GetPoint(sparPos.GetEta(), sparPos.GetXsi(), WING_COORDINATE_SYSTEM);
    }
    else {
        throw CTiglError("Unknown SparPosition InputType found in CCPACSWingRibsDefinition::GetSparMidplanePoint");
    }
    return midplanePoint;
}

void CheckSparPositionOnReference(const std::string& sparPositionUID, const std::string& ribReference,
                                  const CCPACSWingCSStructure& structure)
{
    // first ensure that the reference line is a spar
    int numSparSegments = structure.GetSparSegmentCount();
    int sparSegmentIndex = 1;
    for (; sparSegmentIndex <= numSparSegments; ++sparSegmentIndex) {
        if (structure.GetSparSegment(sparSegmentIndex).GetUID() == ribReference) {
            break;
        }
    }
    if (sparSegmentIndex > numSparSegments) {
        LOG(ERROR) << "Invalid ribs definition: use of spar position for rib positioning only valid when spar is used as rib reference line!";
        throw CTiglError("Invalid ribs definition: use of spar position for rib positioning only valid when spar is used as rib reference line!");
    }

    // next ensure that the spar position is part of the spar segment
    const CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(sparSegmentIndex);
    int numSparPositions = sparSegment.GetSparPositionUIDs().GetSparPositionUIDCount();
    int sparPositionIndex = 1;
    for (; sparPositionIndex <= numSparPositions; ++sparPositionIndex) {
        if (sparSegment.GetSparPositionUIDs().GetSparPositionUID(sparPositionIndex) == sparPositionUID) {
            break;
        }
    }
    if (sparPositionIndex > numSparPositions) {
        LOG(ERROR) << "Invalid ribs definition: used spar position \"" + sparPositionUID + "\" must be part of reference spar \"" + ribReference + "\"!";
        throw CTiglError("Invalid ribs definition: used spar position \"" + sparPositionUID + "\" must be part of reference spar \"" + ribReference + "\"!");
    }
}


} // namespace
