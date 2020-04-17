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

#include "CCPACSWingSparPosition.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"
#include "CPACSWingRibPoint.h"
#include "CPACSEtaXsiPoint.h"
#include "CTiglUIDManager.h"
#include "CCPACSWingRibsDefinition.h"
#include "tiglcommonfunctions.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>


namespace tigl
{

CCPACSWingSparPosition::CCPACSWingSparPosition(CCPACSWingSparPositions* sparPositions, CTiglUIDManager* uidMgr)
: generated::CPACSSparPosition(sparPositions, uidMgr) {}

bool CCPACSWingSparPosition::isOnInnerSectionElement() const
{
    if (GetSparPositionEtaXsi_choice2()) {
        return GetEta() < 1.E-6;
    }
    else {
        return false;
    }
}

bool CCPACSWingSparPosition::isOnOuterSectionElement() const
{
    if (GetSparPositionEtaXsi_choice2()) {
        return (1 - GetEta()) < 1.E-6;
    }
    else {
        return false;
    }
}

bool CCPACSWingSparPosition::isOnSectionElement() const
{
    return isOnInnerSectionElement() || isOnOuterSectionElement();
}

bool CCPACSWingSparPosition::isOnRib() const
{
    return m_sparPositionRib_choice1 ? true : false;
}

const std::string& CCPACSWingSparPosition::GetReferenceUID() const
{
    if (m_sparPositionRib_choice1) {
        return m_sparPositionRib_choice1->GetRibDefinitionUID();
    }
    else if (m_sparPositionEtaXsi_choice2){
        return m_sparPositionEtaXsi_choice2->GetReferenceUID();
    }
    else {
        throw CTiglError("Invalid choice type");
    }
}


double CCPACSWingSparPosition::GetEta() const
{
    if (!m_sparPositionEtaXsi_choice2) {
        throw CTiglError("SparPosition is not defined via eta/xsi. Please check InputType first before calling CCPACSWingSparPosition::GetXsi()");
    }
    return m_sparPositionEtaXsi_choice2->GetEta();
}

double CCPACSWingSparPosition::GetXsi() const
{
    if (m_sparPositionRib_choice1) {
        return m_sparPositionRib_choice1->GetXsi();
    }
    else if (m_sparPositionEtaXsi_choice2) {
        return m_sparPositionEtaXsi_choice2->GetXsi();
    }
    throw CTiglError("Invalid spar position type");
}

const CCPACSEtaXsiPoint &CCPACSWingSparPosition::GetEtaXsiPoint() const
{
    if (!GetSparPositionEtaXsi_choice2()) {
        throw CTiglError("No EtaXsiPoint definied in SparPosition '" + GetUID() + "'");
    }
    
    return GetSparPositionEtaXsi_choice2().value();
}

const generated::CPACSWingRibPoint& CCPACSWingSparPosition::GetRibPoint() const
{
    if (!GetSparPositionRib_choice1()) {
        throw CTiglError("No RibPoint definied in SparPosition '" + GetUID() + "'");
    }

    return GetSparPositionRib_choice1().value();
}

void CCPACSWingSparPosition::SetRibPoint(const CCPACSWingRibPoint& ribPoint)
{
    CCPACSWingRibPoint& rp = GetSparPositionRib_choice1(CreateIfNotExists);
    rp.SetRibDefinitionUID(ribPoint.GetRibDefinitionUID());
    rp.SetRibNumber(ribPoint.GetRibNumber());
    rp.SetXsi(ribPoint.GetXsi());

    RemoveSparPositionEtaXsi_choice2();
    Invalidate();
}

void CCPACSWingSparPosition::SetEtaXsiPoint(const CCPACSEtaXsiPoint& etaXsiPoint)
{
    CCPACSEtaXsiPoint& ep = GetSparPositionEtaXsi_choice2(CreateIfNotExists);
    ep.SetEta(etaXsiPoint.GetEta());
    ep.SetReferenceUID(etaXsiPoint.GetReferenceUID());
    ep.SetXsi(etaXsiPoint.GetXsi());

    RemoveSparPositionRib_choice1();
    Invalidate();
}

int WingRibPointGetRibNumber(const generated::CPACSWingRibPoint& ribPoint)
{
    return ribPoint.GetRibNumber().value_or(1);
}

std::string WingSparPosGetElementUID(const CCPACSWingSparPosition & pos)
{
    if (!pos.isOnSectionElement()) {
        throw CTiglError("Spar positioning is not on section element");
    }

    const CTiglUIDManager::TypedPtr tp = pos.GetUIDManager().ResolveObject(pos.GetEtaXsiPoint().GetReferenceUID());
    if (tp.type == &typeid(CCPACSWingComponentSegment)) {
        const CCPACSWingComponentSegment& cs = *reinterpret_cast<CCPACSWingComponentSegment*>(tp.ptr);
        const SegmentList& segments = cs.GetSegmentList();
        if (pos.isOnInnerSectionElement()) {
            return segments.front()->GetInnerSectionElementUID();
        }
        else {
            return segments.back()->GetOuterSectionElementUID();
        }
    }
    else if (tp.type == &typeid(CCPACSWingSegment)) {
        const CCPACSWingSegment& segment = *reinterpret_cast<CCPACSWingSegment*>(tp.ptr);
        if (pos.isOnInnerSectionElement()) {
            return segment.GetInnerSectionElementUID();
        }
        else {
            return segment.GetOuterSectionElementUID();
        }
    }
    else {
        throw CTiglError("'" + pos.GetEtaXsiPoint().GetReferenceUID() + "' in not a wing segment or a component segment.");
    }
}

gp_Vec CCPACSWingSparPosition::GetUpVector(const CCPACSWingCSStructure& structure, gp_Pnt midplanePnt) const
{
    const CTiglWingStructureReference wsr(structure);

    if ((isOnSectionElement() || isOnRib()) && wsr.GetType() == CTiglWingStructureReference::ComponentSegmentType) {
        // get componentSegment required for getting chordline points of sections
        const CCPACSWingComponentSegment& componentSegment = wsr.GetWingComponentSegment();

        TopoDS_Shape sectionFace;
        if (isOnSectionElement()) {
            const std::string sectionElemUID = WingSparPosGetElementUID(*this);
            sectionFace = componentSegment.GetSectionElementFace(sectionElemUID);
        }
        else if(isOnRib()) {
            const CCPACSWingRibsDefinition& ribs = structure.GetRibsDefinition(GetRibPoint().GetRibDefinitionUID());
            int ribNumber = WingRibPointGetRibNumber(*GetSparPositionRib_choice1());
            sectionFace = ribs.GetRibFace(ribNumber, WING_COORDINATE_SYSTEM);
        }
        else {
            // this should actually not happen
            throw CTiglError("A fatal error as occured");
        }
        
        // compute bounding box of section element face
        Bnd_Box bbox;
        BRepBndLib::Add(sectionFace, bbox);
        double sectionFaceSize = sqrt(bbox.SquareExtent());

        // generate a cut face aligned in the YZ plane
        const gp_Pnt p1 = midplanePnt.Translated(gp_Vec(0, -sectionFaceSize, -sectionFaceSize));
        const gp_Pnt p2 = midplanePnt.Translated(gp_Vec(0, -sectionFaceSize, sectionFaceSize));
        const gp_Pnt p3 = midplanePnt.Translated(gp_Vec(0, sectionFaceSize, -sectionFaceSize));
        const gp_Pnt p4 = midplanePnt.Translated(gp_Vec(0, sectionFaceSize, sectionFaceSize));

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
        const gp_Pnt pCut1               = BRep_Tool::Pnt(vCut1);
        const gp_Pnt pCut2               = BRep_Tool::Pnt(vCut2);

        // build the up vector based on the end points, and ensure correct orientation
        gp_Vec upVec = gp_Vec(pCut1, pCut2).Normalized();
        if (upVec.Dot(gp_Vec(0, 0, 1)) < 0) {
            upVec.Reverse();
        }

        return upVec;
    }
    
    // BUG #149 and #152
    // because of issues with the spar up vectors in adjacent component
    // segments the up vector is set to the z direction
    gp_Vec upVec = gp_Vec(0, 0, 1);
    /*
    // determine up-vector based on midplane line of inner spar point
    double eta = GetEta();
    gp_Pnt pl = cs.GetMidplanePoint(eta, 0);
    gp_Pnt pt = cs.GetMidplanePoint(eta, 1);
    gp_Vec chordLine(pl, pt);
    // determine default segment, in case of inner/outer eta value
    // (required for extended eta line)
    std::string defaultSegmentUID;
    if (eta < 0.5) {
        defaultSegmentUID = cs.GetInnerSegmentUID();
    } else {
        defaultSegmentUID = cs.GetOuterSegmentUID();
    }
    gp_Vec leDir = cs.GetLeadingEdgeDirection(pl, defaultSegmentUID);
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

} // end namespace tigl
