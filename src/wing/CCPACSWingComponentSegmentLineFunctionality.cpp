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

#include <assert.h>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepBndLib.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GC_MakeSegment.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <Bnd_Box.hxx>
#include <gp_Pln.hxx>

#include "CCPACSWingSegment.h"
#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingComponentSegmentLineFunctionality.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CCPACSWingComponentSegmentLineFunctionality::CCPACSWingComponentSegmentLineFunctionality(const CCPACSWingComponentSegment& compSegment)
    : compSegment(compSegment) {}

void CCPACSWingComponentSegmentLineFunctionality::Invalidate()
{
    linesAreValid = false;
}

void CCPACSWingComponentSegmentLineFunctionality::CheckLines()
{
    if (!linesAreValid) {
        BuildLines();
        linesAreValid = true;
    }
}

// Method for building wires for eta-, leading edge-, trailing edge-lines
void CCPACSWingComponentSegmentLineFunctionality::BuildLines()
{
    // search for ETA coordinate
    std::vector<gp_Pnt> lePointContainer;
    std::vector<gp_Pnt> tePointContainer;
    gp_Pnt extendedInnerLePoint;
    gp_Pnt extendedOuterLePoint;
    gp_Pnt extendedInnerTePoint;
    gp_Pnt extendedOuterTePoint;

    gp_Pnt pnt;
    int numberOfSections = 0;

    // get the leading and trailing edge points of all sections
    bool inComponentSection = false;
    for (int i = 1; i <= compSegment.GetWing().GetSegmentCount(); i++) {
        tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) compSegment.GetWing().GetSegment(i);

        // Ok, we found the first segment of this componentSegment
        if (!inComponentSection && segment.GetInnerSectionElementUID() == compSegment.GetFromElementUID()) {
            inComponentSection = true;
        }

        if (inComponentSection) {
            // store number of sections (for number of points per line)
            numberOfSections++;

            // get leading edge point
            pnt = segment.GetPoint(0, 0, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
            lePointContainer.push_back(pnt);
            // get trailing edge point
            pnt = segment.GetPoint(0, 1, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
            tePointContainer.push_back(pnt);

            // if we found the outer section, break...
            if (segment.GetOuterSectionElementUID() == compSegment.GetToElementUID()) {
                numberOfSections++;
                // get leading edge point
                pnt = segment.GetPoint(1, 0, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
                lePointContainer.push_back(pnt);
                // get trailing edge point
                pnt = segment.GetPoint(1, 1, true, CCPACSWingSegment::WING_COORDINATE_SYSTEM);
                tePointContainer.push_back(pnt);
                break;
            }
        }
    }

    // determine extended leading/trailing edge points
    // scale leading or trailing edge to get both points in the section planes of the
    // inner and outer sections
    // see CPACS documentation for "componentSegment" element
    extendedInnerLePoint = lePointContainer.at(0);
    extendedInnerTePoint = tePointContainer.at(0);
    extendedOuterLePoint = lePointContainer.at(lePointContainer.size() - 1);
    extendedOuterTePoint = tePointContainer.at(tePointContainer.size() - 1);
    std::string innerSegmentUID = compSegment.GetInnerSegmentUID();
    std::string outerSegmentUID = compSegment.GetOuterSegmentUID();
    gp_Vec innerLeDirYZ = compSegment.GetLeadingEdgeDirectionYZ(innerSegmentUID);
    gp_Vec outerLeDirYZ = compSegment.GetLeadingEdgeDirectionYZ(outerSegmentUID);
    gp_Vec innerChordVec(extendedInnerTePoint, extendedInnerLePoint);

    // compute length of chord line vector projected to eta line vector
    double lp = innerChordVec.Dot(innerLeDirYZ);
    // check if projection of chord line vector points in direction or in opposite direction of eta line vector
    if (lp > Precision::Confusion()) {
        // scale leading edge
        gp_Vec innerLeDir = compSegment.GetLeadingEdgeDirection(innerSegmentUID);
        // compute cosine of angle between leading edge vector and eta line vector
        double cosPhi = innerLeDir.Dot(innerLeDirYZ);
        // compute the length value for extending the leading edge
        double length = lp / cosPhi;
        extendedInnerLePoint.Translate(-1.0 * innerLeDir * length);
    } else if (lp < -Precision::Confusion()) {
        // scale trailing edge
        gp_Vec innerTeDir = compSegment.GetTrailingEdgeDirection(innerSegmentUID);
        // compute cosine of angle between trailing edge vector and eta line vector
        double cosPhi = innerTeDir.Dot(innerLeDirYZ);
        // compute the length value for extending the trailing edge
        double length = -1.0 * (lp / cosPhi);
        extendedInnerTePoint.Translate(-1.0 * innerTeDir * length);
    }
    gp_Vec outerChordVec(extendedOuterTePoint, extendedOuterLePoint);
    // compute length of chord line vector projected to eta line vector
    lp = outerChordVec.Dot(outerLeDirYZ);
    // check if projection of chord line vector points in direction or in opposite direction of eta line vector
    if (lp > Precision::Confusion()) {
        // scale trailing edge
        gp_Vec outerTeDir = compSegment.GetTrailingEdgeDirection(outerSegmentUID);
        // compute cosine of angle between trailing edge vector and eta line vector
        double cosPhi = outerTeDir.Dot(outerLeDirYZ);
        // compute the length value for extending the trailing edge
        double length = lp / cosPhi;
        extendedOuterTePoint.Translate(outerTeDir * length);
    } else if (lp < -Precision::Confusion()) {
        // scale leading edge
        gp_Vec outerLeDir = compSegment.GetLeadingEdgeDirection(outerSegmentUID);
        // compute cosine of angle between leading edge vector and eta line vector
        double cosPhi = outerLeDir.Dot(outerLeDirYZ);
        // compute the length value for extending the leading edge
        double length = -1.0 * (lp / cosPhi);
        extendedOuterLePoint.Translate(outerLeDir * length);
    }

#ifdef _DEBUG
    innerChordVec = gp_Vec(extendedInnerTePoint, extendedInnerLePoint);
    outerChordVec = gp_Vec(extendedOuterTePoint, extendedOuterLePoint);
    lp = innerChordVec.Dot(innerLeDirYZ);
    assert(fabs(lp) < Precision::Confusion());
    lp = outerChordVec.Dot(outerLeDirYZ);
    assert(fabs(lp) < Precision::Confusion());
#endif

    // build wires: etaLine, extendedEtaLine, leadingEdgeLine, extendedLeadingEdgeLine,
    //              trailingEdgeLine, extendedTrailingEdgeLine
    BRepBuilderAPI_MakeWire wbEta, wbExtEta, wbLe, wbExtLe, wbTe, wbExtTe;
    gp_Pnt innerLePoint, outerLePoint, innerPoint2d, outerPoint2d, innerTePoint, outerTePoint;
    TopoDS_Edge leEdge, etaEdge, teEdge, extLeEdge, extEtaEdge, extTeEdge;
    for (int i = 1; i < numberOfSections; i++) {
        innerLePoint = lePointContainer[i - 1];
        outerLePoint = lePointContainer[i];
        innerPoint2d = gp_Pnt(0, innerLePoint.Y(), innerLePoint.Z());
        outerPoint2d = gp_Pnt(0, outerLePoint.Y(), outerLePoint.Z());
        innerTePoint = tePointContainer[i - 1];
        outerTePoint = tePointContainer[i];
        leEdge = BRepBuilderAPI_MakeEdge(innerLePoint, outerLePoint);
        etaEdge = BRepBuilderAPI_MakeEdge(innerPoint2d, outerPoint2d);
        teEdge = BRepBuilderAPI_MakeEdge(innerTePoint, outerTePoint);
        if (i == 1) {
            innerLePoint = extendedInnerLePoint;
            innerPoint2d = gp_Pnt(0, innerLePoint.Y(), innerLePoint.Z());
            innerTePoint = extendedInnerTePoint;
        }
        if (i == numberOfSections - 1) {
            outerLePoint = extendedOuterLePoint;
            outerPoint2d = gp_Pnt(0, outerLePoint.Y(), outerLePoint.Z());
            outerTePoint = extendedOuterTePoint;
        }
        extLeEdge = BRepBuilderAPI_MakeEdge(innerLePoint, outerLePoint);
        extEtaEdge = BRepBuilderAPI_MakeEdge(innerPoint2d, outerPoint2d);
        extTeEdge = BRepBuilderAPI_MakeEdge(innerTePoint, outerTePoint);
        wbLe.Add(leEdge);
        wbEta.Add(etaEdge);
        wbTe.Add(teEdge);
        wbExtLe.Add(extLeEdge);
        wbExtEta.Add(extEtaEdge);
        wbExtTe.Add(extTeEdge);
    }
    leadingEdgeLine = wbLe.Wire();
    etaLine = wbEta.Wire();
    trailingEdgeLine = wbTe.Wire();
    extendedLeadingEdgeLine = wbExtLe.Wire();
    extendedEtaLine = wbExtEta.Wire();
    extendedTrailingEdgeLine = wbExtTe.Wire();
}

// Getter for midplane points
// returns the midplane points relative to wing coordinate system
// this method uses the eta/xsi definition for Spars, which uses the extended eta line
// see spars definition in CPACS documentation for details
// BUG #223: in case eta==0 or eta==1 the method returns the chordline points
gp_Pnt CCPACSWingComponentSegmentLineFunctionality::GetMidplanePoint(double eta, double xsi)
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetMidplanePoint", TIGL_ERROR);
    }
    if (xsi < 0.0 || xsi > 1.0) {
        throw CTiglError("Error: Parameter xsi not in the range 0.0 <= xsi <= 1.0 in CCPACSWingComponentSegment::GetMidplanePoint", TIGL_ERROR);
    }

    // BUG #223: fix for eta==0 or eta==1
    if (eta <= Precision::Confusion()) {
        return GetInnerChordlinePoint(xsi);
    }
    if (eta >= (1 - Precision::Confusion())) {
        return GetOuterChordlinePoint(xsi);
    }

    // get point on eta line
    gp_Pnt etaPnt;
    gp_Vec etaDir;
    BRepAdaptor_CompCurve etaLineCurve(GetExtendedEtaLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(etaLineCurve);
    etaLineCurve.D1(len * eta, etaPnt, etaDir);

    // get bounding box of leading edge line
    Bnd_Box bbox;
    BRepBndLib::Add(GetExtendedLeadingEdgeLine(), bbox);
    double xmin, xmax, temp;
    bbox.Get(xmin, temp, temp, xmax, temp, temp);

    // compute line along x-axis from eta-point and find intersection point on leading edge line
    Handle(Geom_TrimmedCurve) ray = GC_MakeSegment(gp_Pnt(xmin, etaPnt.Y(), etaPnt.Z()), gp_Pnt(xmax, etaPnt.Y(), etaPnt.Z()));
    BRepBuilderAPI_MakeEdge me(ray);
    TopoDS_Shape rayShape(me.Edge());
    // find intersection point on leading edge line, use minimum distance for stability
    BRepExtrema_DistShapeShape extrema(GetExtendedLeadingEdgeLine(), rayShape);
    extrema.Perform();
    gp_Pnt lePnt = extrema.PointOnShape1(1);

    // shortcut in case leading edge point is requested
    if (xsi == 0) {
        return lePnt;
    }

    // TODO: add support for closed wing (multiple points for single Y-coordinate)
    // determine up-vector for intersection face for cutting trailing edge
    gp_Pnt intersectPoint;
    gp_Pln plane(lePnt, etaDir);
    TopoDS_Face intersectFace = BRepBuilderAPI_MakeFace(plane).Face();
    if (!GetIntersectionPoint(intersectFace, GetExtendedTrailingEdgeLine(), intersectPoint)) {
        throw CTiglError("Unable to find trailing edge point in CCPACSWingComponentSegment::getMidplanePoint!");
    }
    gp_Pnt tePnt = intersectPoint;

    // xsi line
    TopoDS_Edge xsiEdge = BRepBuilderAPI_MakeEdge(lePnt, tePnt);
    TopoDS_Wire xsiLine = BRepBuilderAPI_MakeWire(xsiEdge);
    // get point on xsi line
    gp_Pnt xsiPnt;
    BRepAdaptor_CompCurve xsiLineCurve(xsiLine, Standard_True);
    len = GCPnts_AbscissaPoint::Length(xsiLineCurve);
    xsiLineCurve.D0(len * xsi, xsiPnt);

    return xsiPnt;
}

// Getter for the extended eta line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetEtaLine()
{
    CheckLines();
    return etaLine;
}

// Getter for the extended eta line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetExtendedEtaLine()
{
    CheckLines();
    return extendedEtaLine;
}

// Getter for the leading edge line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetLeadingEdgeLine()
{
    CheckLines();
    return leadingEdgeLine;
}

// Getter for the trailing edge line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetTrailingEdgeLine()
{
    CheckLines();
    return trailingEdgeLine;
}

// Getter for the extended leading edge line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetExtendedLeadingEdgeLine()
{
    CheckLines();
    return extendedLeadingEdgeLine;
}

// Getter for the extended trailing edge line
const TopoDS_Wire& CCPACSWingComponentSegmentLineFunctionality::GetExtendedTrailingEdgeLine()
{
    CheckLines();
    return extendedTrailingEdgeLine;
}

// Getter for leading edge point
gp_Pnt CCPACSWingComponentSegmentLineFunctionality::GetLeadingEdgePoint(double eta)
{
    // get point on leading edge line
    gp_Pnt lePnt;
    BRepAdaptor_CompCurve leLineCurve(GetLeadingEdgeLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(leLineCurve);
    leLineCurve.D0(len * eta, lePnt);

    return lePnt;
}

// Getter for trailing edge point
gp_Pnt CCPACSWingComponentSegmentLineFunctionality::GetTrailingEdgePoint(double eta)
{
    // get point on trailing edge line
    gp_Pnt tePnt;
    BRepAdaptor_CompCurve teLineCurve(GetTrailingEdgeLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(teLineCurve);
    teLineCurve.D0(len * eta, tePnt);

    return tePnt;
}

// Getter for inner chordline point
gp_Pnt CCPACSWingComponentSegmentLineFunctionality::GetInnerChordlinePoint(double xsi)
{
    gp_Pnt lePnt = GetLeadingEdgePoint(0);
    gp_Pnt tePnt = GetTrailingEdgePoint(0);
    gp_Vec chordLine(lePnt, tePnt);
    gp_Pnt result = lePnt.Translated(chordLine.Multiplied(xsi));
    return result;
}

// Getter for outer chordline point
gp_Pnt CCPACSWingComponentSegmentLineFunctionality::GetOuterChordlinePoint(double xsi)
{
    gp_Pnt lePnt = GetLeadingEdgePoint(1);
    gp_Pnt tePnt = GetTrailingEdgePoint(1);
    gp_Vec chordLine(lePnt, tePnt);
    gp_Pnt result = lePnt.Translated(chordLine.Multiplied(xsi));
    return result;
}

// Getter for the length of the leading edge between two eta values
double CCPACSWingComponentSegmentLineFunctionality::GetLeadingEdgeLength()
{
    BRepAdaptor_CompCurve leLineCurve(GetLeadingEdgeLine(), Standard_True);
    double length = GCPnts_AbscissaPoint::Length(leLineCurve);
    return length;
}

// Getter for the length of the trailing edge between two eta values
double CCPACSWingComponentSegmentLineFunctionality::GetTrailingEdgeLength()
{
    BRepAdaptor_CompCurve teLineCurve(GetTrailingEdgeLine(), Standard_True);
    double length = GCPnts_AbscissaPoint::Length(teLineCurve);
    return length;
}

// Getter for eta value for passed point
double CCPACSWingComponentSegmentLineFunctionality::GetMidplaneEta(const gp_Pnt& p)
{
    ShapeAnalysis_Curve sa;
    gp_Pnt etaPoint, projectedEtaPoint;
    Standard_Real curveParam;

    // Step1: compute intersection face for cutting with extended eta line
    gp_Pln plane = gp_Pln(p, gp_Dir(0, 1, 0));
    TopoDS_Face intersectFace = BRepBuilderAPI_MakeFace(plane).Face();

    // Step2: intersect face with extended eta line
    if (!GetIntersectionPoint(intersectFace, GetExtendedEtaLine(), etaPoint)) {
        throw CTiglError("Unable to find point on eta line in CCPACSWingComponentSegment::GetMidplaneEta!");
    }

    // Step3: get eta value of new point on extendedEtaLine
    BRepAdaptor_CompCurve extendedEtaLineCurve(GetExtendedEtaLine(), Standard_True);
    // get curveParam on extended eta line
    sa.Project(extendedEtaLineCurve, etaPoint, Precision::Confusion(), projectedEtaPoint, curveParam);
    // compute eta
    Standard_Real len = GCPnts_AbscissaPoint::Length(extendedEtaLineCurve);
    double eta = curveParam / len;
    if (eta < 0) {
        eta = 0;
    } else if (eta > 1) {
        eta = 1;
    }

#ifdef _DEBUG
    // first ensure that etaPoint and projectedEtaPoint are at identical positions
    if (etaPoint.Distance(projectedEtaPoint) > Precision::Confusion()) {
        throw CTiglError("INTERNAL ERROR: CCPACSWingComponentSegment::GetMidplaneEta(): projection of point in extended eta line resulted in different position!");
    }
    // now test whether the passed point lies on the xsi-line for the passed
    // eta point:
    // NOTE: since we can't ensure that the input point lies in the midplane
    // we must accept differences in the Z-axis. Thus we remove it from the
    // test points
    gp_Pnt testPnt = p;
    gp_Pnt testStartPnt = GetMidplanePoint(eta, 0);
    gp_Pnt testEndPnt = GetMidplanePoint(eta, 1);
    testPnt.SetZ(0);
    testStartPnt.SetZ(0);
    testEndPnt.SetZ(0);
    TopoDS_Vertex testV = BRepBuilderAPI_MakeVertex(testPnt);
    TopoDS_Shape xsiLine = BRepBuilderAPI_MakeEdge(testStartPnt, testEndPnt);
    // find intersection point on leading edge line, use minimum distance for stability
    BRepExtrema_DistShapeShape extrema(testV, xsiLine);
    extrema.Perform();
    double testDistance = extrema.Value();
    double testPrecision = 1E-6;
    if (testDistance > testPrecision) {
        // TODO: Bug #309: computation fails on dihedral combined with twist!!!
        //            tigl::CTiglCommon::dumpShape(testV, "C:/Tmp/occ", "testV");
        //            tigl::CTiglCommon::dumpShape(xsiLine, "C:/Tmp/occ", "xsiLine");
        //            tigl::CTiglCommon::dumpShape(loft, "C:/Tmp/occ", "loft");
        //            throw CTiglError("INTERNAL ERROR: CCPACSWingComponentSegment::GetMidplaneEta() computed wrong eta value");
    }
#endif
    return eta;
}

// Getter for eta direction of midplane
gp_Vec CCPACSWingComponentSegmentLineFunctionality::GetMidplaneEtaDir(double eta)
{
    // get point on eta line
    gp_Pnt etaPnt;
    gp_Vec etaDir;
    BRepAdaptor_CompCurve extendedEtaLineCurve(GetExtendedEtaLine(), Standard_True);
    Standard_Real len = GCPnts_AbscissaPoint::Length(extendedEtaLineCurve);
    extendedEtaLineCurve.D1(len * eta, etaPnt, etaDir);
    return etaDir.Normalized();
}

// Getter for midplane normal vector
gp_Vec CCPACSWingComponentSegmentLineFunctionality::GetMidplaneNormal(double eta)
{
    if (eta < 0.0 || eta > 1.0) {
        throw CTiglError("Error: Parameter eta not in the range 0.0 <= eta <= 1.0 in CCPACSWingComponentSegment::GetMidplanePoint", TIGL_ERROR);
    }

    gp_Pnt lePnt = GetMidplanePoint(eta, 0);
    gp_Pnt tePnt = GetMidplanePoint(eta, 1);
    gp_Vec etaDir = GetMidplaneEtaDir(eta);
    gp_Vec chordLine(lePnt, tePnt);
    gp_Vec normal = chordLine.Normalized().Crossed(etaDir);
    return normal;
}
}
