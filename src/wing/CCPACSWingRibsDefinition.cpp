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

void CCPACSWingRibsDefinition::Invalidate()
{
    ribSetDataCache.clear();
    auxGeomCache.clear();
    ribGeometryCache.clear();
    splittedRibGeomCache.clear();
    ribCapsCache.clear();
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
        throw CTiglError("Invalid rib number requested in ribs definition \"" + m_uID.value_or("") + "\"");
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
        LOG(ERROR) << "No rib caps are defined for ribs definition " << m_uID.value_or("");
        throw CTiglError("Error in CCPACSWingRibsDefinition::GetRibCapsGeometry(): No rib caps are defined for ribs definition " + m_uID.value_or("") + "!");
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

        // STEP 3: determine elementUID or sparPositionUID where rib should be placed
        std::string elementUID = "";
        if (i == 0 && m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_START) {
            elementUID = *m_ribsPositioning_choice1->GetElementStartUID_choice2();
        }
        // NOTE: we have to check the eta difference here (instead of the index) to support spacing definitions
        else if (fabs(ribSetDataCache->referenceEtaEnd - currentEta) <= Precision::Confusion() && m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_END) {
            elementUID = *m_ribsPositioning_choice1->GetElementEndUID_choice2();
        }

        std::string sparPositionUID = "";
        if (i == 0 && m_ribsPositioning_choice1.value().GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_START) {
            sparPositionUID = m_ribsPositioning_choice1.value().GetSparPositionStartUID_choice3().value();
        }
        // NOTE: we have to check the eta difference here (instead of the index) to support spacing definitions
        else if (fabs(ribSetDataCache->referenceEtaEnd - currentEta) <= Precision::Confusion() && m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_END) {
            sparPositionUID = m_ribsPositioning_choice1.value().GetSparPositionEndUID_choice3().value();
        }

        // STEP 4: build the rib cut geometry based on the current eta value
        //         and the element UID (if defined)
        CutGeometry cutGeom = BuildRibCutGeometry(currentEta, elementUID, sparPositionUID, cache);
        cache.cutGeometries.push_back(cutGeom);
    }
}

CCPACSWingRibsDefinition::CutGeometry CCPACSWingRibsDefinition::BuildRibCutGeometry(double currentEta, const std::string& elementUID, const std::string& sparPositionUID, AuxiliaryGeomCache& cache) const
{
    std::string ribStart = m_ribsPositioning_choice1.value().GetRibStart();
    std::string ribEnd = m_ribsPositioning_choice1.value().GetRibEnd();
    std::string ribReference = m_ribsPositioning_choice1.value().GetRibReference();

    // handle case when rib lies within a section element (elementUID defined)
    if (!elementUID.empty()) {
        TopoDS_Face ribFace = GetSectionRibGeometry(elementUID, currentEta, ribStart, ribEnd);
        // Compute rib start and end point for the cell definition
        RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribFace);
        cache.midplanePoints.push_back(midplanePoints);
        return CutGeometry(ribFace, true);
    }

    // handle case when rib lies within inner or outer section element
    if (currentEta < Precision::Confusion() || currentEta > 1 - Precision::Confusion()) {
        if (ribReference == "leadingEdge" || ribReference == "trailingEdge" || IsOuterSparPointInSection(ribReference, currentEta, getStructure())) {
            TopoDS_Face ribFace = GetSectionRibGeometry("", currentEta, ribStart, ribEnd);
            // Compute rib start and end point for the cell definition
            RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribFace);
            cache.midplanePoints.push_back(midplanePoints);
            return CutGeometry(ribFace, true);
        }
    }

    // otherwise rib cut face must be built
    // STEP 1: compute the reference point for the rib
    gp_Pnt referencePnt = GetReferencePoint(getStructure(), ribReference, currentEta);

    // STEP 2: compute the up vector for the rib cut face without x rotation
    gp_Vec upVec = GetUpVectorWithoutXRotation(m_ribsPositioning_choice1.value().GetRibReference(), currentEta, referencePnt, sparPositionUID, getStructure());

    // STEP 3: compute direction vector of rib (points from start point to end point)
    gp_Vec ribDir = GetRibDirection(currentEta, referencePnt, upVec);

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

void CCPACSWingRibsDefinition::BuildAuxGeomExplicitRibPositioning(AuxiliaryGeomCache& cache) const
{
    gp_Pnt startPnt, endPnt;
    const CTiglWingStructureReference wsr(getStructure());

    // get values from m_ribExplicitPositioning_choice2
    std::string startReference = m_ribExplicitPositioning_choice2.value().GetStartReference();
    std::string endReference = m_ribExplicitPositioning_choice2.value().GetEndReference();
    double startEta = m_ribExplicitPositioning_choice2.value().GetEtaStart();
    double endEta = m_ribExplicitPositioning_choice2.value().GetEtaEnd();

    // Step 1: get start point of rib on midplane
    startPnt = GetReferencePoint(getStructure(), startReference, startEta);
    endPnt = GetReferencePoint(getStructure(), endReference, endEta);

    // Step 2: Save the Rib start and end point (required for cells)
    cache.midplanePoints.push_back(RibMidplanePoints(startPnt, endPnt));

    // Step 3: check whether rib lies within section
    //         (use section face as rib geometry)
    if ((startEta < Precision::Confusion() && endEta < Precision::Confusion()) ||
        (startEta > 1 - Precision::Confusion() && endEta > 1 - Precision::Confusion())) {
        if ((startReference == "leadingEdge" || startReference == "trailingEdge" || IsOuterSparPointInSection(startReference, startEta, getStructure())) &&
            (endReference == "leadingEdge" || endReference == "trailingEdge" || IsOuterSparPointInSection(endReference, endEta, getStructure()))) {
            TopoDS_Face ribFace = GetSectionRibGeometry("", startEta, startReference, endReference);
            CutGeometry cutGeom(ribFace, true);
            cache.cutGeometries.push_back(cutGeom);
            return;
        }
    }

    // Step 4: compute up vectors in start and end point
    gp_Vec upVecStart(0, 0, 0), upVecEnd(0, 0, 0);
    if ((startReference == "leadingEdge" || startReference == "trailingEdge") &&
        (endReference == "leadingEdge" || endReference == "trailingEdge")) {
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
        upVecStart = wsr.GetMidplaneNormal(midplaneEta);
        upVecEnd = upVecStart;
    }
    else {
        // check whether the startReference is a spar
        if (startReference != "leadingEdge" && startReference != "trailingEdge") {
            const CCPACSWingSparSegment& spar = getStructure().GetSparSegment(startReference);
            TopoDS_Shape sparShape = spar.GetSparGeometry(WING_COORDINATE_SYSTEM);
            double midplaneEta, dummy;
            wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
            gp_Vec midplaneNormal = wsr.GetMidplaneNormal(midplaneEta);
            gp_Vec xDir(1, 0, 0);
            gp_Vec cutPlaneNormal = midplaneNormal.Crossed(xDir).Normalized();
            gp_Pln cutPlane(endPnt, cutPlaneNormal);
            TopoDS_Face cutPlaneFace = BRepBuilderAPI_MakeFace(cutPlane);

            BRepAlgoAPI_Section splitter(sparShape, cutPlaneFace, Standard_False);
            splitter.ComputePCurveOn1(Standard_True);
            splitter.Approximation(Standard_True);
            splitter.Build();
            if (!splitter.IsDone()) {
                LOG(ERROR) << "Error cutting shapes!";
                throw CTiglError("Error cutting shapes!");
            }
            TopoDS_Shape cutLine = splitter.Shape();

            gp_Pnt minPnt(0, 0, 0), maxPnt(0, 0, 0);
            GetMinMaxPoint(cutLine, gp_Vec(0, 0, 1), minPnt, maxPnt);
            upVecStart = gp_Vec(minPnt, maxPnt);
        }
        // check whether the endReference is a spar
        if (endReference != "leadingEdge" && endReference != "trailingEdge") {
            const CCPACSWingSparSegment& spar = getStructure().GetSparSegment(endReference);
            TopoDS_Shape sparShape = spar.GetSparGeometry(WING_COORDINATE_SYSTEM);
            double midplaneEta, dummy;
            wsr.GetEtaXsiLocal(endPnt, midplaneEta, dummy);
            gp_Vec midplaneNormal = wsr.GetMidplaneNormal(midplaneEta);
            gp_Vec xDir(1, 0, 0);
            gp_Vec cutPlaneNormal = midplaneNormal.Crossed(xDir).Normalized();
            gp_Pln cutPlane(endPnt, cutPlaneNormal);
            TopoDS_Face cutPlaneFace = BRepBuilderAPI_MakeFace(cutPlane);
            TopoDS_Shape cutLine = CutShapes(sparShape, cutPlaneFace);
            gp_Pnt minPnt(0, 0, 0), maxPnt(0, 0, 0);
            GetMinMaxPoint(cutLine, gp_Vec(0, 0, 1), minPnt, maxPnt);
            upVecEnd = gp_Vec(minPnt, maxPnt);
        }
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
    }

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
    TopoDS_Face ribCutFace = BuildRibCutFace(startPnt, ribDir, startReference, endReference, upVecStart, upVecEnd);

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
                    LOG(ERROR) << "unable to generate rib face for rib definition: " << m_uID.value_or("");
                    throw CTiglError("unable to generate rib face for rib definition \"" + m_uID.value_or("") + "\"! Please check for a correct rib definition!");
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
                LOG(ERROR) << "unable to generate rib face for rib definition: " << m_uID.value_or("");
                std::stringstream ss;
                ss << "Error: unable to generate rib face for rib definition: " << m_uID.value_or("");
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
    const CTiglWingStructureReference wsr(getStructure());
    TopoDS_Wire referenceLine;
    std::string ribReference = m_ribsPositioning_choice1.value().GetRibReference();
    if (ribReference == "leadingEdge") {
        referenceLine = wsr.GetLeadingEdgeLine();
    }
    else if (ribReference == "trailingEdge") {
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

    const CTiglWingStructureReference wsr(getStructure());
    if (m_ribsPositioning_choice1.value().GetStartDefinitionType() == CCPACSWingRibsPositioning::ETA_START) {
        return m_ribsPositioning_choice1.value().GetEtaStart_choice1().value();
    }
    else if (m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_START) {
        return ComputeSectionElementEta(m_ribsPositioning_choice1.value().GetElementStartUID_choice2().value());
    }
    else if (m_ribsPositioning_choice1.value().GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_START) {
        return ComputeSparPositionEta(m_ribsPositioning_choice1.value().GetSparPositionStartUID_choice3().value());
    }
    else {
        throw CTiglError("Unknown StartDefinitionType found for RibsPositioning in CCPACSWingRibsDefinition::GetEtaStart");
    }
}

double CCPACSWingRibsDefinition::ComputeReferenceEtaEnd() const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    const CTiglWingStructureReference wsr(getStructure());
    if (m_ribsPositioning_choice1.value().GetEndDefinitionType() == CCPACSWingRibsPositioning::ETA_END) {
        return m_ribsPositioning_choice1.value().GetEtaEnd_choice1().value();
    }
    else if (m_ribsPositioning_choice1.value().GetEndDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_END) {
        return ComputeSectionElementEta(m_ribsPositioning_choice1.value().GetElementEndUID_choice2().value());
    }
    else if (m_ribsPositioning_choice1.value().GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_END) {
        return ComputeSparPositionEta(m_ribsPositioning_choice1.value().GetSparPositionEndUID_choice3().value());
    }
    else {
        throw CTiglError("Unknown EndDefinitionType found for RibsPositioning in CCPACSWingRibsDefinition::GetEtaEnd");
    }
}

double CCPACSWingRibsDefinition::ComputeSectionElementEta(const std::string& sectionElementUID) const
{
    CTiglWingStructureReference wsr(getStructure());
    if (wsr.GetType() != CTiglWingStructureReference::ComponentSegmentType)
        throw CTiglError("ComputeSectionElementEta is not implemented for trailing edge devices");

    // get component segment required for getting section element face
    const CCPACSWingComponentSegment& cs = wsr.GetWingComponentSegment();

    // get the section element face
    TopoDS_Face sectionFace = cs.GetSectionElementFace(sectionElementUID);

    // next get the reference line
    TopoDS_Wire referenceLine = GetReferenceLine();

    // compute intersection point between reference line and section element
    gp_Pnt intersectionPoint;
    if (!GetIntersectionPoint(sectionFace, referenceLine, intersectionPoint)) {
        LOG(ERROR) << "Computation of intersection point between rib reference line and section element failed!";
        throw CTiglError("Computation of intersection point between rib reference line and section element failed!");
    }

    // get eta
    return ProjectPointOnWire(referenceLine, intersectionPoint);
}

double CCPACSWingRibsDefinition::ComputeSparPositionEta(const std::string& sparPositionUID) const
{
    // ensure that this is only called when m_ribsPositioning_choice1 is used!!!
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    // ensure that the spar position is part of the spar reference line!!!
    CheckSparPositionOnReference(sparPositionUID, m_ribsPositioning_choice1.value().GetRibReference(), getStructure());

    // get componentSegment required for getting section element face
    const CCPACSWingSpars& spars = *getStructure().GetSpars();

    // obtain the spar position instance for the referenced UID
    const CCPACSWingSparPosition& sparPos = spars.GetSparPositions().GetSparPosition(sparPositionUID);

    // compute midplane point according to spar definition
    gp_Pnt midplanePoint = GetSparMidplanePoint(sparPos, getStructure());

    // next get the reference line
    TopoDS_Wire referenceLine = GetReferenceLine();

    // get eta on wire
    return ProjectPointOnWire(referenceLine, midplanePoint);
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

TopoDS_Face CCPACSWingRibsDefinition::GetSectionRibGeometry(const std::string& elementUID, double eta, const std::string& ribStart, const std::string& ribEnd) const
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
        // NOTE: the check whether the eta value matches to the border of the 
        // component segment (in case it is a spar eta) was done before this
        // method was called!
        // TODO: find a better way, e.g. replace eta by enum
        if (eta < Precision::Confusion()) {
            ribFace = wsr.GetInnerFace();
        }
        else {
            ribFace = wsr.GetOuterFace();
        }
    }

    // cut rib face in case it starts at spar
    if (ribStart != "leadingEdge" && ribStart != "trailingEdge") {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribStart);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to trailing edge
        ribFace = GetNearestFace(cutResult, wsr.GetPoint(eta, 1, WING_COORDINATE_SYSTEM));
    }
    
    // cut rib face in case it ends at spar
    if (ribEnd != "leadingEdge" && ribEnd != "trailingEdge") {
        // find spar with uid
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribEnd);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to leading edge
        ribFace = GetNearestFace(cutResult, wsr.GetPoint(eta, 0, WING_COORDINATE_SYSTEM));
    }

    return ribFace;
}

gp_Vec CCPACSWingRibsDefinition::GetRibDirection(double currentEta, const gp_Pnt& startPnt, const gp_Vec& upVec) const
{
    gp_Vec ribDir;
    const CTiglWingStructureReference wsr(getStructure());

    std::string ribReference = m_ribsPositioning_choice1.value().GetRibReference();
    double zRotation = m_ribsPositioning_choice1.value().GetRibRotation().GetZ() * M_PI / 180.0;

    boost::optional<ECPACSRibRotation_ribRotationReference> ribRotationReference;
    if (m_ribsPositioning_choice1.value().GetRibRotation().GetRibRotationReference())
        ribRotationReference = m_ribsPositioning_choice1.value().GetRibRotation().GetRibRotationReference();

    if (!ribRotationReference) {
        double midplaneEta, dummy;
        wsr.GetEtaXsiLocal(startPnt, midplaneEta, dummy);
        ribDir = wsr.GetMidplaneEtaDir(midplaneEta);
    }
    else if (ribRotationReference == LeadingEdge) {
        ribDir = wsr.GetLeadingEdgeDirection(startPnt);
    }
    else if (ribRotationReference == TrailingEdge) {
        ribDir = wsr.GetTrailingEdgeDirection(startPnt);
    }
    else if (ribRotationReference == globalY) {
        // rotate y-axis around z-axis by zRotation in order to get rib direction
        ribDir = gp_Vec(0, 1, 0);
    }
    else if (ribRotationReference == globalX) {
        // rotate x-axis around z-axis by zRotation in order to get rib direction
        // The -1 compensates for the following multiplocation with -1
        ribDir = gp_Vec(-1, 0, 0);
    }
    else {
        // rotation computation only implemented for ribs which have the
        // reference point in the same spar, otherwise computation could
        // be difficult
        const std::string ribRotationReferenceStr = generated::CPACSRibRotation_ribRotationReferenceToString(*ribRotationReference);
        if (ribReference != ribRotationReferenceStr) {
            LOG(ERROR) << "using spar as rib rotation reference but not as rib reference is not supported!";
            throw CTiglError("using spar as rib rotation reference but not as rib reference is not supported!");
        }
        const CCPACSWingSparSegment& sparSegment = getStructure().GetSparSegment(ribRotationReferenceStr);
        ribDir = sparSegment.GetDirection(currentEta);
    }

    // rotate rib direction by z rotation around up vector
    // special handling for globalY, by default the zRotation defines the rotation around the up-vector
    if (ribRotationReference == globalY || ribRotationReference == globalX) {
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
    if (ribStart != "leadingEdge" && ribStart != "trailingEdge") {
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
            throw CTiglError("Geometric intersection of Rib \"" + m_uID.value_or("") + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
        }
    }
    if (ribEnd != "leadingEdge" && ribEnd != "trailingEdge") {
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
            throw CTiglError("Geometric intersection of Rib \"" + m_uID.value_or("") + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
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
    return GetUID().value_or("Unknown_Ribs_Definition");
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
