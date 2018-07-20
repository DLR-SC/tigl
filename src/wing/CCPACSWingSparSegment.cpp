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

#include "CCPACSWingSparSegment.h"

#include <cassert>

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <gp_Pln.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include "CCPACSWing.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSparPosition.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglWingStructureReference.h"
#include "tiglcommonfunctions.h"
#include "tigletaxsifunctions.h"
#include "CNamedShape.h"


// TODO: const sparsNode
TopoDS_Shape ApplyWingTransformation(tigl::CCPACSWingSpars& sparsNode, const TopoDS_Shape& shape)
{
    return sparsNode.GetParent()->GetWingStructureReference().GetWing().GetTransformationMatrix().Transform(shape);
}

namespace tigl
{

CCPACSWingSparSegment::CCPACSWingSparSegment(CCPACSWingSparSegments* sparSegments, CTiglUIDManager* uidMgr)
    : generated::CPACSSparSegment(sparSegments, uidMgr), sparsNode(*sparSegments->GetParent())
{
    Invalidate();
}

void CCPACSWingSparSegment::Invalidate()
{
    auxGeomCache = boost::none;
    geometryCache = boost::none;
    splittedGeomCache = boost::none;
    sparCapsCache = boost::none;
}

CCPACSWingSparPosition& CCPACSWingSparSegment::GetSparPosition(std::string uID)
{
    return sparsNode.GetSparPositions().GetSparPosition(uID);
}

const CCPACSWingSparPosition& CCPACSWingSparSegment::GetSparPosition(std::string uid) const
{
    return sparsNode.GetSparPositions().GetSparPosition(uid);
}

// Returns the eta point on the midplane line of the spar segment
gp_Pnt CCPACSWingSparSegment::GetPoint(double eta) const
{
    // get point on spar line
    gp_Pnt sparPnt;
    TopoDS_Wire sparMidplaneLine = GetSparMidplaneLine();
    BRepAdaptor_CompCurve sparMidplaneLineCurve(sparMidplaneLine, Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(sparMidplaneLineCurve);
    sparMidplaneLineCurve.D0(len * eta, sparPnt);

    return sparPnt;
}

gp_Vec CCPACSWingSparSegment::GetDirection(double eta) const
{
    // get point on spar line
    gp_Pnt sparPnt;
    gp_Vec sparDir;
    TopoDS_Wire sparMidplaneLine = GetSparMidplaneLine();
    BRepAdaptor_CompCurve sparMidplaneLineCurve(sparMidplaneLine, Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(sparMidplaneLineCurve);
    sparMidplaneLineCurve.D1(len * eta, sparPnt, sparDir);

    return sparDir;
}

double CCPACSWingSparSegment::GetSparLength() const
{
    TopoDS_Wire sparMidplaneLine = GetSparMidplaneLine();
    BRepAdaptor_CompCurve sparMidplaneLineCurve(sparMidplaneLine, Standard_True);
    double length = GCPnts_AbscissaPoint::Length(sparMidplaneLineCurve);
    return length;
}

gp_Pnt CCPACSWingSparSegment::GetMidplanePoint(int positionIndex) const
{
    if (positionIndex > m_sparPositionUIDs.GetSparPositionUIDCount()) {
        LOG(ERROR) << "Invalid spar position index " << positionIndex << " requested from spar segment \"" << m_uID.value_or("") << "\"!";
        throw CTiglError("Invalid spar position index requested from spar segment \"" + m_uID.value_or("") + "\"!");
    }

    return GetMidplanePoint(m_sparPositionUIDs.GetSparPositionUID(positionIndex));
}

void CCPACSWingSparSegment::GetEtaXsi(int positionIndex, double& eta, double& xsi) const
{
    if (positionIndex < 1 || positionIndex > m_sparPositionUIDs.GetSparPositionUIDCount()) {
        LOG(ERROR) << "Invalid spar position index " << positionIndex << " requested from spar segment \"" << m_uID.value_or("") << "\"!";
        throw CTiglError("Invalid spar position index requested from spar segment \"" + m_uID.value_or("") + "\"!");
    }

    const std::string& sparPositionUID = m_sparPositionUIDs.GetSparPositionUID(positionIndex);
    const CCPACSWingSparPosition& sparPosition = sparsNode.GetSparPositions().GetSparPosition(sparPositionUID);

    if (sparPosition.GetInputType() == CCPACSWingSparPosition::Eta) {
        eta = sparPosition.GetEta();
    }
    else if (sparPosition.GetInputType() == CCPACSWingSparPosition::ElementUID) {
        gp_Pnt sparPositionPoint = GetMidplanePoint(sparPositionUID);
        double dummy;
        sparsNode.GetParent()->GetWingStructureReference().GetEtaXsiLocal(sparPositionPoint, eta, dummy);
        assert(fabs(dummy - xsi) < 1.E-6);
    }
    else {
        throw CTiglError("Unknown SparPosition-InputType found in CCPACSWingSparSegment::GetEtaXsi");
    }
    xsi = sparPosition.GetXsi();
}

TopoDS_Wire CCPACSWingSparSegment::GetSparMidplaneLine() const
{
    if (!auxGeomCache) {
        BuildAuxiliaryGeometry();
    }
    assert(auxGeomCache);

    return auxGeomCache.value().sparMidplaneLine;
}

TopoDS_Shape CCPACSWingSparSegment::GetSparGeometry(TiglCoordinateSystem referenceCS) const
{
    if (!geometryCache) {
        BuildGeometry();
    }
    assert(geometryCache);

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return geometryCache.value().shape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, geometryCache.value().shape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingSparSegment::GetSparGeometry!");
    }
}

TopoDS_Shape CCPACSWingSparSegment::GetSplittedSparGeometry(TiglCoordinateSystem referenceCS) const
{
    if (!splittedGeomCache) {
        BuildSplittedSparGeometry();
    }
    assert(splittedGeomCache);

    TopoDS_Shape splittedSparShape;
    splittedSparShape = splittedGeomCache.value().shape;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return splittedSparShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, splittedSparShape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingSparSegment::GetSplittedSparGeometry!");
    }
}

TopoDS_Shape CCPACSWingSparSegment::GetSparCutGeometry(TiglCoordinateSystem referenceCS) const
{
    if (!auxGeomCache) {
        BuildAuxiliaryGeometry();
    }
    assert(auxGeomCache);

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return auxGeomCache.value().sparCutShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, auxGeomCache.value().sparCutShape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingSparSegment::GetSparCutGeometry!");
    }
}

bool CCPACSWingSparSegment::HasCap(SparCapSide side) const
{
    switch (side) {
    case UPPER:
        return !!m_sparCrossSection.GetUpperCap();
        break;
    case LOWER:
        return !!m_sparCrossSection.GetLowerCap();
        break;
    default:
        throw CTiglError("Unsupported SparCapSide passed to CCPACSWingSparSegment::HasCap!");
    }
}

TopoDS_Shape CCPACSWingSparSegment::GetSparCapsGeometry(SparCapSide side, TiglCoordinateSystem referenceCS) const
{
    if (!sparCapsCache) {
        BuildSparCapsGeometry();
    }
    assert(sparCapsCache);

    TopoDS_Shape capsShape;
    if (side == UPPER) {
        capsShape = sparCapsCache.value().upperCapsShape;
    }
    else { // side == LOWER
        capsShape = sparCapsCache.value().lowerCapsShape;
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return capsShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, capsShape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingSparSegment::GetSparCapsGeometry!");
    }
}

std::string CCPACSWingSparSegment::GetDefaultedUID() const
{
    if (GetUID().is_initialized()) {
        return GetUID().value();
    }
    else {
        return "Unknown_Spar_Segment";
    }
}

TiglGeometricComponentType CCPACSWingSparSegment::GetComponentType() const
{
    return TIGL_COMPONENT_PHYSICAL;
}

// Builds the cutting geometry for the spar as well as the midplane line
void CCPACSWingSparSegment::BuildAuxiliaryGeometry() const
{
    // get assigned componentsegment
    CTiglWingStructureReference wingStructureReference = sparsNode.GetParent()->GetWingStructureReference();

    // build compound for cut geometry
    BRepBuilderAPI_Sewing cutGeomSewer;

    // get bounding box of loft
    TopoDS_Shape loft = wingStructureReference.GetLoft()->Shape();
    Bnd_Box bbox;
    BRepBndLib::Add(loft, bbox);
    double bboxSize = sqrt(bbox.SquareExtent());

    // container for all midplane points of the spar segment
    BRepBuilderAPI_MakeWire sparMidplaneLineBuilder;

    // up-vector of spar, initialized at first spar segment
    gp_Vec upVec;

    // check for defined rotation and print warning since it is not used in geometry code
    double rotation = m_sparCrossSection.GetRotation();
    if (fabs(rotation - 90.0) > Precision::Confusion()) {
        LOG(WARNING) << "Spar \"" << m_uID.value_or("") << "\" has a cross section rotation defined which is not supported by the geometry code right now! The angle will be ignored and the wing's z-axis is used as up-vector of the spar!";
    }

    // corner points for spar cut faces
    gp_Pnt innerPoint, outerPoint;
    gp_Vec innerUpVec, outerUpVec;
    gp_Pnt p1, p2, p3, p4;
    std::string innerPositionUID, outerPositionUID;

    for (int i = 1; i < m_sparPositionUIDs.GetSparPositionUIDCount(); i++) {
        // STEP 1: compute inner and outer midplane point and up vector for spar segment face
        // First inner point and up vector is computed, others are taken from preceding segment face
        if (i == 1) {
            innerPositionUID = m_sparPositionUIDs.GetSparPositionUID(i);
            innerPoint = GetMidplanePoint(innerPositionUID);
            innerUpVec = GetUpVector(innerPositionUID, innerPoint);
        }
        else {
            // take points from previous spar segment face
            innerPositionUID = outerPositionUID;
            innerPoint = outerPoint;
            innerUpVec = outerUpVec;
        }
        // Get outer midplane point
        outerPositionUID = m_sparPositionUIDs.GetSparPositionUID(i + 1);
        outerPoint = GetMidplanePoint(outerPositionUID);
        outerUpVec = GetUpVector(outerPositionUID, outerPoint);

        // STEP 2: Compute points for spar face used for cutting with loft
        gp_Pnt p1 = innerPoint.Translated(bboxSize * innerUpVec);
        gp_Pnt p2 = innerPoint.Translated(-bboxSize * innerUpVec);
        gp_Pnt p3 = outerPoint.Translated(bboxSize * outerUpVec);
        gp_Pnt p4 = outerPoint.Translated(-bboxSize * outerUpVec);

        // enlarge cut face for inner and outer sections
        // only extend in case the definition is not inside a section
        if (sparsNode.GetSparPositions().GetSparPosition(innerPositionUID).GetInputType() == CCPACSWingSparPosition::Eta &&
            sparsNode.GetSparPositions().GetSparPosition(innerPositionUID).GetEta() <= Precision::Confusion()) {
            gp_Vec sparDir(outerPoint, innerPoint);
            p1.Translate(bboxSize * sparDir.Normalized());
            p2.Translate(bboxSize * sparDir.Normalized());
        }
        // only extend in case the definition is not inside a section
        if (sparsNode.GetSparPositions().GetSparPosition(outerPositionUID).GetInputType() == CCPACSWingSparPosition::Eta &&
            sparsNode.GetSparPositions().GetSparPosition(outerPositionUID).GetEta() >= (1 - Precision::Confusion())) {
            gp_Vec sparDir(innerPoint, outerPoint);
            p3.Translate(bboxSize * sparDir.Normalized());
            p4.Translate(bboxSize * sparDir.Normalized());
        }

        // STEP 3: build face for cutting with loft
        TopoDS_Shape sparCutFace = BuildFace(p1, p2, p3, p4);

        // STEP 4: add face to split geometry compound
        cutGeomSewer.Add(sparCutFace);

        // STEP 5: compute part of midplane line (required by rib definition)
        TopoDS_Wire sparMidplaneLinePart = wingStructureReference.GetMidplaneLine(innerPoint, outerPoint);
        sparMidplaneLineBuilder.Add(sparMidplaneLinePart);
    }

    auxGeomCache.emplace();
    // store spar midplane line (required for rib position computation)
    auxGeomCache->sparMidplaneLine = sparMidplaneLineBuilder.Wire();
    // and generate sewed cut geometry
    cutGeomSewer.Perform();
    auxGeomCache->sparCutShape = cutGeomSewer.SewedShape();
}

void CCPACSWingSparSegment::BuildGeometry() const
{
    CTiglWingStructureReference wingStructureReference = sparsNode.GetParent()->GetWingStructureReference();

    // build compound for spar geometry
    BRepBuilderAPI_Sewing sewing;

    // get bounding box of loft
    TopoDS_Shape loft = wingStructureReference.GetLoft()->Shape();
    Bnd_Box bbox;
    BRepBndLib::Add(loft, bbox);
    double bboxSize = sqrt(bbox.SquareExtent());

    // iterate over all spar cut faces
    TopoDS_Shape sparCutGeometry = GetSparCutGeometry(WING_COORDINATE_SYSTEM);
    TopExp_Explorer exp;
    for (exp.Init(sparCutGeometry, TopAbs_FACE); exp.More(); exp.Next()) {
        TopoDS_Face sparCutFace = TopoDS::Face(exp.Current());

        // intersect spar cut face with loft
        TopoDS_Shape sparCutEdges = CutShapes(loft, sparCutFace);

        // build wires out of connected edges
        TopTools_ListOfShape wireList;
        BuildWiresFromConnectedEdges(sparCutEdges, wireList);

        // build face(s) for spar
        TopoDS_Shape sparFaces;
        if (wireList.Extent() == 1) {
            TopoDS_Wire sparWire = CloseWire(TopoDS::Wire(wireList.First()));
            sparFaces = BuildFace(sparWire);
        } else if (wireList.Extent() == 2) {
            sparFaces = BuildFace(TopoDS::Wire(wireList.First()), TopoDS::Wire(wireList.Last()));
        } else {
            LOG(ERROR) << "no geometry for spar definition found!";
            throw CTiglError("no geometry for spar definition found!");
        }

        // add spar face to compound
        sewing.Add(sparFaces);
    }

    // return spar geometry
    sewing.Perform();
    geometryCache.emplace();
    geometryCache->shape = sewing.SewedShape();
}

// Builds the spar geometry splitted with the ribs
void CCPACSWingSparSegment::BuildSplittedSparGeometry() const
{
    // first build the spar geometry itself
    if (!geometryCache) {
        BuildGeometry();
    }
    assert(geometryCache);
    // initialize splitted spar geometry with unsplitted one
    TopoDS_Shape splittedSparGeometry = geometryCache.value().shape;

    // next iterate over all ribs from the component segment
    const CCPACSWingCSStructure& structure = *sparsNode.GetParent();
    int numRibs = structure.GetRibsDefinitionCount();
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    for (int i = 1; i <= numRibs; i++) {
        // get the split geometry from the ribs
        const tigl::CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(i);
        for (int k = 0; k < ribsDefinition.GetNumberOfRibs(); k++) {
            const tigl::CCPACSWingRibsDefinition::CutGeometry& cutGeom = ribsDefinition.GetRibCutGeometry(k + 1);
            builder.Add(compound, cutGeom.shape);
        }
    }

    // split the spar geometry with the rib split geometry
    splittedSparGeometry = SplitShape(splittedSparGeometry, compound);

    splittedGeomCache.emplace();
    splittedGeomCache->shape = splittedSparGeometry;
}

void CCPACSWingSparSegment::BuildSparCapsGeometry() const
{
    CTiglWingStructureReference wingStructureReference = sparsNode.GetParent()->GetWingStructureReference();
    TopoDS_Compound upperCompound;
    TopoDS_Compound lowerCompound;
    BRep_Builder builder;
    builder.MakeCompound(upperCompound);
    builder.MakeCompound(lowerCompound);

    TopoDS_Shape sparCutting = GetSparCutGeometry(WING_COORDINATE_SYSTEM);

    if (m_sparCrossSection.GetUpperCap()) {
        TopoDS_Shape loft = wingStructureReference.GetUpperShape();

        // Get the cutting edge of the spar cutting plane and the loft
        TopExp_Explorer ExpEdges;

        TopoDS_Shape result = CutShapes(loft, sparCutting);

        TopoDS_Edge edge;
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {
            builder.Add(upperCompound, TopoDS::Edge(ExpEdges.Current()));
        }
    }

    if (m_sparCrossSection.GetLowerCap()) {
        TopoDS_Shape loft = wingStructureReference.GetLowerShape();

        // Get the cutting edge of the spar cutting plane and the loft
        TopExp_Explorer ExpEdges;

        TopoDS_Shape result = CutShapes(loft, sparCutting);

        TopoDS_Edge edge;
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {
            builder.Add(lowerCompound, TopoDS::Edge(ExpEdges.Current()));
        }
    }

    sparCapsCache.emplace();
    sparCapsCache->upperCapsShape = upperCompound;
    sparCapsCache->lowerCapsShape = lowerCompound;
}

// Builds the cutting geometry for the spar as well as the midplane line
gp_Pnt CCPACSWingSparSegment::GetMidplanePoint(const std::string& positionUID) const
{
    gp_Pnt midplanePoint;
    CCPACSWingSparPosition& position = sparsNode.GetSparPositions().GetSparPosition(positionUID);
    CTiglWingStructureReference wingStructureReference = sparsNode.GetParent()->GetWingStructureReference();

    if (position.GetInputType() == CCPACSWingSparPosition::ElementUID) {
        CCPACSWingComponentSegment& componentSegment = wingStructureReference.GetWingComponentSegment();
        midplanePoint = getSectionElementChordlinePoint(componentSegment, position.GetElementUID(), position.GetXsi());
    }
    else if (position.GetInputType() == CCPACSWingSparPosition::Eta) {
        midplanePoint = wingStructureReference.GetPoint(position.GetEta(), position.GetXsi(), WING_COORDINATE_SYSTEM);
    }
    else {
        throw CTiglError("Unkwnonw SparPosition InputType found in CCPACSWingSparSegment::GetMidplanePoint");
    }
    return midplanePoint;
}

gp_Vec CCPACSWingSparSegment::GetUpVector(const std::string& positionUID, gp_Pnt midplanePnt) const
{
    gp_Vec upVec;
    CCPACSWingSparPosition& position = sparsNode.GetSparPositions().GetSparPosition(positionUID);
    CTiglWingStructureReference wingStructureReference = sparsNode.GetParent()->GetWingStructureReference();

    if (position.GetInputType() == CCPACSWingSparPosition::ElementUID) {
        // get componentSegment required for getting chordline points of sections
        CCPACSWingComponentSegment& componentSegment = wingStructureReference.GetWingComponentSegment();

        // compute bounding box of section element face
        TopoDS_Shape sectionFace = componentSegment.GetSectionElementFace(position.GetElementUID());
        Bnd_Box bbox;
        BRepBndLib::Add(sectionFace, bbox);
        double sectionFaceSize = sqrt(bbox.SquareExtent());

        // generate a cut face aligned in the YZ plane
        gp_Pnt p1 = midplanePnt.Translated(gp_Vec(0, -sectionFaceSize, -sectionFaceSize));
        gp_Pnt p2 = midplanePnt.Translated(gp_Vec(0, -sectionFaceSize,  sectionFaceSize));
        gp_Pnt p3 = midplanePnt.Translated(gp_Vec(0,  sectionFaceSize, -sectionFaceSize));
        gp_Pnt p4 = midplanePnt.Translated(gp_Vec(0,  sectionFaceSize,  sectionFaceSize));

        // build face for cutting with the section face
        TopoDS_Shape cutFace = BuildFace(p1, p2, p3, p4);

        // cut faces with section face for getting the up vector line
        TopoDS_Shape cutLine = CutShapes(sectionFace, cutFace);

        // next get the two end points of the resulting cut line
        TopTools_ListOfShape endVertices;
        GetEndVertices(cutLine, endVertices);
        if (endVertices.Extent() != 2) {
            LOG(ERROR) << "Error computing up vector for section element: incorrect result of intersection!";
            throw CTiglError("Error computing up vector for section element: incorrect result of intersection!");
        }
        const TopoDS_Vertex& vCut1 = TopoDS::Vertex(endVertices.First());
        const TopoDS_Vertex& vCut2 = TopoDS::Vertex(endVertices.Last());
        gp_Pnt pCut1 = BRep_Tool::Pnt(vCut1);
        gp_Pnt pCut2 = BRep_Tool::Pnt(vCut2);

        // build the up vector based on the end points, and ensure correct orientation
        upVec = gp_Vec(pCut1, pCut2).Normalized();
        if (upVec.Dot(gp_Vec(0, 0, 1)) < 0) {
            upVec.Reverse();
        }

        return upVec;
    }

    // BUG #149 and #152
    // because of issues with the spar up vectors in adjacent component
    // segments the up vector is set to the z direction
    upVec = gp_Vec(0,0,1);
    /*
    // determine up-vector based on midplane line of inner spar point
    double eta = position.GetEta();
    gp_Pnt pl = componentSegment.GetMidplanePoint(eta, 0);
    gp_Pnt pt = componentSegment.GetMidplanePoint(eta, 1);
    gp_Vec chordLine(pl, pt);
    // determine default segment, in case of inner/outer eta value
    // (required for extended eta line)
    std::string defaultSegmentUID;
    if (eta < 0.5) {
        defaultSegmentUID = componentSegment.GetInnerSegmentUID();
    } else {
        defaultSegmentUID = componentSegment.GetOuterSegmentUID();
    }
    gp_Vec leDir = componentSegment.GetLeadingEdgeDirection(pl, defaultSegmentUID);
    leDir.SetX(0);
    leDir.Normalize();
    double rotation = sparCrossSection->GetRotation() * M_PI / 180.0;
    // determine up-vector by rotating the chordLine by the defined rotation angle,
    // invert the result because after the rotation the vector shows downwards
    upVec = -1 * chordLine.Rotated(gp_Ax1(gp_Pnt(0,0,0), leDir), rotation);
    upVec.Normalize();
    */
    
    return upVec;
}

PNamedShape CCPACSWingSparSegment::BuildLoft()
{
    return PNamedShape(new CNamedShape(GetSparGeometry(GLOBAL_COORDINATE_SYSTEM), GetDefaultedUID()));
}

} // end namespace tigl
