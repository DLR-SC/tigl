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

#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingSparSegment.h"
#include "CTiglError.h"
#include "CTiglWingStructureReference.h"
#include "tiglcommonfunctions.h"
#include "Debugging.h"


namespace tigl
{

gp_Pnt getSectionElementChordlinePoint(const CCPACSWingComponentSegment& cs, const std::string& sectionElementUID, double xsi)
{
    gp_Pnt chordlinePoint;
    CCPACSWing& wing = cs.GetWing();
    // find section element
    const CCPACSWingSegment& segment = static_cast<const CCPACSWingSegment&>(wing.GetSegment(1));
    if (sectionElementUID == segment.GetInnerSectionElementUID()) {
        // convert into wing coordinate system
        CTiglTransformation wingTrans = wing.GetTransformationMatrix();
        chordlinePoint = wingTrans.Inverted().Transform(segment.GetChordPoint(0, xsi));
    }
    else {
        int i;
        for (i = 1; i <= wing.GetSegmentCount(); ++i) {
            const CCPACSWingSegment& segment = static_cast<const CCPACSWingSegment&>(wing.GetSegment(i));
            if (sectionElementUID == segment.GetOuterSectionElementUID()) {
                // convert into wing coordinate system
                CTiglTransformation wingTrans = wing.GetTransformationMatrix();
                chordlinePoint = wingTrans.Inverted().Transform(segment.GetChordPoint(1, xsi));
                break;
            }
        }
        if (i > wing.GetSegmentCount()) {
            throw CTiglError("Error in getSectionElementChordlinePoint: section element not found!");
        }
    }
    return chordlinePoint;
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

}
