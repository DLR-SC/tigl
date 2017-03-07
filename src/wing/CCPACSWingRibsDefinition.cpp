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


namespace tigl
{

CCPACSWingRibsDefinition::CCPACSWingRibsDefinition(CCPACSWingRibsDefinitions* parent)
: generated::CPACSWingRibsDefinition(parent), structure(*parent->GetParent()) {
    Invalidate();
}

void CCPACSWingRibsDefinition::Invalidate()
{
    ribSetDataCache.valid = false;
    auxGeomCache.valid = false;
    ribGeometryCache.valid = false;
    splittedRibGeomCache.valid = false;
    ribCapsCache.valid = false;
}

CCPACSWingRibsDefinition::RibPositioningType CCPACSWingRibsDefinition::GetRibPositioningType() const
{
    if (HasRibsPositioning_choice1())
        return RibPositioningType::RIBS_POSITIONING;
    else if (HasRibExplicitPositioning_choice2())
        return RibPositioningType::RIB_EXPLICIT_POSITIONING;
    else
        return RibPositioningType::UNDEFINED_POSITIONING;
}

const CCPACSWingRibsPositioning& CCPACSWingRibsDefinition::GetRibsPositioning() const
{
    if (!m_ribsPositioning_choice1) {
        LOG(ERROR) << "CCPACSWingRibsDefinition is not defined via m_ribsPositioning_choice1!";
        throw CTiglError("CCPACSWingRibsDefinition is not defined via m_ribsPositioning_choice1! Please check first via GetRibPositioningType()");
    }
    return *m_ribsPositioning_choice1;
}

CCPACSWingRibsPositioning& CCPACSWingRibsDefinition::GetRibsPositioning()
{
    // forward call to const method
    return const_cast<CCPACSWingRibsPositioning&>(static_cast<const CCPACSWingRibsDefinition&>(*this).GetRibsPositioning());
}

const CCPACSWingRibExplicitPositioning& CCPACSWingRibsDefinition::GetRibExplicitPositioning() const
{
    if (!m_ribExplicitPositioning_choice2) {
        LOG(ERROR) << "CCPACSWingRibsDefinition is not defined via ribExplicitPositioning!";
        throw CTiglError("CCPACSWingRibsDefinition is not defined via ribExplicitPositioning! Please check first via GetRibPositioningType()!");
    }
    return *m_ribExplicitPositioning_choice2;
}

CCPACSWingRibExplicitPositioning& CCPACSWingRibsDefinition::GetRibExplicitPositioning()
{
    // forward call to const method
    return const_cast<CCPACSWingRibExplicitPositioning&>(static_cast<const CCPACSWingRibsDefinition&>(*this).GetRibExplicitPositioning());
}

const CCPACSWingCSStructure& CCPACSWingRibsDefinition::GetStructure() const
{
    return structure;
}

CCPACSWingCSStructure& CCPACSWingRibsDefinition::GetStructure()
{
    return structure;
}

int CCPACSWingRibsDefinition::GetNumberOfRibs() const
{
    int numberOfRibs = 0;
    switch (GetRibPositioningType()) {
    case RIB_EXPLICIT_POSITIONING:
        numberOfRibs = 1;
        break;
    case RIBS_POSITIONING:
        if (!ribSetDataCache.valid) {
            UpdateRibSetDataCache();
        }
        assert(ribSetDataCache.valid);
        numberOfRibs = ribSetDataCache.numberOfRibs;
        break;
    default:
        throw CTiglError("Unknown GetRibPositioningType() found in CCPACSWingRibsDefinition::GetNumberOfRibs()!");
    }
    return numberOfRibs;
}

void CCPACSWingRibsDefinition::GetRibMidplanePoints(int ribNumber, gp_Pnt& startPoint, gp_Pnt& endPoint) const
{
    if (!auxGeomCache.valid) {
        BuildAuxiliaryGeometry();
    }
    assert(auxGeomCache.valid);

    int index = ribNumber - 1;
    if (index < 0 || index >= GetNumberOfRibs()) {
        throw CTiglError("Invalid rib number requested in ribs definition \"" + m_uID + "\"");
    }
    startPoint = auxGeomCache.midplanePoints[index].startPnt;
    endPoint = auxGeomCache.midplanePoints[index].endPnt;
}

TopoDS_Shape CCPACSWingRibsDefinition::GetRibsGeometry(TiglCoordinateSystem referenceCS) const
{
    if (!ribGeometryCache.valid) {
        BuildGeometry();
    }
    assert(ribGeometryCache.valid);

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return ribGeometryCache.shape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(structure, ribGeometryCache.shape);
        break;
    default:
        throw CTiglError("Invalid coordinateSystem passed to CCPACSWingRibsDefinition::GetRibsGeometry");
    }
}

const CCPACSWingRibsDefinition::CutGeometry& CCPACSWingRibsDefinition::GetRibCutGeometry(int ribNumber) const
{
    if (!auxGeomCache.valid) {
        BuildAuxiliaryGeometry();
    }
    assert(auxGeomCache.valid);

    int index = ribNumber - 1;
    if (index < 0 || index >= GetNumberOfRibs()) {
        LOG(ERROR) << "invalid rib number passed to CCPACSWingRibsDefinition::GetRibCutGeometry!";
        throw CTiglError("ERROR: invalid rib number passed to CCPACSWingRibsDefinition::GetRibCutGeometry!");
    }
    return auxGeomCache.cutGeometries[index];
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

    // build rib geometries
    if (!ribGeometryCache.valid) {
        BuildGeometry();
    }
    assert(ribGeometryCache.valid);

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return ribGeometryCache.ribFaces[index];
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return TopoDS::Face(ApplyWingTransformation(structure, ribGeometryCache.ribFaces[index]));
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingRibsDefinition::GetRibFace");
    }
}

TopoDS_Shape CCPACSWingRibsDefinition::GetSplittedRibsGeometry(TiglCoordinateSystem referenceCS) const
{
    if (!splittedRibGeomCache.valid) {
        BuildSplittedRibsGeometry();
    }
    assert(splittedRibGeomCache.valid);

    TopoDS_Shape splittedRibShape;
    splittedRibShape = splittedRibGeomCache.shape;

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return splittedRibShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(structure, splittedRibShape);
        break;
    default:
        throw CTiglError("Invalid coordinate system passed to CCPACSWingRibsDefinition::GetSplittedRibsGeometry");
    }
}

bool CCPACSWingRibsDefinition::HasCaps() const
{
    return (m_ribCrossSection.HasUpperCap() || m_ribCrossSection.HasLowerCap());
}

TopoDS_Shape CCPACSWingRibsDefinition::GetRibCapsGeometry(RibCapSide side, TiglCoordinateSystem referenceCS) const
{
    if (!HasCaps()) {
        LOG(ERROR) << "No rib caps are defined for ribs definition " << m_uID;
        throw CTiglError("Error in CCPACSWingRibsDefinition::GetRibCapsGeometry(): No rib caps are defined for ribs definition " + m_uID + "!");
    }

    if (!ribCapsCache.valid) {
        BuildRibCapsGeometry();
    }
    assert(ribCapsCache.valid);

    TopoDS_Shape capsShape;
    if (side == UPPER) {
        capsShape = ribCapsCache.upperCapsShape;
    }
    else { // side == LOWER
        capsShape = ribCapsCache.lowerCapsShape;
    }

    switch (referenceCS) {
    case WING_COORDINATE_SYSTEM:
        return capsShape;
        break;
    case GLOBAL_COORDINATE_SYSTEM:
        return ApplyWingTransformation(structure, capsShape);
        break;
    default:
        throw CTiglError("Unsupported Coordinate System passed to CCPACSWingRibsDefinition::GetRibCapsGeometry!");
    }
}

void CCPACSWingRibsDefinition::UpdateRibSetDataCache() const
{
    // ensure that this is only called when m_ribsPositioning_choice1 is used!!!
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    ribSetDataCache.valid = false;

    // compute eta/xsi values of ribs definition
    ribSetDataCache.referenceEtaStart = ComputeReferenceEtaStart();
    ribSetDataCache.referenceEtaEnd = ComputeReferenceEtaEnd();

    // compute the offset between the ribs
    ribSetDataCache.referenceEtaOffset = ComputeEtaOffset(ribSetDataCache.referenceEtaStart, ribSetDataCache.referenceEtaEnd);

    // compute number of ribs
    ribSetDataCache.numberOfRibs = ComputeNumberOfRibs(ribSetDataCache.referenceEtaStart, ribSetDataCache.referenceEtaEnd);

    ribSetDataCache.valid = true;
}

void CCPACSWingRibsDefinition::BuildAuxiliaryGeometry() const
{
    auxGeomCache.valid = false;
    auxGeomCache.clear();

    switch (GetRibPositioningType()) {
    case RIBS_POSITIONING:
        BuildAuxGeomRibsPositioning();
        break;
    case RIB_EXPLICIT_POSITIONING:
        BuildAuxGeomExplicitRibPositioning();
        break;
    default:
        LOG(ERROR) << "Invalid GetRibPositioningType() found in CCPACSWingRibsDefinition::BuildAuxiliaryGeometry!";
        throw CTiglError("Invalid GetRibPositioningType() found in CCPACSWingRibsDefinition::BuildAuxiliaryGeometry!");
    }

    auxGeomCache.valid = true;
}

void CCPACSWingRibsDefinition::BuildAuxGeomRibsPositioning() const
{
    if (!ribSetDataCache.valid) {
        UpdateRibSetDataCache();
    }
    assert(ribSetDataCache.valid);

    // STEP 1: iterate over all ribs for this rib definition
    for (int i = 0; i < ribSetDataCache.numberOfRibs; i++) {

        // STEP 2: compute the current eta value
        double currentEta = ribSetDataCache.referenceEtaStart + ribSetDataCache.referenceEtaOffset * i;

        // STEP 3: determine elementUID or sparPositionUID where rib should be placed
        std::string elementUID = "";
        if (i == 0 && m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_START) {
            elementUID = m_ribsPositioning_choice1->GetElementStartUID();
        }
        // NOTE: we have to check the eta difference here (instead of the index) to support spacing definitions
        else if (fabs(ribSetDataCache.referenceEtaEnd - currentEta) <= Precision::Confusion() && m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_END) {
            elementUID = m_ribsPositioning_choice1->GetElementEndUID();
        }
        std::string sparPositionUID = "";
        if (i == 0 && m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_START) {
            sparPositionUID = m_ribsPositioning_choice1->GetSparPositionStartUID();
        }
        // NOTE: we have to check the eta difference here (instead of the index) to support spacing definitions
        else if (fabs(ribSetDataCache.referenceEtaEnd - currentEta) <= Precision::Confusion() && m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_END) {
            sparPositionUID = m_ribsPositioning_choice1->GetSparPositionEndUID();
        }

        // STEP 4: build the rib cut geometry based on the current eta value
        //         and the element UID (if defined)
        CutGeometry cutGeom = BuildRibCutGeometry(currentEta, elementUID, sparPositionUID);
        auxGeomCache.cutGeometries.push_back(cutGeom);
    }
}

CCPACSWingRibsDefinition::CutGeometry CCPACSWingRibsDefinition::BuildRibCutGeometry(double currentEta, const std::string& elementUID, const std::string& sparPositionUID) const
{
    std::string ribStart = m_ribsPositioning_choice1->GetRibStart();
    std::string ribEnd = m_ribsPositioning_choice1->GetRibEnd();
    std::string ribReference = m_ribsPositioning_choice1->GetRibReference();

    // handle case when rib lies within a section element (elementUID defined)
    if (!elementUID.empty()) {
        TopoDS_Face ribFace = GetSectionRibGeometry(elementUID, currentEta, ribStart, ribEnd);
        // Compute rib start and end point for the cell definition
        RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribFace);
        auxGeomCache.midplanePoints.push_back(midplanePoints);
        return CutGeometry(ribFace, true);
    }

    // handle case when rib lies within inner or outer section element
    if (currentEta < Precision::Confusion() || currentEta > 1 - Precision::Confusion()) {
        if (ribReference == "leadingEdge" || ribReference == "trailingEdge" || IsOuterSparPointInSection(ribReference, currentEta, structure)) {
            TopoDS_Face ribFace = GetSectionRibGeometry("", currentEta, ribStart, ribEnd);
            // Compute rib start and end point for the cell definition
            RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribFace);
            auxGeomCache.midplanePoints.push_back(midplanePoints);
            return CutGeometry(ribFace, true);
        }
    }

    // otherwise rib cut face must be built
    // STEP 1: compute the reference point for the rib
    gp_Pnt referencePnt = GetReferencePoint(structure, ribReference, currentEta);

    // STEP 2: compute the up vector for the rib cut face without x rotation
    gp_Vec upVec = GetUpVectorWithoutXRotation(m_ribsPositioning_choice1->GetRibReference(), currentEta, referencePnt, sparPositionUID, structure);

    // STEP 3: compute direction vector of rib (points from start point to end point)
    gp_Vec ribDir = GetRibDirection(currentEta, referencePnt, upVec);

    // STEP 4: apply x rotation to up vector, negative to be identical to section rotation
    ApplyXRotationToUpVector(m_ribCrossSection.GetXRotation(), upVec, ribDir);

    // STEP 5: build the rib cut face
    TopoDS_Face ribCutFace = BuildRibCutFace(referencePnt, ribDir, ribStart, ribEnd, upVec, upVec);

    // STEP 6: compute the start and end point for the rib by intersection of the rib
    //         cut face with the according line (required for cell definition)
    RibMidplanePoints midplanePoints = ComputeRibDefinitionPoints(ribStart, ribEnd, ribCutFace);
    auxGeomCache.midplanePoints.push_back(midplanePoints);

    // finally return resulting rib cut face
    return CutGeometry(ribCutFace, false);
}

void CCPACSWingRibsDefinition::BuildAuxGeomExplicitRibPositioning() const
{
    gp_Pnt startPnt, endPnt;
    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();

    // get values from m_ribExplicitPositioning_choice2
    std::string startReference = m_ribExplicitPositioning_choice2->GetStartReference();
    std::string endReference = m_ribExplicitPositioning_choice2->GetEndReference();
    double startEta = m_ribExplicitPositioning_choice2->GetEtaStart();
    double endEta = m_ribExplicitPositioning_choice2->GetEtaEnd();

    // Step 1: get start point of rib on midplane
    startPnt = GetReferencePoint(structure, startReference, startEta);
    endPnt = GetReferencePoint(structure, endReference, endEta);

    // Step 2: Save the Rib start and end point (required for cells)
    auxGeomCache.midplanePoints.push_back(RibMidplanePoints(startPnt, endPnt));

    // Step 3: check whether rib lies within section
    //         (use section face as rib geometry)
    if ((startEta < Precision::Confusion() && endEta < Precision::Confusion()) ||
        (startEta > 1 - Precision::Confusion() && endEta > 1 - Precision::Confusion())) {
        if ((startReference == "leadingEdge" || startReference == "trailingEdge" || IsOuterSparPointInSection(startReference, startEta, structure)) &&
            (endReference == "leadingEdge" || endReference == "trailingEdge" || IsOuterSparPointInSection(endReference, endEta, structure))) {
            TopoDS_Face ribFace = GetSectionRibGeometry("", startEta, startReference, endReference);
            CutGeometry cutGeom(ribFace, true);
            auxGeomCache.cutGeometries.push_back(cutGeom);
            return;
        }
    }

    // Step 4: compute up vectors in start and end point
    gp_Vec upVecStart(0, 0, 0), upVecEnd(0, 0, 0);
    if ((startReference == "leadingEdge" || startReference == "trailingEdge") &&
        (endReference == "leadingEdge" || endReference == "trailingEdge")) {
        double midplaneEta, dummy;
        wingStructureReference.GetMidplaneEtaXsi(startPnt, midplaneEta, dummy);
        upVecStart = wingStructureReference.GetMidplaneNormal(midplaneEta);
        upVecEnd = upVecStart;
    }
    else {
        // check whether the startReference is a spar
        if (startReference != "leadingEdge" && startReference != "trailingEdge") {
            CCPACSWingSparSegment& spar = structure.GetSparSegment(startReference);
            TopoDS_Shape sparShape = spar.GetSparGeometry(WING_COORDINATE_SYSTEM);
            double midplaneEta, dummy;
            wingStructureReference.GetMidplaneEtaXsi(startPnt, midplaneEta, dummy);
            gp_Vec midplaneNormal = wingStructureReference.GetMidplaneNormal(midplaneEta);
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
            CCPACSWingSparSegment& spar = structure.GetSparSegment(endReference);
            TopoDS_Shape sparShape = spar.GetSparGeometry(WING_COORDINATE_SYSTEM);
            double midplaneEta, dummy;
            wingStructureReference.GetMidplaneEtaXsi(endPnt, midplaneEta, dummy);
            gp_Vec midplaneNormal = wingStructureReference.GetMidplaneNormal(midplaneEta);
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
            wingStructureReference.GetMidplaneEtaXsi(startPnt, midplaneEta, dummy);
            upVecStart = wingStructureReference.GetMidplaneNormal(midplaneEta);
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
    auxGeomCache.cutGeometries.push_back(cutGeom);
}

void CCPACSWingRibsDefinition::BuildGeometry() const
{
    ribGeometryCache.valid = false;
    ribGeometryCache.clear();

    // build target compound for ribs
    TopoDS_Compound compound;
    BRep_Builder compoundBuilder;
    compoundBuilder.MakeCompound(compound);

    TopoDS_Shape loft = structure.GetWingStructureReference().GetLoft()->Shape();

    // Step 3: iterate over all ribs for this rib definition
    for (int i = 0; i < GetNumberOfRibs(); i++) {

        const CutGeometry& cutGeometry = GetRibCutGeometry(i + 1);
        // handle case when ribCutFace is identical with target rib face
        if (cutGeometry.isTargetFace) {
            compoundBuilder.Add(compound, cutGeometry.shape);
            ribGeometryCache.ribFaces.push_back(cutGeometry.shape);
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
                    LOG(ERROR) << "unable to generate rib face for rib definition: " << GetUID();
                    throw CTiglError("Error: unable to generate rib face for rib definition \"" + GetUID() + "\"! Please check for a correct rib definition!");
                }
            }
            else if (wireList.Extent() == 2) {
                ribFace = BuildFace(TopoDS::Wire(wireList.First()), TopoDS::Wire(wireList.Last()));
            }
            else {
                LOG(ERROR) << "no geometry for ribs definition found!";
                throw CTiglError("Error: no geometry for ribs definition found!");
            }

            if (ribFace.IsNull()) {
                LOG(ERROR) << "unable to generate rib face for rib definition: " << GetUID();
                std::stringstream ss;
                ss << "Error: unable to generate rib face for rib definition: " << GetUID();
                throw CTiglError(ss.str());
            }

            // add rib face to compound
            compoundBuilder.Add(compound, ribFace);
            ribGeometryCache.ribFaces.push_back(ribFace);
        }
    }

    ribGeometryCache.shape = compound;
    ribGeometryCache.valid = true;
}

// Builds the ribs geometry splitted with the spars
void CCPACSWingRibsDefinition::BuildSplittedRibsGeometry() const
{
    splittedRibGeomCache.valid = false;

    // ensure that the rib geometry is built
    if (!ribGeometryCache.valid) {
        BuildGeometry();
    }
    assert(ribGeometryCache.valid);

    // split the ribs geometry with the spar split geometry
    splittedRibGeomCache.shape = CutShapeWithSpars(ribGeometryCache.shape, structure);

    splittedRibGeomCache.valid = true;
}

void CCPACSWingRibsDefinition::BuildRibCapsGeometry() const
{
    ribCapsCache.valid = false;
    ribCapsCache.upperCapsShape.Nullify();
    ribCapsCache.lowerCapsShape.Nullify();

    // build up single compound of all rib cut shapes
    BRep_Builder builder;
    TopoDS_Compound ribCuttingCompound;
    builder.MakeCompound(ribCuttingCompound);
    for (int k = 0; k < GetNumberOfRibs(); k++) {
        builder.Add(ribCuttingCompound, GetRibCutGeometry(k + 1).shape);
    }

    // build caps shape for upper cap
    if (m_ribCrossSection.HasUpperCap()) {
        TopoDS_Shape loft = structure.GetWingStructureReference().GetUpperShape();
        TopoDS_Shape cutResult = CutShapes(loft, ribCuttingCompound);
        // Get the cutting edge of the rib cutting plane and the loft
        TopoDS_Compound capEdges;
        builder.MakeCompound(capEdges);
        TopExp_Explorer explorer;
        for (explorer.Init(cutResult, TopAbs_EDGE); explorer.More(); explorer.Next()) {
            builder.Add(capEdges, TopoDS::Edge(explorer.Current()));
        }
        ribCapsCache.upperCapsShape = capEdges;
    }
    // build caps shape for lower cap
    if (m_ribCrossSection.HasLowerCap()) {
        TopoDS_Shape loft = structure.GetWingStructureReference().GetLowerShape();
        TopoDS_Shape cutResult = CutShapes(loft, ribCuttingCompound);
        // Get the cutting edge of the rib cutting plane and the loft
        TopoDS_Compound capEdges;
        builder.MakeCompound(capEdges);
        TopExp_Explorer explorer;
        for (explorer.Init(cutResult, TopAbs_EDGE); explorer.More(); explorer.Next()) {
            builder.Add(capEdges, TopoDS::Edge(explorer.Current()));
        }
        ribCapsCache.lowerCapsShape = capEdges;
    }

    ribCapsCache.valid = true;
}

TopoDS_Wire CCPACSWingRibsDefinition::GetReferenceLine() const
{
    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();
    TopoDS_Wire referenceLine;
    std::string ribReference = m_ribsPositioning_choice1->GetRibReference();
    if (ribReference == "leadingEdge") {
        referenceLine = wingStructureReference.GetLeadingEdgeLine();
    }
    else if (ribReference == "trailingEdge") {
        referenceLine = wingStructureReference.GetTrailingEdgeLine();
    }
    else {
        // find spar with m_uID
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(ribReference);
        referenceLine = sparSegment.GetSparMidplaneLine();
    }
    return referenceLine;
}

double CCPACSWingRibsDefinition::ComputeReferenceEtaStart() const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();
    if (m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::ETA_START) {
        return m_ribsPositioning_choice1->GetEtaStart();
    }
    else if (m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_START) {
        return ComputeSectionElementEta(m_ribsPositioning_choice1->GetElementStartUID());
    }
    else if (m_ribsPositioning_choice1->GetStartDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_START) {
        return ComputeSparPositionEta(m_ribsPositioning_choice1->GetSparPositionStartUID());
    }
    else {
        throw CTiglError("Unknown StartDefinitionType found for RibsPositioning in CCPACSWingRibsDefinition::GetEtaStart");
    }
}

double CCPACSWingRibsDefinition::ComputeReferenceEtaEnd() const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();
    if (m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::ETA_END) {
        return m_ribsPositioning_choice1->GetEtaEnd();
    }
    else if (m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::ELEMENT_END) {
        return ComputeSectionElementEta(m_ribsPositioning_choice1->GetElementEndUID());
    }
    else if (m_ribsPositioning_choice1->GetEndDefinitionType() == CCPACSWingRibsPositioning::SPARPOSITION_END) {
        return ComputeSparPositionEta(m_ribsPositioning_choice1->GetSparPositionEndUID());
    }
    else {
        throw CTiglError("Unknown EndDefinitionType found for RibsPositioning in CCPACSWingRibsDefinition::GetEtaEnd");
    }
}

double CCPACSWingRibsDefinition::ComputeSectionElementEta(const std::string& sectionElementUID) const
{
    // get componentSegment required for getting section element face
    CCPACSWingComponentSegment& componentSegment = structure.GetWingStructureReference().GetWingComponentSegment();

    // get the section element face
    TopoDS_Face sectionFace = componentSegment.GetSectionElementFace(sectionElementUID);

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
    CheckSparPositionOnReference(sparPositionUID, m_ribsPositioning_choice1->GetRibReference(), structure);

    // get componentSegment required for getting section element face
    CCPACSWingSpars& spars = structure.GetSpars();

    // obtain the spar position instance for the referenced UID
    CCPACSWingSparPosition& sparPos = spars.GetSparPositions().GetSparPosition(sparPositionUID);

    // compute midplane point according to spar definition
    gp_Pnt midplanePoint = GetSparMidplanePoint(sparPos, structure);

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
    if (m_ribsPositioning_choice1->GetRibCountDefinitionType() == CCPACSWingRibsPositioning::NUMBER_OF_RIBS) {
        numberOfRibs = m_ribsPositioning_choice1->GetNumberOfRibs();
    }
    else if (m_ribsPositioning_choice1->GetRibCountDefinitionType() == CCPACSWingRibsPositioning::SPACING) {
        // otherwise compute the number based on the spacing
        double spacing = m_ribsPositioning_choice1->GetSpacing();

        // handle the case when start and end eta are identical
        double deltaEta = fabs(etaEnd - etaStart);
        if (deltaEta <= Precision::Confusion()) {
            numberOfRibs = 1;
        }
        else {
            // get the length of the reference line between the two points
            double length = GetRibReferenceLength(m_ribsPositioning_choice1->GetRibReference(), structure) * fabs(etaEnd - etaStart);
            // finally compute the number of ribs by dividing the length by the spacing
            // NOTE: adding of Precision::Confusion in order to avoid floating point problems
            numberOfRibs = (int)((length + Precision::Confusion()) / spacing) + 1;
        }
    }
    else {
        LOG(ERROR) << "Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeNumberOfRibs!";
        throw CTiglError("Error: Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeNumberOfRibs!");
    }

    return numberOfRibs;
}

double CCPACSWingRibsDefinition::ComputeEtaOffset(double etaStart, double etaEnd) const
{
    assert(GetRibPositioningType() == RIBS_POSITIONING);

    double etaOffset;

    // check whether number of ribs is defined, or spacing is defined
    if (m_ribsPositioning_choice1->GetRibCountDefinitionType() == CCPACSWingRibsPositioning::NUMBER_OF_RIBS) {
        int numberOfRibs = m_ribsPositioning_choice1->GetNumberOfRibs();
        // in case only 1 rib is defined define eta offset as 0
        if (numberOfRibs == 1) {
            etaOffset = 0;
        }
        else {
            etaOffset = fabs(etaEnd - etaStart) / (numberOfRibs - 1);
        }
    }
    else if (m_ribsPositioning_choice1->GetRibCountDefinitionType() == CCPACSWingRibsPositioning::SPACING) {
        double spacing = m_ribsPositioning_choice1->GetSpacing();
        // get the length of the rib reference line between the two eta points
        double referenceLength = GetRibReferenceLength(m_ribsPositioning_choice1->GetRibReference(), structure);
        etaOffset = spacing / referenceLength;
    }
    else {
        LOG(ERROR) << "Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeEtaOffset!";
        throw CTiglError("Error: Unknown ribsCountDefinitionType found in CCPACSWingRibsDefinition::ComputeEtaOffset!");
    }

    return etaOffset;
}

TopoDS_Face CCPACSWingRibsDefinition::GetSectionRibGeometry(const std::string& elementUID, double eta, const std::string& ribStart, const std::string& ribEnd) const
{
    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();
    TopoDS_Face ribFace;
    if (!elementUID.empty()) {
        CCPACSWingComponentSegment& componentSegment = wingStructureReference.GetWingComponentSegment();
        ribFace = componentSegment.GetSectionElementFace(elementUID);
    }
    else {
        // NOTE: the check whether the eta value matches to the border of the 
        // component segment (in case it is a spar eta) was done before this
        // method was called!
        // TODO: find a better way, e.g. replace eta by enum
        if (eta < Precision::Confusion()) {
            ribFace = wingStructureReference.GetInnerFace();
        }
        else {
            ribFace = wingStructureReference.GetOuterFace();
        }
    }

    // cut rib face in case it starts at spar
    if (ribStart != "leadingEdge" && ribStart != "trailingEdge") {
        // find spar with m_uID
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(ribStart);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to trailing edge
        ribFace = GetNearestFace(cutResult, wingStructureReference.GetMidplaneOrChordlinePoint(eta, 1));
    }
    
    // cut rib face in case it ends at spar
    if (ribEnd != "leadingEdge" && ribEnd != "trailingEdge") {
        // find spar with m_uID
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(ribEnd);
        // split rib with spar cut shape
        TopoDS_Shape cutShape = sparSegment.GetSparCutGeometry(WING_COORDINATE_SYSTEM);
        TopoDS_Shape cutResult = SplitShape(ribFace, cutShape);
        // get face from split result which is nearest to leading edge
        ribFace = GetNearestFace(cutResult, wingStructureReference.GetMidplaneOrChordlinePoint(eta, 0));
    }

    return ribFace;
}

gp_Vec CCPACSWingRibsDefinition::GetRibDirection(double currentEta, const gp_Pnt& startPnt, const gp_Vec& upVec) const
{
    gp_Vec ribDir;
    const CTiglWingStructureReference& wingStructureReference = structure.GetWingStructureReference();

    std::string ribReference = m_ribsPositioning_choice1->GetRibReference();
    double zRotation = m_ribsPositioning_choice1->GetRibRotation().GetZ() * M_PI / 180.0;

    boost::optional<generated::CPACSRibRotation_ribRotationReference_SimpleContent> ribRotationReference;
    if(m_ribsPositioning_choice1->GetRibRotation().HasRibRotationReference())
        ribRotationReference = m_ribsPositioning_choice1->GetRibRotation().GetRibRotationReference().GetSimpleContent();

    if (!ribRotationReference) {
        double midplaneEta, dummy;
        wingStructureReference.GetMidplaneEtaXsi(startPnt, midplaneEta, dummy);
        ribDir = wingStructureReference.GetMidplaneEtaDir(midplaneEta);
    } else if (ribRotationReference == generated::CPACSRibRotation_ribRotationReference_SimpleContent::LeadingEdge) {
        ribDir = wingStructureReference.GetLeadingEdgeDirection(startPnt);
    }
    else if (ribRotationReference == generated::CPACSRibRotation_ribRotationReference_SimpleContent::TrailingEdge) {
        ribDir = wingStructureReference.GetTrailingEdgeDirection(startPnt);
    }
    else if (ribRotationReference == generated::CPACSRibRotation_ribRotationReference_SimpleContent::globalY) {
        // rotate y-axis around z-axis by zRotation in order to get rib direction
        ribDir = gp_Vec(0, 1, 0);
    }
    else {
        // rotation computation only implemented for ribs which have the
        // reference point in the same spar, otherwise computation could
        // be difficult
        const std::string ribRotationReferenceStr = generated::CPACSRibRotation_ribRotationReference_SimpleContentToString(*ribRotationReference);
        if (ribReference != ribRotationReferenceStr) {
            LOG(ERROR) << "using spar as rib rotation reference but not as rib reference is not supported!";
            throw CTiglError("ERROR: using spar as rib rotation reference but not as rib reference is not supported!");
        }
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(ribRotationReferenceStr);
        ribDir = sparSegment.GetDirection(currentEta);
    }

    // rotate rib direction by z rotation around up vector
    // special handling for globalY, by default the zRotation defines the rotation around the up-vector
    if (ribRotationReference == generated::CPACSRibRotation_ribRotationReference_SimpleContent::globalY) {
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
    double bboxSize = GetBoundingBoxSize(structure.GetWingStructureReference().GetLoft()->Shape());

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
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(sparUid);
        TopoDS_Shape sparGeometry = sparSegment.GetSparGeometry(WING_COORDINATE_SYSTEM);

        try {
            // cut the rib face with the spar, returning the front wire of the resulting rib
            CutFaceWithSpar(ribCutFace, sparGeometry, bboxSize, cutFaceFrontWire, cutFaceBackWire, upVecStart.Multiplied(-1));
        }
        catch (const CTiglError&) {
            throw CTiglError("Geometric intersection of Rib \"" + GetUID() + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
        }
    }
    if (ribEnd != "leadingEdge" && ribEnd != "trailingEdge") {
        // get geometry of spar
        std::string sparUid = ribEnd;
        // find spar with m_uID
        CCPACSWingSparSegment& sparSegment = structure.GetSparSegment(sparUid);
        TopoDS_Shape sparGeometry = sparSegment.GetSparGeometry(WING_COORDINATE_SYSTEM);

        try {
            // cut the rib face with the spar, returning the back wire of the resulting rib
            CutFaceWithSpar(ribCutFace, sparGeometry, bboxSize, cutFaceBackWire, cutFaceFrontWire, upVecEnd);
        }
        catch (const CTiglError&) {
            throw CTiglError("Geometric intersection of Rib \"" + GetUID() + "\" with Spar \"" + sparUid + "\" failed! Please check for correct definition!");
        }
    }

    return ribCutFace;
}

CCPACSWingRibsDefinition::RibMidplanePoints CCPACSWingRibsDefinition::ComputeRibDefinitionPoints(const std::string& ribStart, 
                                                                                                 const std::string& ribEnd, 
                                                                                                 const TopoDS_Face& ribCutFace) const
{
    // Compute the intersection points of the rib definition lines and the cut face
    gp_Pnt ribStartPnt = GetRibDefinitionPoint(ribStart, ribCutFace, structure);
    gp_Pnt ribEndPnt = GetRibDefinitionPoint(ribEnd, ribCutFace, structure);
    return RibMidplanePoints(ribStartPnt, ribEndPnt);
}

} // end namespace tigl
