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

#include "CCPACSWingRibsDefinition.h"

#include <assert.h>

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFill.hxx>
#include <BRepTools.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GEOMAlgo_Splitter.hxx>
#include <gp_Pln.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include "CCPACSWing.h"
#include "CCPACSWingRibCrossSection.h"
#include "CCPACSWingRibRotation.h"
#include "CCPACSWingRibsPositioning.h"
#include "CCPACSWingSparPosition.h"
#include "CCPACSWingSpars.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"
#include "tiglwingribhelperfunctions.h"
#include "CNamedShape.h"
#include "CCPACSWingSegment.h"


namespace tigl
{

CCPACSWingRibsDefinition::CCPACSWingRibsDefinition(CCPACSWingRibsDefinitions* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingRibsDefinition(parent, uidMgr)
    , ribSetDataCache(*this, &CCPACSWingRibsDefinition::UpdateRibSetDataCache)
    , auxGeomCache(*this, &CCPACSWingRibsDefinition::BuildAuxiliaryGeometry)
    , ribGeometryCache(*this, &CCPACSWingRibsDefinition::BuildGeometry)
    , splittedRibGeomCache(*this, &CCPACSWingRibsDefinition::BuildSplittedRibsGeometry)
    , ribCapsCache(*this, &CCPACSWingRibsDefinition::BuildRibCapsGeometry)
{
    Invalidate();
}

void CCPACSWingRibsDefinition::InvalidateImpl(const boost::optional<std::string>& source) const
{
    ribSetDataCache.clear();
    auxGeomCache.clear();
    ribGeometryCache.clear();
    splittedRibGeomCache.clear();
    ribCapsCache.clear();
    InvalidateReferencesTo(GetUID(), m_uidMgr);
}

CCPACSWingRibsDefinition::RibPositioningType CCPACSWingRibsDefinition::GetRibPositioningType() const
{
    if (m_ribsPositioning_choice1)
        return RIBS_POSITIONING;
    else if (m_ribExplicitPositioning_choice2)
        return RIB_EXPLICIT_POSITIONING;
    else
        return UNDEFINED_POSITIONING;
}

int CCPACSWingRibsDefinition::GetNumberOfRibs() const
{
    int numberOfRibs = 0;
    switch (GetRibPositioningType()) {
    case RIB_EXPLICIT_POSITIONING:
        numberOfRibs = 1;
        break;
    case RIBS_POSITIONING:
        numberOfRibs = ribSetDataCache->numberOfRibs;
        break;
    default:
        throw CTiglError("Unknown ribPositioningType found in CCPACSWingRibsDefinition::GetNumberOfRibs()!");
    }
    return numberOfRibs;
}

void CCPACSWingRibsDefinition::GetRibMidplanePoints(int ribNumber, gp_Pnt& startPoint, gp_Pnt& endPoint) const
{
    int index = ribNumber - 1;
    if (index < 0 || index >= GetNumberOfRibs()) {
        throw CTiglError("Invalid rib number requested in ribs definition \"" + m_uID + "\"");
    }
    startPoint = auxGeomCache->midplanePoints[index].startPnt;
    endPoint = auxGeomCache->midplanePoints[index].endPnt;
}

TopoDS_Shape CCPACSWingRibsDefinition::GetRibsGeometry(TiglCoordinateSystem referenceCS) const
{
    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return ribGeometryCache->shape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(getStructure(), ribGeometryCache->shape);
        break;
    default:
        throw CTiglError("Invalid coordinateSystem passed to CCPACSWingRibsDefinition::GetRibsGeometry");
    }
}

const CCPACSWingRibsDefinition::CutGeometry& CCPACSWingRibsDefinition::GetRibCutGeometry(int ribNumber) const
{
    int index = ribNumber - 1;
    if (index < 0 || index >= GetNumberOfRibs()) {
        LOG(ERROR) << "invalid rib number passed to CCPACSWingRibsDefinition::GetRibCutGeometry!";
        throw CTiglError("invalid rib number passed to CCPACSWingRibsDefinition::GetRibCutGeometry!");
    }
    return auxGeomCache->cutGeometries[index];
}

TopoDS_Face CCPACSWingRibsDefinition::GetRibFace(int ribNumber, TiglCoordinateSystem referenceCS) const
{
    int index = ribNumber - 1;
    if (index < 0 || index >= GetNumberOfRibs()) {
        std::stringstream ss;
        ss << "Rib number " << ribNumber << " does not exist in RibsDefinition (requested from CCPACSWingRibsDefinition::GetRibFace)";
        LOG(ERROR) << ss.str();
        throw CTiglError(ss.str());
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return ribGeometryCache->ribFaces[index];
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Face(ApplyWingTransformation(getStructure(), ribGeometryCache->ribFaces[index]));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingRibsDefinition::GetRibFace");
    }
}

TopoDS_Shape CCPACSWingRibsDefinition::GetSplittedRibsGeometry(TiglCoordinateSystem referenceCS) const
{
    TopoDS_Shape splittedRibShape = splittedRibGeomCache->shape;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return splittedRibShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(getStructure(), splittedRibShape);
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingRibsDefinition::GetSplittedRibsGeometry");
    }
}

bool CCPACSWingRibsDefinition::HasCaps() const
{
    return (m_ribCrossSection.GetUpperCap() || m_ribCrossSection.GetLowerCap());
}

TopoDS_Shape CCPACSWingRibsDefinition::GetRibCapsGeometry(RibCapSide side, TiglCoordinateSystem referenceCS) const
{
    if (!HasCaps()) {
        LOG(ERROR) << "No rib caps are defined for ribs definition " << m_uID;
        throw CTiglError("Error in CCPACSWingRibsDefinition::GetRibCapsGeometry(): No rib caps are defined for ribs definition " + m_uID + "!");
    }

    TopoDS_Shape capsShape;
    if (side == UPPER) {
        capsShape = ribCapsCache->upperCapsShape;
    }
    else { // side == LOWER
        capsShape = ribCapsCache->lowerCapsShape;
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return capsShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(getStructure(), capsShape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingRibsDefinition::GetRibCapsGeometry!");
    }
}

void CCPACSWingRibsDefinition::UpdateRibSetDataCache(RibSetDataCache& cache) const
{
    // ensure that this is only called when m_ribsPositioning_choice1 is used!!!
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    // compute eta/xsi values of ribs definition
    cache.referenceEtaStart = ComputeReferenceEtaStart();
    cache.referenceEtaEnd = ComputeReferenceEtaEnd();

    // compute the offset between the ribs
    cache.referenceEtaOffset = ComputeEtaOffset(cache.referenceEtaStart, cache.referenceEtaEnd);

    // compute number of ribs
    cache.numberOfRibs = ComputeNumberOfRibs(cache.referenceEtaStart, cache.referenceEtaEnd);
}

void CCPACSWingRibsDefinition::BuildAuxiliaryGeometry(AuxiliaryGeomCache& cache) const
{
    switch (GetRibPositioningType()) {
    case RIBS_POSITIONING:
        BuildAuxGeomRibsPositioning(cache);
        break;
    case RIB_EXPLICIT_POSITIONING:
        BuildAuxGeomExplicitRibPositioning(cache);
        break;
    default:
        LOG(ERROR) << "Invalid ribPositioningType found in CCPACSWingRibsDefinition::BuildAuxiliaryGeometry!";
        throw CTiglError("Invalid ribPositioningType found in CCPACSWingRibsDefinition::BuildAuxiliaryGeometry!");
    }
}

void CCPACSWingRibsDefinition::BuildAuxGeomRibsPositioning(AuxiliaryGeomCache& cache) const
{
    // STEP 1: iterate over all ribs for this rib definition
    for (int i = 0; i < ribSetDataCache->numberOfRibs; i++) {

        // STEP 2: compute the current eta value
        double currentEta = ribSetDataCache->referenceEtaStart + ribSetDataCache->referenceEtaOffset * i;

        // STEP 3: determine sparPositionUID where rib should be placed
        bool curentlyOnSparPosition = false;
        if (i == 0 && m_ribsPositioning_choice1.value().GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_STARTEND) {
            curentlyOnSparPosition = true;
        }
        // NOTE: we have to check the eta difference here (instead of the index) to support spacing definitions
        else if (fabs(ribSetDataCache->referenceEtaEnd - currentEta) <= Precision::Confusion() && m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_STARTEND) {
            curentlyOnSparPosition = true;
        }

        // STEP 4: build the rib cut geometry based on the current eta value
        //         and the element UID (if defined)
        CutGeometry cutGeom = BuildRibCutGeometry(currentEta, curentlyOnSparPosition, cache);
        cache.cutGeometries.push_back(cutGeom);
    }
}

gp_Pnt CCPACSWingRibsDefinition::GetRibReferenceMidplanePoint(const boost::optional<CCPACSEtaXsiPoint>& etaXsi,
                                                              const boost::optional<CCPACSCurvePoint>& curvePoint,
                                                              const boost::optional<std::string>& sparPosition) const
{
    if (etaXsi) {
        const CTiglWingStructureReference wsr(getStructure());
        return wsr.GetPoint(etaXsi->GetEta(), etaXsi->GetXsi(), etaXsi->GetReferenceUID(), WING_COORDINATE_SYSTEM);
    }
    else if (curvePoint){
        const TopoDS_Wire refCurve = GetReferenceLine(curvePoint->GetReferenceUID());
        return WireGetPoint(refCurve, curvePoint->GetEta());
    }
    else if(sparPosition) {
        // get componentSegment required for getting section element face
        const CCPACSWingSpars& spars = *getStructure().GetSpars();

        // obtain the spar position instance for the referenced UID
        const CCPACSWingSparPosition& sparPos = spars.GetSparPositions().GetSparPosition(*sparPosition);

        // compute midplane point according to spar definition
        return GetSparMidplanePoint(sparPos, getStructure());
    }
    else {
        throw CTiglError("Invalid choice of start / end point in rib definition '" + GetUID() + "'.");
    }
}

CCPACSWingRibsDefinition::CutGeometry CCPACSWingRibsDefinition::BuildRibCutGeometry(double currentEta, bool onSparDefined, AuxiliaryGeomCache& cache) const
{
    std::string ribStart = m_ribsPositioning_choice1.value().GetRibStart();
    std::string ribEnd = m_ribsPositioning_choice1.value().GetRibEnd();
    const std::string ribReference = m_ribsPositioning_choice1.value().GetRibReference();

    // otherwise rib cut face must be built
    // STEP 1: compute the reference point for the rib
    gp_Pnt referencePnt = GetReferencePoint(getStructure(), ribReference, currentEta);

    // STEP 2: compute the up vector for the rib cut face without x rotation
    gp_Vec upVec = GetUpVectorWithoutXRotation(m_ribsPositioning_choice1.value().GetRibReference(), currentEta, referencePnt, onSparDefined, getStructure());

    // STEP 3: compute direction vector of rib (points from start point to end point)
    gp_Vec ribDir = GetRibDirection(referencePnt, upVec);

    // STEP 4: apply x rotation to up vector, negative to be identical to section rotation
    ApplyXRotationToUpVector(m_ribCrossSection.GetXRotation(), upVec, ribDir);

    // STEP 5: build the rib cut face
    TopoDS_Face ribCutFace = BuildRibCutFace(referencePnt, ribDir, ribStart, ribEnd, upVec, upVec);

    // STEP 6: compute the start and end point for the rib by intersection of the rib
    //         cut face with the according line (required for cell definition)
    RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribCutFace);
    cache.midplanePoints.push_back(midplanePoints);

    // finally return resulting rib cut face
    return CutGeometry(ribCutFace, false);
}

boost::optional<std::string> CCPACSWingRibsDefinition::GetElementUID(const CCPACSEtaXsiPoint& point) const
{
    CCPACSEtaXsiPoint pcopy((CCPACSWingSparPosition*)nullptr, m_uidMgr);
    pcopy.SetEta(point.GetEta());
    pcopy.SetXsi(point.GetXsi());
    pcopy.SetReferenceUID(point.GetReferenceUID());

    const CTiglWingStructureReference wsr(getStructure());
    if (wsr.GetType() == CTiglWingStructureReference::ComponentSegmentType) {
        const CCPACSWingComponentSegment& cs = wsr.GetWingComponentSegment();
        if (cs.GetUID() == point.GetReferenceUID()) {
            // convert into segment coordinates
            std::string segmentUID;
            double sEta = 0., sXsi = 0.;
            cs.GetSegmentEtaXsi(point.GetEta(), point.GetXsi(), segmentUID, sEta, sXsi);
            pcopy.SetEta(sEta);
            pcopy.SetXsi(sXsi);
            pcopy.SetReferenceUID(segmentUID);
        }
    }

    try {
        const CCPACSWingSegment& segment = GetUIDManager().ResolveObject<CCPACSWingSegment>(pcopy.GetReferenceUID());
        
        double tol = 1e-4;
        if (fabs(pcopy.GetEta()) < tol) {
            // the point lies on the inner section
            return segment.GetInnerSectionElementUID();
        }
        else if (fabs(pcopy.GetEta() - 1.) < tol) {
            // the point lies on the outer section
            return segment.GetOuterSectionElementUID();
        }
        else {
            return boost::none;
        }
    }
    catch(CTiglError&) {
        return boost::none;
    }
}

gp_Vec CCPACSWingRibsDefinition::GetRibUpVector(const CTiglWingStructureReference& wsr, gp_Pnt startPnt, gp_Pnt endPoint, bool atStart) const
{
    assert(GetRibPositioningType() == RIB_EXPLICIT_POSITIONING);
    
    const CCPACSWingRibExplicitPositioning& explicitRibPosition = m_ribExplicitPositioning_choice2.value();
    const std::string trimmingSparUID = atStart? explicitRibPosition.GetRibStart() : explicitRibPosition.GetRibEnd();

    gp_Pnt refPoint = atStart ? startPnt : endPoint;
    boost::optional<std::string> sparPositionUID = atStart ? explicitRibPosition.GetStartSparPositionUID_choice3() : explicitRibPosition.GetEndSparPositionUID_choice3();

    if (sparPositionUID) {
        // rib should be aligned to the spar position
        const CCPACSWingSparPosition& sparPosition(GetUIDManager().ResolveObject<CCPACSWingSparPosition>(sparPositionUID.value()));
        return sparPosition.GetUpVector(*wsr.GetStructure(), refPoint);
    }
    // align to section
    else if (to_lower(trimmingSparUID) != to_lower("leadingEdge") && to_lower(trimmingSparUID) != to_lower("trailingEdge")){
        const CCPACSWingSparSegment& spar = getStructure().GetSparSegment(trimmingSparUID);
        TopoDS_Shape sparShape = spar.GetSparGeometry(WING_COORDINATE_SYSTEM);
        
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(refPoint, midplaneEta, dummy);
        gp_Vec midplaneNormal = wsr.GetMidplaneNormal(midplaneEta);
        gp_Vec xDir(startPnt, endPoint);
        gp_Vec cutPlaneNormal = midplaneNormal.Crossed(xDir).Normalized();
        gp_Pln cutPlane(refPoint, cutPlaneNormal);
        TopoDS_Face cutPlaneFace = BRepBuilderAPI_MakeFace(cutPlane);
        TopoDS_Shape cutLine = CutShapes(sparShape, cutPlaneFace);
    
        gp_Pnt minPnt(0, 0, 0), maxPnt(0, 0, 0);
        GetMinMaxPoint(cutLine, gp_Vec(0, 0, 1), minPnt, maxPnt);
        return gp_Vec(minPnt, maxPnt);
    }
    else {
        // least priority. No direct up vector forced. We must handle this case outside of this function
        return gp_Vec(0., 0., 0.);
    }
}

void CCPACSWingRibsDefinition::BuildAuxGeomExplicitRibPositioning(AuxiliaryGeomCache& cache) const
{
    const CTiglWingStructureReference wsr(getStructure());
    const CCPACSWingRibExplicitPositioning& explicitRibPosition = m_ribExplicitPositioning_choice2.value();

        // Step 1: get start point of rib on midplane
    gp_Pnt startPnt = GetRibReferenceMidplanePoint(
                explicitRibPosition.GetStartEtaXsiPoint_choice1(),
                explicitRibPosition.GetStartCurvePoint_choice2(),
                explicitRibPosition.GetStartSparPositionUID_choice3());

    gp_Pnt endPnt = GetRibReferenceMidplanePoint(
                explicitRibPosition.GetEndEtaXsiPoint_choice1(),
                explicitRibPosition.GetEndCurvePoint_choice2(),
                explicitRibPosition.GetEndSparPositionUID_choice3());

    // Step 2: Save the Rib start and end point (required for cells)
    cache.midplanePoints.push_back(RibMidplanePoints(startPnt, endPnt));

    // check, if rib is defined in a section
    // This is the case, if both are defined on the same segment and both share the same eta value == 0 or 1
    boost::optional<std::string> startElementUID, endElementUID;
    if (explicitRibPosition.GetStartEtaXsiPoint_choice1()) {
        startElementUID = GetElementUID(*explicitRibPosition.GetStartEtaXsiPoint_choice1());
    }

    if (explicitRibPosition.GetEndEtaXsiPoint_choice1()) {
        endElementUID = GetElementUID(*explicitRibPosition.GetEndEtaXsiPoint_choice1());
    }

    const std::string ribStart = explicitRibPosition.GetRibStart();
    const std::string ribEnd = explicitRibPosition.GetRibEnd();

    // Step 3: check whether rib lies within section
    //         (use section face as rib geometry)
    if (startElementUID && endElementUID && *startElementUID == *endElementUID) {
        TopoDS_Face ribFace = GetSectionRibGeometry(*startElementUID, *explicitRibPosition.GetStartEtaXsiPoint_choice1(), ribStart, ribEnd);
        CutGeometry cutGeom(ribFace, true);
        cache.cutGeometries.push_back(cutGeom);
        return;
    }

    // Step 4: compute up vectors in start and end point
    // check whether the ribStart is a spar
    gp_Vec upVecStart = GetRibUpVector(wsr, startPnt, endPnt, true);
    
    // check whether the ribEnd is a spar
    gp_Vec upVecEnd = GetRibUpVector(wsr, startPnt, endPnt, false);

    // in case no up vector could be found (e.g. when intersection results in a point) use the miplane normal
    // in case only one up vector could be found, use this for both
    if (upVecStart.SquareMagnitude() == 0 && upVecEnd.SquareMagnitude() == 0) {
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
        upVecStart = wsr.GetMidplaneNormal(midplaneEta);
        upVecEnd = upVecStart;
    }
    else if (upVecStart.SquareMagnitude() == 0) {
        upVecStart = upVecEnd;
    }
    else if (upVecEnd.SquareMagnitude() == 0) {
        upVecEnd = upVecStart;
    }
    // normalize the vectors
    upVecStart.Normalize();
    upVecEnd.Normalize();


    // Step 5: rotate up vector by x rotation
    // NOTE: we need to use the global x-axis here for allowing ribs to be
    // connected, otherwise those would be rotated around different axis
    // NOTE: up-vector is already perpendicular to midplane, so rotation angle is decreased by 90 deg
    double xRotation = (m_ribCrossSection.GetXRotation() - 90) * M_PI / 180.0;
    upVecStart.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), -xRotation);
    upVecEnd.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), -xRotation);

    // Step 6: compute rib direction
    gp_Vec ribDir(startPnt, endPnt);
    ribDir.Normalize();

    // Step 7: build rib cut face (used for cutting with loft/spars)
    TopoDS_Face ribCutFace = BuildRibCutFace(startPnt, ribDir, ribStart, ribEnd, upVecStart, upVecEnd);

    // Step 8: add resulting rib cut face to cut geometries
    CutGeometry cutGeom(ribCutFace, false);
    cache.cutGeometries.push_back(cutGeom);
}

void CCPACSWingRibsDefinition::BuildGeometry(RibGeometryCache& cache) const
{
    // build target compound for ribs
    TopoDS_Compound compound;
    BRep_Builder compoundBuilder;
    compoundBuilder.MakeCompound(compound);

    TopoDS_Shape loft = CTiglWingStructureReference(getStructure()).GetLoft(WING_COORDINATE_SYSTEM)->Shape();

    // Step 3: iterate over all ribs for this rib definition
    for (int i = 0; i < GetNumberOfRibs(); i++) {
        const CutGeometry& cutGeometry = GetRibCutGeometry(i + 1);
        // handle case when ribCutFace is identical with target rib face
        if (cutGeometry.isTargetFace) {
            compoundBuilder.Add(compound, cutGeometry.shape);
            cache.ribFaces.push_back(cutGeometry.shape);
        }
        else {
            // intersect rib cut face with loft
            TopoDS_Shape ribCutEdges = CutShapes(cutGeometry.shape, loft);

            // build wires out of connected edges
            TopTools_ListOfShape wireList;
            BuildWiresFromConnectedEdges(ribCutEdges, wireList);

            // build face(s) for spar
            TopoDS_Face ribFace;
            if (wireList.Extent() == 1) {
                try {
                    TopoDS_Wire ribWire = CloseWire(TopoDS::Wire(wireList.First()));
                    ribFace = BRepBuilderAPI_MakeFace(ribWire);
                }
                catch (const CTiglError&) {
                    LOG(ERROR) << "unable to generate rib face for rib definition: " << m_uID;
                    throw CTiglError("unable to generate rib face for rib definition \"" + m_uID + "\"! Please check for a correct rib definition!");
                }
            }
            else if (wireList.Extent() == 2) {
                ribFace = BuildFace(TopoDS::Wire(wireList.First()), TopoDS::Wire(wireList.Last()));
            }
            else {
                LOG(ERROR) << "no geometry for ribs definition found!";
                throw CTiglError("no geometry for ribs definition found!");
            }

            if (ribFace.IsNull()) {
                LOG(ERROR) << "unable to generate rib face for rib definition: " << m_uID;
                std::stringstream ss;
                ss << "Error: unable to generate rib face for rib definition: " << m_uID;
                throw CTiglError(ss.str());
            }

            // add rib face to compound
            compoundBuilder.Add(compound, ribFace);
            cache.ribFaces.push_back(ribFace);
        }
    }

    cache.shape = compound;
}

// Builds the ribs geometry splitted with the spars
void CCPACSWingRibsDefinition::BuildSplittedRibsGeometry(SplittedRibGeometryCache& cache) const
{
    // split the ribs geometry with the spar split geometry
    cache.shape = CutShapeWithSpars(cache.shape, getStructure());
}

void CCPACSWingRibsDefinition::BuildRibCapsGeometry(RibCapsGeometryCache& cache) const
{
    // build up single compound of all rib cut shapes
    BRep_Builder builder;
    TopoDS_Compound ribCuttingCompound;
    builder.MakeCompound(ribCuttingCompound);
    for (int k = 0; k < GetNumberOfRibs(); k++) {
        builder.Add(ribCuttingCompound, GetRibCutGeometry(k + 1).shape);
    }

    // build caps shape for upper cap
    if (m_ribCrossSection.GetUpperCap()) {
        TopoDS_Shape loft = CTiglWingStructureReference(getStructure()).GetUpperShape();
        TopoDS_Shape cutResult = CutShapes(loft, ribCuttingCompound);
        // Get the cutting edge of the rib cutting plane and the loft
        TopoDS_Compound capEdges;
        builder.MakeCompound(capEdges);
        TopExp_Explorer explorer;
        for (explorer.Init(cutResult, TopAbs_EDGE); explorer.More(); explorer.Next()) {
            builder.Add(capEdges, TopoDS::Edge(explorer.Current()));
        }
        cache.upperCapsShape = capEdges;
    }
    // build caps shape for lower cap
    if (m_ribCrossSection.GetLowerCap()) {
        TopoDS_Shape loft = CTiglWingStructureReference(getStructure()).GetLowerShape();
        TopoDS_Shape cutResult = CutShapes(loft, ribCuttingCompound);
        // Get the cutting edge of the rib cutting plane and the loft
        TopoDS_Compound capEdges;
        builder.MakeCompound(capEdges);
        TopExp_Explorer explorer;
        for (explorer.Init(cutResult, TopAbs_EDGE); explorer.More(); explorer.Next()) {
            builder.Add(capEdges, TopoDS::Edge(explorer.Current()));
        }
        cache.lowerCapsShape = capEdges;
    }
}

TopoDS_Wire CCPACSWingRibsDefinition::GetReferenceLine() const
{
    const std::string ribReference = m_ribsPositioning_choice1.value().GetRibReference();
    return GetReferenceLine(ribReference);
}

TopoDS_Wire CCPACSWingRibsDefinition::GetReferenceLine(const std::string& ribReference) const
{
    const CTiglWingStructureReference wsr(getStructure());
    TopoDS_Wire referenceLine;
    if (to_lower(ribReference) == to_lower("leadingEdge")) {
        referenceLine = wsr.GetLeadingEdgeLine();
    }
    else if (to_lower(ribReference) == to_lower("trailingEdge")) {
        referenceLine = wsr.GetTrailingEdgeLine();
    }
    else {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribReference);
        referenceLine = sparSegment.GetSparMidplaneLine();
    }
    return referenceLine;
}

double CCPACSWingRibsDefinition::ComputeReferenceEtaStart() const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    bool isOnSpar = m_ribsPositioning_choice1.value().GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_STARTEND;

    gp_Pnt midplanePoint = GetRibReferenceMidplanePoint(
                m_ribsPositioning_choice1.value().GetStartEtaXsiPoint_choice1(),
                m_ribsPositioning_choice1.value().GetStartCurvePoint_choice2(),
                m_ribsPositioning_choice1.value().GetStartSparPositionUID_choice3());

    return ComputeEtaOnReferenceLine(midplanePoint, isOnSpar);
}

double CCPACSWingRibsDefinition::ComputeReferenceEtaEnd() const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    bool isOnSpar = m_ribsPositioning_choice1.value().GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_STARTEND;

    gp_Pnt midplanePoint = GetRibReferenceMidplanePoint(
                m_ribsPositioning_choice1.value().GetEndEtaXsiPoint_choice1(),
                m_ribsPositioning_choice1.value().GetEndCurvePoint_choice2(),
                m_ribsPositioning_choice1.value().GetEndSparPositionUID_choice3());

    return ComputeEtaOnReferenceLine(midplanePoint, isOnSpar);
}

double CCPACSWingRibsDefinition::ComputeEtaOnReferenceLine(const gp_Pnt& midplanePoint, bool onSpar) const
{
    // ensure that this is only called when m_ribsPositioning_choice1 is used!!!
    assert(GetRibPositioningType() == RIBS_POSITIONING);


    const CTiglWingStructureReference wsr(getStructure());

    // next get the reference line
    TopoDS_Wire referenceLine = GetReferenceLine();

    gp_Vec upVec(0 ,0, 1);
    if (!onSpar) {
        // use the midplane normal as up vector
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(midplanePoint, midplaneEta, dummy);
        upVec = wsr.GetMidplaneNormal(midplaneEta);
    }
    
    // TODO: check if in section, then we probably have a different up vector
    
    // compute intersection of rib with rib rotation reference
    gp_Vec ribDir = GetRibDirection(midplanePoint, upVec);
    
    // apply x rotation to up vector, negative to be identical to section rotation
    ApplyXRotationToUpVector(m_ribCrossSection.GetXRotation(), upVec, ribDir);
    
    // upDir, ribDir and midplanePoint define the rib plane.
    // Compute intersection of the plane with the reference line
    const gp_Dir normal(upVec.Crossed(ribDir).Normalized());
    const gp_Pln ribPlane(gp_Ax3(midplanePoint, normal, gp_Dir(ribDir)));
    
    const TopoDS_Face ribFace = BRepBuilderAPI_MakeFace(ribPlane).Face();

    gp_Pnt pOnRefLine;
    if (!GetIntersectionPoint(ribFace, referenceLine, pOnRefLine)) {
        // no intersection found between rib and reference line
        throw CTiglError("Error computing rib intersection with reference line");
    }
    else {
        return ProjectPointOnWire(referenceLine, pOnRefLine);
    }
}

int CCPACSWingRibsDefinition::ComputeNumberOfRibs(double etaStart, double etaEnd) const
{
    // ensure that this is only called when m_ribsPositioning_choice1 is used!!!
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    int numberOfRibs = 0;

    // check whether the number is defined in the ribs positioning
    if (m_ribsPositioning_choice1.value().GetRibCountDefinitionType() == CCPACSWingRibsPositioning::NUMBER_OF_RIBS) {
        numberOfRibs = *m_ribsPositioning_choice1.value().GetNumberOfRibs_choice2();
    }
    else if (m_ribsPositioning_choice1.value().GetRibCountDefinitionType() == CCPACSWingRibsPositioning::SPACING) {
        // otherwise compute the number based on the spacing
        double spacing = *m_ribsPositioning_choice1.value().GetSpacing_choice1();

        // handle the case when start and end eta are identical
        double deltaEta = fabs(etaEnd - etaStart);
        if (deltaEta <= Precision::Confusion()) {
            numberOfRibs = 1;
        }
        else {
            // get the length of the reference line between the two points
            double length = GetRibReferenceLength(m_ribsPositioning_choice1.value().GetRibReference(), getStructure()) * fabs(etaEnd - etaStart);
            // finally compute the number of ribs by dividing the length by the spacing
            // NOTE: adding of Precision::Confusion in order to avoid floating point problems
            numberOfRibs = (int)((length + Precision::Confusion()) / spacing) + 1;
        }
    }
    else {
        LOG(ERROR) << "Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeNumberOfRibs!";
        throw CTiglError("Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeNumberOfRibs!");
    }

    return numberOfRibs;
}

double CCPACSWingRibsDefinition::ComputeEtaOffset(double etaStart, double etaEnd) const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    double etaOffset;

    // check whether number of ribs is defined, or spacing is defined
    if (m_ribsPositioning_choice1.value().GetRibCountDefinitionType() == CCPACSWingRibsPositioning::NUMBER_OF_RIBS) {
        int numberOfRibs = *m_ribsPositioning_choice1.value().GetNumberOfRibs_choice2();
        // in case only 1 rib is defined define eta offset as 0
        if (numberOfRibs == 1) {
            etaOffset = 0;
        }
        else {
            etaOffset = fabs(etaEnd - etaStart) / (numberOfRibs - 1);
        }
    }
    else if (m_ribsPositioning_choice1.value().GetRibCountDefinitionType() == CCPACSWingRibsPositioning::SPACING) {
        double spacing = *m_ribsPositioning_choice1.value().GetSpacing_choice1();
        // get the length of the rib reference line between the two eta points
        double referenceLength = GetRibReferenceLength(m_ribsPositioning_choice1.value().GetRibReference(), getStructure());
        etaOffset = spacing / referenceLength;
    }
    else {
        LOG(ERROR) << "Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeEtaOffset!";
        throw CTiglError("Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeEtaOffset!");
    }

    return etaOffset;
}

TopoDS_Face CCPACSWingRibsDefinition::GetSectionRibGeometry(const std::string& elementUID, const CCPACSEtaXsiPoint& etaxsi, const std::string& ribStart, const std::string& ribEnd) const
{
    const CTiglWingStructureReference wsr(getStructure());
    TopoDS_Face ribFace;
    if (!elementUID.empty()) {
        if (wsr.GetType() != CTiglWingStructureReference::ComponentSegmentType)
            throw CTiglError("GetSectionRibGeometry with non-empty elementUID (" + elementUID + ") is not implemented for trailing edge devices");
        const CCPACSWingComponentSegment& cs = wsr.GetWingComponentSegment();
        ribFace = cs.GetSectionElementFace(elementUID);
    }
    else {
        throw CTiglError("Fatal Error: element uid must not be empty");
    }

    // cut rib face in case it starts at spar
    if (!ribStart.empty() && to_lower(ribStart) != to_lower("leadingEdge") && to_lower(ribStart) != to_lower("trailingEdge")) {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribStart);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to trailing edge
        gp_Pnt tePoint = wsr.GetPoint(etaxsi.GetEta(), 1., etaxsi.GetReferenceUID(), WING_COORDINATE_SYSTEM);
        ribFace = GetNearestFace(cutResult, tePoint);
    }
    
    // cut rib face in case it ends at spar
    if (!ribEnd.empty() && to_lower(ribEnd) != to_lower("leadingEdge") && to_lower(ribEnd) != to_lower("trailingEdge")) {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribEnd);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to leading edge
        gp_Pnt lePoint = wsr.GetPoint(etaxsi.GetEta(), 0., etaxsi.GetReferenceUID(), WING_COORDINATE_SYSTEM);
        ribFace = GetNearestFace(cutResult, lePoint);
    }

    return ribFace;
}

gp_Vec CCPACSWingRibsDefinition::GetRibDirection(const gp_Pnt& startPnt, const gp_Vec& upVec) const
{
    gp_Vec ribDir;
    const CTiglWingStructureReference wsr(getStructure());

    double zRotation = m_ribsPositioning_choice1.value().GetRibRotation().GetZ() * M_PI / 180.0;

    boost::optional<std::string> ribRotationReference;
    if (m_ribsPositioning_choice1.value().GetRibRotation().GetRibRotationReference())
        ribRotationReference = m_ribsPositioning_choice1.value().GetRibRotation().GetRibRotationReference();

    if (!ribRotationReference) {
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
        ribDir = wsr.GetMidplaneEtaDir(midplaneEta);
    }
    else if (to_lower(ribRotationReference.value()) == to_lower("globalY")) {
        // rotate y-axis around z-axis by zRotation in order to get rib direction
        ribDir = gp_Vec(0, 1, 0);
    }
    else if (to_lower(ribRotationReference.value()) == to_lower("globalX")) {
        // rotate x-axis around z-axis by zRotation in order to get rib direction
        // The -1 compensates for the following multiplocation with -1
        ribDir = gp_Vec(-1, 0, 0);
    }
    else {
        TopoDS_Wire referenceWire = GetReferenceLine(ribRotationReference.value());

        // project the start point on the reference line at the specified angle
        double paramOnWire = ProjectPointOnWireAtAngle(referenceWire, startPnt, upVec, zRotation);
        gp_Pnt p(0,0,0);
        WireGetPointTangent(referenceWire, paramOnWire, p, ribDir);
    }

    // rotate rib direction by z rotation around up vector
    // special handling for globalY, by default the zRotation defines the rotation around the up-vector
    if (ribRotationReference && (to_lower(ribRotationReference.value()) == to_lower("globalY") || to_lower(ribRotationReference.value()) == to_lower("globalX"))) {
        ribDir.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), zRotation);
    }
    else {
        // default behavior
        ribDir.Rotate(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(upVec)), zRotation);
    }
    // rib direction must be inverted, since CPACS specification results in 
    // a vector which points from the end point to the start point
    ribDir *= -1;

    ribDir.Normalize();

    return ribDir;
}

TopoDS_Face CCPACSWingRibsDefinition::BuildRibCutFace(const gp_Pnt& startPnt, const gp_Vec& ribDir, const std::string& ribStart,
                                                      const std::string& ribEnd, const gp_Vec& upVecStart, const gp_Vec& upVecEnd) const
{
    // STEP 1: compute the size of the bounding box
    double bboxSize = GetBoundingBoxSize(CTiglWingStructureReference(getStructure()).GetLoft()->Shape());

    // STEP 2: build initial rib cut face (used for cutting with loft/spars)
    //         extend points to ensure that the ribCutFace is larger than bounding box
    gp_Pnt p1 = startPnt.Translated(bboxSize * -ribDir);
    gp_Pnt p2 = startPnt.Translated(bboxSize * -ribDir);
    gp_Pnt p3 = startPnt.Translated(bboxSize * ribDir);
    gp_Pnt p4 = startPnt.Translated(bboxSize * ribDir);
    p1.Translate(bboxSize * upVecStart);
    p2.Translate(bboxSize * -upVecStart);
    p3.Translate(bboxSize * -upVecEnd);
    p4.Translate(bboxSize * upVecEnd);

    // build front and back wire of rib cut face and build ribCutFace out of it
    TopoDS_Wire cutFaceFrontWire = BuildWire(p1, p2);
    TopoDS_Wire cutFaceBackWire = BuildWire(p3, p4);
    TopoDS_Face ribCutFace = BuildFace(cutFaceFrontWire, cutFaceBackWire, upVecStart);

    // no need to change when starting at leading edge/trailing edge
    if (to_lower(ribStart) != to_lower("leadingEdge") && to_lower(ribStart) != to_lower("trailingEdge")) {
        // get geometry of spar
        std::string sparUid = ribStart;
        // find spar with uidC
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(sparUid);
        TopoDS_Shape sparGeometry = sparSegment.GetSparGeometry(WING_COORDINATE_SYSTEM);

        try {
            // cut the rib face with the spar, returning the front wire of the resulting rib
            CutFaceWithSpar(ribCutFace, sparGeometry, bboxSize, cutFaceFrontWire, cutFaceBackWire, upVecStart.Multiplied(-1));
        }
        catch (const CTiglError&) {
            throw CTiglError("Geometric intersection of Rib \"" + m_uID + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
        }
    }
    if (to_lower(ribEnd) != to_lower("leadingEdge") && to_lower(ribEnd) != to_lower("trailingEdge")) {
        // get geometry of spar
        std::string sparUid = ribEnd;
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(sparUid);
        TopoDS_Shape sparGeometry = sparSegment.GetSparGeometry(WING_COORDINATE_SYSTEM);

        try {
            // cut the rib face with the spar, returning the back wire of the resulting rib
            CutFaceWithSpar(ribCutFace, sparGeometry, bboxSize, cutFaceBackWire, cutFaceFrontWire, upVecEnd);
        }
        catch (const CTiglError&) {
            throw CTiglError("Geometric intersection of Rib \"" + m_uID + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
        }
    }

    return ribCutFace;
}

CCPACSWingRibsDefinition::RibMidplanePoints CCPACSWingRibsDefinition::ComputeRibDefinitionPoints(const std::string& ribStart, 
                                                                                                 const std::string& ribEnd, 
                                                                                                 const TopoDS_Face& ribCutFace) const
{
    // Compute the intersection points of the rib definition lines and the cut face
    gp_Pnt ribStartPnt = GetRibDefinitionPoint(ribStart, ribCutFace, getStructure());
    gp_Pnt ribEndPnt = GetRibDefinitionPoint(ribEnd, ribCutFace, getStructure());
    return RibMidplanePoints(ribStartPnt, ribEndPnt);
}

CCPACSWingCSStructure & CCPACSWingRibsDefinition::getStructure()
{
    return *m_parent->GetParent();
}

const CCPACSWingCSStructure & CCPACSWingRibsDefinition::getStructure() const
{
    return *m_parent->GetParent();
}

std::string CCPACSWingRibsDefinition::GetDefaultedUID() const
{
    return GetUID();
}

TiglGeometricComponentType CCPACSWingRibsDefinition::GetComponentType() const
{
    return TIGL_COMPONENT_WINGRIB;
}

TiglGeometricComponentIntent CCPACSWingRibsDefinition::GetComponentIntent() const
{
    return TIGL_INTENT_PHYSICAL | TIGL_INTENT_INNER_STRUCTURE;
}

PNamedShape CCPACSWingRibsDefinition::BuildLoft() const
{
    return PNamedShape(new CNamedShape(GetRibsGeometry(GLOBAL_COORDINATE_SYSTEM), GetDefaultedUID()));
}

} // end namespace tigl
