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

#include "tigletaxsifunctions.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <Geom_Line.hxx>
#include <Extrema_POnCurv.hxx>


#include "CTiglWingChordface.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSEtaIsoLine.h"
#include "CCPACSXsiIsoLine.h"
#include "CTiglError.h"
#include "CTiglWingStructureReference.h"
#include "tiglcommonfunctions.h"
#include "Debugging.h"


namespace tigl
{

gp_Pnt getSectionElementChordlinePoint(const CCPACSWingComponentSegment& cs, const std::string& sectionElementUID, double xsi)
{
    const CCPACSWing& wing = cs.GetWing();
    for (int i = 1; i <= wing.GetSegmentCount(); ++i) {
        const CCPACSWingSegment& segment = wing.GetSegment(i);

        double eta = -1;
        if (sectionElementUID == segment.GetInnerSectionElementUID()) {
            eta = 0;
        }
        if (sectionElementUID == segment.GetOuterSectionElementUID()) {
            eta = 1;
        }

        if (eta != -1) {
            // convert into wing coordinate system
            return wing.GetTransformationMatrix().Inverted().Transform(segment.GetChordPoint(eta, xsi));
        }
    }

    throw CTiglError("Error in getSectionElementChordlinePoint: section element not found!");
}

TopoDS_Face buildEtaCutFace(const CTiglWingStructureReference& wsr, double eta)
{
    // NOTE: we have to determine the cut plane by using the 
    // GetPoint methods for handling the case when the eta line
    // is extended because of an z-rotation of the outer or inner sections
    gp_Pnt startPnt = wsr.GetPoint(eta, 0., WING_COORDINATE_SYSTEM);
    gp_Pnt endPnt = wsr.GetPoint(eta, 1., WING_COORDINATE_SYSTEM);
    gp_Vec midplaneNormal = wsr.GetMidplaneNormal(eta);
    gp_Dir cutPlaneNormal = midplaneNormal.Crossed(gp_Vec(startPnt, endPnt));
    gp_Pln etaCutPlane = gp_Pln(startPnt, cutPlaneNormal);
    return BRepBuilderAPI_MakeFace(etaCutPlane);
}

double computeSparXsiValue(const CTiglWingStructureReference& wsr, const CCPACSWingSparSegment& spar, double eta)
{
    TopoDS_Wire sparMidplaneLine = spar.GetSparMidplaneLine();
    TopoDS_Face etaCutFace = buildEtaCutFace(wsr, eta);
    gp_Pnt etaXsiPoint;
    if (!GetIntersectionPoint(etaCutFace, sparMidplaneLine, etaXsiPoint)) {
        throw CTiglError("Error in computation of spar eta point in tigletaxsifunctions::computeSparXsiValue");
    }
    double newEta, xsi;
    wsr.GetEtaXsiLocal(etaXsiPoint, newEta, xsi);
    return xsi;
}

double computeRibEtaValue(const CTiglWingStructureReference& wsr, const CCPACSWingRibsDefinition& rib, int ribIndex, double xsi)
{
    // determine rib start and ent point
    gp_Pnt ribStartPoint, ribEndPoint;
    rib.GetRibMidplanePoints(ribIndex, ribStartPoint, ribEndPoint);

    double startEta, startXsi, endEta, endXsi;
    wsr.GetEtaXsiLocal(ribStartPoint, startEta, startXsi);
    wsr.GetEtaXsiLocal(ribEndPoint, endEta, endXsi);

    // fix numeric inaccuracy in xsi
    double precision = 1.E-8;
    if (xsi < startXsi && xsi >= (startXsi - precision)) {
        xsi = startXsi;
    }
    else if (xsi > endXsi && xsi <= (endXsi + precision)) {
        xsi = endXsi;
    }

    if (xsi < startXsi || xsi > endXsi) {
        throw CTiglError("Error in computeRibEtaValue: rib is not defined at passed xsi coordinate!");
    }

    // NOTE: may not be fully correct when rib lies within more than 1 segment!
    return startEta + (endEta - startEta) * ((xsi - startXsi) / (endXsi - startXsi));
}

EtaXsi computeRibSparIntersectionEtaXsi(const CTiglWingStructureReference& wsr, const CCPACSWingRibsDefinition& rib, int ribIndex, const CCPACSWingSparSegment& spar)
{
    TopoDS_Face ribShape = rib.GetRibFace(ribIndex, WING_COORDINATE_SYSTEM);

    DEBUG_SCOPE(a);
    TopoDS_Wire w = spar.GetSparMidplaneLine();
    a.addShape(w, "SparMidLine");
    a.addShape(ribShape, "RibFace");

    const double precision = 1E-6;

    gp_Pnt pIntersect;
    if (!GetIntersectionPoint(ribShape, w, pIntersect, precision)) {
        throw CTiglError("Error in computeRibSparIntersectionEtaXsi: intersection of rib and spar not found!");
    }

    // Get Eta Xsi coordinates of the point
    EtaXsi result;
    wsr.GetEtaXsiLocal(pIntersect, result.eta, result.xsi);

    return result;
}

double transformEtaToCSOrTed(const CCPACSEtaIsoLine& eta, const CTiglUIDManager& uidMgr)
{
    return transformEtaToCSOrTed(eta.GetEta(), eta.GetReferenceUID(), uidMgr);
}

double transformXsiToCSOrTed(const CCPACSXsiIsoLine& xsi, const CTiglUIDManager& uidMgr)
{
    // xsi does not depend on whether it is defined in a segment or a CS
    return xsi.GetXsi();
    //return transformXsiToCSOrTed(xsi.GetXsi(), xsi.GetReferenceUID(), uidMgr);
}
double transformEtaToCSOrTed(double eta, const std::string& referenceUid, const CTiglUIDManager& uidMgr)
{
    return transformEtaXsiToCSOrTed({eta, 0}, referenceUid, uidMgr).eta;
}

double transformXsiToCSOrTed(double xsi, const std::string& referenceUid, const CTiglUIDManager& uidMgr)
{
    // xsi does not depend on whether it is defined in a segment or a CS
    return xsi;
    //return transformEtaXsiToCSOrTed({0, xsi}, referenceUid, uidMgr).xsi;
}

EtaXsi transformEtaXsiToCSOrTed(EtaXsi etaXsi, const std::string& referenceUid, const CTiglUIDManager& uidMgr)
{
    const CTiglUIDManager::TypedPtr tp = uidMgr.ResolveObject(referenceUid);
    if (tp.type == &typeid(CCPACSWingSegment)) {
        const CCPACSWingSegment& segment = *reinterpret_cast<CCPACSWingSegment*>(tp.ptr);
        const boost::optional<CCPACSWingComponentSegments>& css = segment.GetParent()->GetParent<CCPACSWing>()->GetComponentSegments();
        if (css) {
            for (const auto& cs : css->GetComponentSegments()) {
                if (cs->IsSegmentContained(segment)) {
                    EtaXsi r;
                    cs->GetEtaXsiFromSegmentEtaXsi(referenceUid, etaXsi.eta, etaXsi.xsi, r.eta, r.xsi);
                    return r;
                }
            }
        }

        throw CTiglError("Wing of segment referenced by UID " + referenceUid + " has no component segments");
    }
    if (tp.type == &typeid(CCPACSWingComponentSegment))
        return etaXsi;
    //if (tp.type == &typeid(CCPACSTrailingEdgeDevice))
    //    return etaXsi;

    throw CTiglError("Unsupported type for iso line transformation referenced by UID " + referenceUid);
}


const CCPACSWingComponentSegment& _getBelongingComponentSegment(const std::string referenceUid, const CTiglUIDManager& uidMgr)
{
    const CTiglUIDManager::TypedPtr tp = uidMgr.ResolveObject(referenceUid);
    if (tp.type == &typeid(CCPACSWingSegment)) {
        const auto& segment = *reinterpret_cast<CCPACSWingSegment*>(tp.ptr);
        const boost::optional<CCPACSWingComponentSegments>& css = segment.GetParent()->GetParent<CCPACSWing>()->GetComponentSegments();
        if (css) {
            for (const auto& cs : css->GetComponentSegments()) {
                if (cs->IsSegmentContained(segment)) {
                    return *cs;
                }
            }
        }

        throw CTiglError("No component segment associated to wing segment with UID " + referenceUid);
    }
    if (tp.type == &typeid(CCPACSWingComponentSegment)) {
        const auto& compSeg = *reinterpret_cast<CCPACSWingComponentSegment*>(tp.ptr);
        return compSeg;
    }

    throw CTiglError("No component segment associated to component with UID " + referenceUid);
}

void InterpolateXsi(const std::string& refUID1, const EtaXsi& etaXsi1,
                    const std::string& refUID2, const EtaXsi& etaXsi2,
                    const std::string& targetUID, double eta,
                    const CTiglUIDManager& uidMgr,
                    double &xsi, double& errorDistance)
{
    if (eta > 1 + Precision::Confusion() || eta < - Precision::Confusion()) {
        throw CTiglError("Eta not in range [0,1] in InterpolateXsi.", TIGL_MATH_ERROR);
    }

    // declare points to compute the (component) segment line
    gp_Pnt p1;
    gp_Pnt p2;

    // declare points to compute the iso eta line of segment
    gp_Pnt pLE;
    gp_Pnt pTE;

    std::string wingUID1;
    std::string wingUID2;
    std::string wingUIDTarget;

    // compute the start point of the (component) segment line
    if (uidMgr.IsType<CCPACSWingSegment>(refUID1)) {
        const auto& segment1 = uidMgr.ResolveObject<CCPACSWingSegment>(refUID1);
        p1 = segment1.GetChordPoint(etaXsi1.eta, etaXsi1.xsi);
        wingUID1 = segment1.GetParent()->GetParent<CCPACSWing>()->GetUID();
    }
    else if (uidMgr.IsType<CCPACSWingComponentSegment>(refUID1)) {
        const auto& compSeg1 = uidMgr.ResolveObject<CCPACSWingComponentSegment>(refUID1);
        p1 = compSeg1.GetPoint(etaXsi1.eta, etaXsi1.xsi);
        wingUID1 = compSeg1.GetParent()->GetParent()->GetUID();
    }

    // compute the end point of the (component) segment line
    if (uidMgr.IsType<CCPACSWingSegment>(refUID2)) {
       const auto& segment2 = uidMgr.ResolveObject<CCPACSWingSegment>(refUID2);
        p2 = segment2.GetChordPoint(etaXsi2.eta, etaXsi2.xsi);
        wingUID2 = segment2.GetParent()->GetParent<CCPACSWing>()->GetUID();
    }
    else if (uidMgr.IsType<CCPACSWingComponentSegment>(refUID2)) {
        const auto& compSeg2 = uidMgr.ResolveObject<CCPACSWingComponentSegment>(refUID2);
        p2 = compSeg2.GetPoint(etaXsi2.eta, etaXsi2.xsi);
        wingUID2 = compSeg2.GetParent()->GetParent()->GetUID();
    }

    // compute the start and end point of the iso eta line
     if (uidMgr.IsType<CCPACSWingSegment>(targetUID)) {
        const auto& segmentTarget = uidMgr.ResolveObject<CCPACSWingSegment>(targetUID);
        pLE = segmentTarget.GetChordPoint(eta, 0.);
        pTE = segmentTarget.GetChordPoint(eta, 1.);
        wingUIDTarget = segmentTarget.GetParent()->GetParent<CCPACSWing>()->GetUID();
    }
    else if (uidMgr.IsType<CCPACSWingComponentSegment>(targetUID)) {
        const auto& compSegTarget = uidMgr.ResolveObject<CCPACSWingComponentSegment>(targetUID);
        pLE = compSegTarget.GetPoint(eta, 0.);
        pTE = compSegTarget.GetPoint(eta, 1.);
        wingUIDTarget = compSegTarget.GetParent()->GetParent()->GetUID();
    }

    // throw an error if the UIDs do not refer to one and the same wing
    if (wingUID1 != wingUID2 || wingUID2 != wingUIDTarget) {
        throw CTiglError("The referenced segments or component segments don't lie on the same wing.", TIGL_UID_ERROR);
    }

   // compute (component) segment line
   gp_Lin Line(p1, p2.XYZ() - p1.XYZ());

   // compute iso eta line of segment
   double chordDepth = pTE.Distance(pLE);
   gp_Lin etaLine(pLE, pTE.XYZ() - pLE.XYZ());

   // check, if both lines are parallel
   if (etaLine.Direction().IsParallel(Line.Direction(), M_PI/180.)) {
       throw CTiglError("(Component) segment line does not intersect iso eta line of segment.", TIGL_MATH_ERROR);
   }

   Handle(Geom_Curve) Curve = new Geom_Line(Line);
   Handle(Geom_Curve) etaCurve = new Geom_Line(etaLine);
   GeomAdaptor_Curve AdptAcuve(Curve);
   GeomAdaptor_Curve etaAdptCurve(etaCurve);

   // find point on etaLine, that minimizes distance to Line
   Extrema_ExtCC minimizer(AdptAcuve, etaAdptCurve);
   minimizer.Perform();

   // there should be exactly on minimum between two lines
   // if they are not parallel
   if (!minimizer.IsDone() || minimizer.NbExt() != 1) {
       throw CTiglError("(Component) segment line does not intersect iso eta line of segment.", TIGL_MATH_ERROR);
   }

   Extrema_POnCurv pOnLine, pOnEtaLine;
   minimizer.Points(1, pOnLine, pOnEtaLine);

   // If parameter on Line is < 0 or larger than
   // Length of Line, there is not actual intersection,
   // i.e. the Line is chosen too small
   // We use a tolerance here, to account for small user errors
   double tol = 1e-5;
   if (pOnLine.Parameter() < -tol || pOnLine.Parameter() > p1.Distance(p2) + tol) {
       throw CTiglError("(Component) segment line does not intersect iso eta line of segment.", TIGL_MATH_ERROR);
   }

   xsi = pOnEtaLine.Parameter()/chordDepth;

   // compute the error distance
   // This is the distance from the line to the nearest point on the chord face
   errorDistance = pOnLine.Value().Distance(pOnEtaLine.Value());

}

} // namespace tigl
