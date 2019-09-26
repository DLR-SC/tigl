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
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>


#include "CCPACSWing.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSparPosition.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingCSStructure.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglWingStructureReference.h"
#include "tiglcommonfunctions.h"
#include "tiglwingribhelperfunctions.h"
#include "tigletaxsifunctions.h"
#include "CNamedShape.h"


namespace tigl
{
TopoDS_Shape ApplyWingTransformation(const CCPACSWingSpars& sparsNode, const TopoDS_Shape& shape)
{
    return CTiglWingStructureReference(*sparsNode.GetParent()).GetWing().GetTransformationMatrix().Transform(shape);
}
gp_Pnt ApplyWingTransformation(const CCPACSWingSpars& sparsNode, const gp_Pnt& p)
{
    return CTiglWingStructureReference(*sparsNode.GetParent()).GetWing().GetTransformationMatrix().Transform(p);
}

CCPACSWingSparSegment::CCPACSWingSparSegment(CCPACSWingSparSegments* sparSegments, CTiglUIDManager* uidMgr)
    : generated::CPACSSparSegment(sparSegments, uidMgr), sparsNode(*sparSegments->GetParent())
    , auxGeomCache(*this, &CCPACSWingSparSegment::BuildAuxiliaryGeometry)
    , geometryCache(*this, &CCPACSWingSparSegment::BuildGeometry)
    , splittedGeomCache(*this, &CCPACSWingSparSegment::BuildSplittedSparGeometry)
    , sparCapsCache(*this, &CCPACSWingSparSegment::BuildSparCapsGeometry)
{
    Invalidate();
}

void CCPACSWingSparSegment::Invalidate()
{
    auxGeomCache.clear();
    geometryCache.clear();
    splittedGeomCache.clear();
    sparCapsCache.clear();
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
        LOG(ERROR) << "Invalid spar position index " << positionIndex << " requested from spar segment \"" << m_uID << "\"!";
        throw CTiglError("Invalid spar position index requested from spar segment \"" + m_uID + "\"!");
    }

    return GetMidplanePoint(m_sparPositionUIDs.GetSparPositionUID(positionIndex));
}

void CCPACSWingSparSegment::GetEtaXsi(int positionIndex, double& eta, double& xsi) const
{
    if (positionIndex < 1 || positionIndex > m_sparPositionUIDs.GetSparPositionUIDCount()) {
        LOG(ERROR) << "Invalid spar position index " << positionIndex << " requested from spar segment \"" << m_uID << "\"!";
        throw CTiglError("Invalid spar position index requested from spar segment \"" + m_uID + "\"!");
    }

    const std::string& sparPositionUID = m_sparPositionUIDs.GetSparPositionUID(positionIndex);
    const CCPACSWingSparPosition& sparPosition = sparsNode.GetSparPositions().GetSparPosition(sparPositionUID);

    CTiglWingStructureReference refCS(*sparsNode.GetParent());

    if (sparPosition.isOnRib() || sparPosition.GetReferenceUID() != refCS.GetUID()) {
        gp_Pnt sparPositionPoint = GetMidplanePoint(sparPositionUID);
        refCS.GetEtaXsiLocal(sparPositionPoint, eta, xsi);
    }
    else if (sparPosition.GetReferenceUID() == refCS.GetUID()) {
        eta = sparPosition.GetEta();
        xsi = sparPosition.GetXsi();
    }
    else {
        throw CTiglError("Unknown SparPosition-InputType found in CCPACSWingSparSegment::GetEtaXsi");
    }
}

TopoDS_Wire CCPACSWingSparSegment::GetSparMidplaneLine() const
{
    return auxGeomCache->sparMidplaneLine;
}

TopoDS_Shape CCPACSWingSparSegment::GetSparGeometry(TiglCoordinateSystem referenceCS) const
{
    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return geometryCache->shape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, geometryCache->shape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingSparSegment::GetSparGeometry!");
    }
}

TopoDS_Shape CCPACSWingSparSegment::GetSplittedSparGeometry(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Shape splittedSparShape;
    splittedSparShape = splittedGeomCache->shape;

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
    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return auxGeomCache->sparCutShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(sparsNode, auxGeomCache->sparCutShape);
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
    TopoDS_Shape capsShape;
    if (side == UPPER) {
        capsShape = sparCapsCache->upperCapsShape;
    }
    else { // side == LOWER
        capsShape = sparCapsCache->lowerCapsShape;
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
    return GetUID();
}

TiglGeometricComponentType CCPACSWingSparSegment::GetComponentType() const
{
    return TIGL_COMPONENT_WINGSPAR;
}

TiglGeometricComponentIntent CCPACSWingSparSegment::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL | TIGL_INTENT_INNER_STRUCTURE;
}


// Builds the cutting geometry for the spar as well as the midplane line
void CCPACSWingSparSegment::BuildAuxiliaryGeometry(AuxiliaryGeomCache& cache) const
{
    // get assigned componentsegment
    const CTiglWingStructureReference wsr(*sparsNode.GetParent());

    // build compound for cut geometry
    BRepBuilderAPI_Sewing cutGeomSewer;

    // get bounding box of loft
    TopoDS_Shape loft = wsr.GetLoft()->Shape();
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
        LOG(WARNING) << "Spar \"" << m_uID << "\" has a cross section rotation defined which is not supported by the geometry code right now! The angle will be ignored and the wing's z-axis is used as up-vector of the spar!";
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

        // enlarge cut face for inner and outer component segment sections
        double eta, xsi;
        GetEtaXsi(i, eta, xsi);
        if (i == 1 && (eta < 1.E-6 || 1 - eta < 1.E-6)) {
            p1.Translate(gp_Vec(p3, p1));
            p2.Translate(gp_Vec(p4, p2));
        }

        GetEtaXsi(i + 1, eta, xsi);
        if (i == m_sparPositionUIDs.GetSparPositionUIDCount() - 1 && (eta < 1.E-6 || 1 - eta < 1.E-6)) {
            p3.Translate(gp_Vec(p1, p3));
            p4.Translate(gp_Vec(p2, p4));
        }

        // STEP 3: build face for cutting with loft
        TopoDS_Shape sparCutFace = BuildFace(p1, p2, p3, p4);

        // STEP 4: add face to split geometry compound
        cutGeomSewer.Add(sparCutFace);

        // STEP 5: compute part of midplane line (required by rib definition)
        TopoDS_Wire sparMidplaneLinePart = wsr.GetMidplaneLine(innerPoint, outerPoint);
        sparMidplaneLineBuilder.Add(sparMidplaneLinePart);
    }

    // store spar midplane line (required for rib position computation)
    cache.sparMidplaneLine = sparMidplaneLineBuilder.Wire();
    // and generate sewed cut geometry
    cutGeomSewer.Perform();
    cache.sparCutShape = cutGeomSewer.SewedShape();
}

void CCPACSWingSparSegment::BuildGeometry(GeometryCache& cache) const
{
    CTiglWingStructureReference wsr(*sparsNode.GetParent());

    // build compound for spar geometry
    BRepBuilderAPI_Sewing sewing;

    TopoDS_Shape loft = wsr.GetLoft()->Shape();

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
    cache.shape = sewing.SewedShape();
}

// Builds the spar geometry splitted with the ribs
void CCPACSWingSparSegment::BuildSplittedSparGeometry(SplittedGeomCache& cache) const
{
    // initialize splitted spar geometry with unsplitted one
    TopoDS_Shape splittedSparGeometry = geometryCache->shape;

    // next iterate over all ribs from the component segment
    const CCPACSWingCSStructure& structure = *sparsNode.GetParent();
    int numRibs = structure.GetRibsDefinitionCount();
    // NOTE: changed implementation to separate splitting because
    //       of problems with OCC7.1 when using compound
    //BRep_Builder builder;
    //TopoDS_Compound compound;
    //builder.MakeCompound(compound);
    for (int i = 1; i <= numRibs; i++) {
        // get the split geometry from the ribs
        const tigl::CCPACSWingRibsDefinition& ribsDefinition = structure.GetRibsDefinition(i);
        for (int k = 0; k < ribsDefinition.GetNumberOfRibs(); k++) {
            const tigl::CCPACSWingRibsDefinition::CutGeometry& cutGeom = ribsDefinition.GetRibCutGeometry(k + 1);
            //builder.Add(compound, cutGeom.shape);
            // split the spar geometry with the rib split geometry
            splittedSparGeometry = SplitShape(splittedSparGeometry, cutGeom.shape);
        }
    }
    //// split the spar geometry with the rib split geometry
    //splittedSparGeometry = SplitShape(splittedSparGeometry, compound);

    // split spar geometry with all other Spars
    Bnd_Box bBoxBorder1, bBoxBorder2;
    bBoxBorder1 = Bnd_Box();
    BRepBndLib::Add(geometryCache->shape, bBoxBorder1);

    int numSpars = structure.GetSparSegmentCount();

    for (int i = 1; i <= numSpars; i++)
    {
        const tigl::CCPACSWingSparSegment& otherSegment = structure.GetSparSegment(i);

        if (m_uID == otherSegment.GetUID())
            continue;

        bBoxBorder2 = Bnd_Box();
        BRepBndLib::Add(otherSegment.GetSparGeometry(WING_COORDINATE_SYSTEM), bBoxBorder2);

        if (!bBoxBorder1.IsOut(bBoxBorder2))
        {
            splittedSparGeometry = SplitShape(splittedSparGeometry, otherSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM));
        }
    }

    cache.shape = splittedSparGeometry;
}

void CCPACSWingSparSegment::BuildSparCapsGeometry(SparCapsCache& cache) const
{
    const CTiglWingStructureReference wsr(*sparsNode.GetParent());
    TopoDS_Compound upperCompound;
    TopoDS_Compound lowerCompound;
    BRep_Builder builder;
    builder.MakeCompound(upperCompound);
    builder.MakeCompound(lowerCompound);

    TopoDS_Shape sparCutting = GetSparCutGeometry(WING_COORDINATE_SYSTEM);

    if (m_sparCrossSection.GetUpperCap()) {
        TopoDS_Shape loft = wsr.GetUpperShape();

        // Get the cutting edge of the spar cutting plane and the loft
        TopExp_Explorer ExpEdges;

        TopoDS_Shape result = CutShapes(loft, sparCutting);

        TopoDS_Edge edge;
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {
            builder.Add(upperCompound, TopoDS::Edge(ExpEdges.Current()));
        }
    }

    if (m_sparCrossSection.GetLowerCap()) {
        TopoDS_Shape loft = wsr.GetLowerShape();

        // Get the cutting edge of the spar cutting plane and the loft
        TopExp_Explorer ExpEdges;

        TopoDS_Shape result = CutShapes(loft, sparCutting);

        TopoDS_Edge edge;
        for (ExpEdges.Init(result, TopAbs_EDGE); ExpEdges.More(); ExpEdges.Next()) {
            builder.Add(lowerCompound, TopoDS::Edge(ExpEdges.Current()));
        }
    }

    cache.upperCapsShape = upperCompound;
    cache.lowerCapsShape = lowerCompound;
}

// Builds the cutting geometry for the spar as well as the midplane line
gp_Pnt CCPACSWingSparSegment::GetMidplanePoint(const std::string& positionUID) const
{
    CCPACSWingSparPosition& sparPosition = sparsNode.GetSparPositions().GetSparPosition(positionUID);

    return GetSparMidplanePoint(sparPosition, *sparsNode.GetParent());
}

gp_Vec CCPACSWingSparSegment::GetUpVector(const std::string& positionUID, gp_Pnt midplanePnt) const
{
    const CCPACSWingSparPosition& position = sparsNode.GetSparPositions().GetSparPosition(positionUID);
    return position.GetUpVector(*sparsNode.GetParent(), midplanePnt);
}

PNamedShape CCPACSWingSparSegment::BuildLoft() const
{
    return PNamedShape(new CNamedShape(GetSparGeometry(GLOBAL_COORDINATE_SYSTEM), GetDefaultedUID()));
}

bool PointIsInfrontSparGeometry(gp_Dir nNormal, gp_Pnt nTestPoint, TopoDS_Shape nSparSegments)
{
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(nSparSegments, TopAbs_FACE, faceMap);
    double u_min = 0., u_max = 0., v_min = 0., v_max = 0.;

    // for symmetry
    nTestPoint.SetY(fabs(nTestPoint.Y()));

    for (int f = 1; f <= faceMap.Extent(); f++) {
        TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
        BRepAdaptor_Surface surf(loftFace);
        Handle(Geom_Surface) geomSurf = BRep_Tool::Surface(loftFace);

        BRepTools::UVBounds(TopoDS::Face(faceMap(f)), u_min, u_max, v_min, v_max);

        gp_Pnt startPnt = surf.Value(u_min, v_min + ((v_max - v_min) / 2));
        gp_Pnt endPnt   = surf.Value(u_max, v_min + ((v_max - v_min) / 2));

        // if the U / V direction of the spar plane is changed
        gp_Ax1 a1Test0   = gp_Ax1(startPnt, gp_Vec(startPnt, endPnt));
        gp_Ax1 a1TestZ   = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., 1.)));
        gp_Ax1 a1Test_mZ = gp_Ax1(startPnt, gp_Vec(gp_Pnt(0., 0., 0.), gp_Pnt(0., 0., -1.)));

        if (a1Test0.Angle(a1TestZ) < Radians(20.0) || a1Test0.Angle(a1Test_mZ) < Radians(20.0)) {
            startPnt = surf.Value(u_min + ((u_max - u_min) / 2), v_min);
            endPnt   = surf.Value(u_min + ((u_max - u_min) / 2), v_max);
        }

        // Here it is checked if the stringer is in the Y area of the corresponding spar face

        if (endPnt.Y() > startPnt.Y()) {
            if (startPnt.Y() > nTestPoint.Y() || endPnt.Y() < nTestPoint.Y()) {
                continue;
            }
        }
        else {
            if (startPnt.Y() < fabs(nTestPoint.Y()) || endPnt.Y() > fabs(nTestPoint.Y())) {
                continue;
            }
        }

        // project test point onto the surface
        Handle(ShapeAnalysis_Surface) SA_surf = new ShapeAnalysis_Surface(geomSurf);
        gp_Pnt2d uv                           = SA_surf->ValueOfUV(nTestPoint, 0.0);
        gp_Pnt pTestProj                      = surf.Value(uv.X(), uv.Y());

        GeomLProp_SLProps prop(geomSurf, uv.X(), uv.Y(), 1, 0.01);
        gp_Dir normal = prop.Normal();

        gp_Ax1 planeNormal(pTestProj, normal);

        if (nNormal.Angle(planeNormal.Direction()) > Radians(90.0)) {
            planeNormal = planeNormal.Reversed();
        }

        gp_Vec vTest = gp_Vec(pTestProj, nTestPoint);

        if (vTest.Magnitude() == 0.) {
            continue;
        }

        gp_Ax1 a1Test = gp_Ax1(pTestProj, vTest);

        if (a1Test.Angle(planeNormal) < Radians(89.0)) {
            return true;
        }
        else {
            continue;
        }
    }

    return false;
}

bool HasRecursiveSparPositionDefined(const CCPACSWingSparPosition& position, const std::string& sparUID)
{
    // check, that in case of rib positioning, the rib does not reference the spar itself
    if (position.isOnRib()) {
        std::string ribUID = position.GetReferenceUID();
        const CCPACSWingRibsDefinition& ribs = position.GetUIDManager().ResolveObject<CCPACSWingRibsDefinition>(ribUID);
        if (ribs.GetRibsPositioning_choice1()) {
            const CCPACSWingRibsPositioning& pos = ribs.GetRibsPositioning_choice1().value();
            if (pos.GetRibEnd() == sparUID|| pos.GetRibStart() == sparUID) {
                LOG(ERROR) << "Recursive reference of spar positioning '" << position.GetUID() << "' and ribs definition '" << ribs.GetDefaultedUID() << "'.";
                return true;
            }
        }
    }
    return false;
}

void CCPACSWingSparSegment::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath)
{
    generated::CPACSSparSegment::ReadCPACS(tixiHandle, xpath);

    // The spar positioning on a rib works, as long as the referenced rib does not reference the spar
    // as the leading or trailing edge of the rib. We must test this case and throw an error otherwise
    bool hasError = false;
    for (const std::string sparPositionUID : GetSparPositionUIDs().GetSparPositionUIDs()) {
        const CCPACSWingSparPosition& position = m_uidMgr->ResolveObject<CCPACSWingSparPosition>(sparPositionUID);
        if (HasRecursiveSparPositionDefined(position, GetDefaultedUID())) {
            hasError = true;
        }
    }

    if (hasError) {
        throw CTiglError("Recursive spar/rib definition. Spar '" + GetDefaultedUID() + "' will be skipped.", TIGL_XML_ERROR);
    }
}


} // end namespace tigl
