/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2018 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "CTiglCurveConnector.h"

#include <algorithm>
#include <stack>

#include <CCPACSGuideCurve.h>
#include <CPACSPointXYZ.h>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Precision.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColStd_HArray1OfBoolean.hxx>
#include <ShapeAnalysis_Edge.hxx>

namespace tigl {

CTiglCurveConnector::CTiglCurveConnector(std::vector<CCPACSGuideCurve*> roots)
{
    // check if all guide curves have the same number of segments
    VerifyNumberOfSegments(roots);

    // at each root, a connected guide curve starts.
    //  * A connected guide curve consists of a list of partial guide curves.
    //  * A partial guide curve consists of a list of segmentwise guide curves.
    m_connectedCurves.reserve(roots.size());
    for (int i=0; i< roots.size(); i++) {
        guideCurveConnected connectedCurve;
        m_connectedCurves.push_back(connectedCurve);
        CreatePartialCurves(m_connectedCurves.back(), roots[i]);
    }

    // For every connected guide curve, create the interpolation order according
    // to the depencies of the partial curves
    for (int i=0; i< roots.size(); i++) {
        CreateInterpolationOrder(m_connectedCurves[i]);
    }
}

TopoDS_Compound CTiglCurveConnector::GetConnectedGuideCurves()
{
    TopoDS_Compound result;
    BRep_Builder builder;
    builder.MakeCompound(result);

    // iterate list of guide curves
    std::vector<guideCurveConnected>::iterator it;
    for (it = m_connectedCurves.begin(); it != m_connectedCurves.end(); ++it) {
        guideCurveConnected curCurve = *it;

        // interpolate the guide curve parts of the current guide curve
        for ( int i=0; i< curCurve.parts.size(); i++ ) {
            int idx = curCurve.interpolationOrder[i];
            InterpolateGuideCurvePart(curCurve, idx);
        }

        // connect the guide curve parts to a wire
        BRepBuilderAPI_MakeWire wireMaker;
        for ( int i=0; i< curCurve.parts.size(); i++ ) {
            wireMaker.Add(curCurve.parts[i].localCurve);
        }

        // add the wire of the current guide curve to the compound
        builder.Add(result, wireMaker.Wire());
    }

    return result;
}

void CTiglCurveConnector::VerifyNumberOfSegments(std::vector<CCPACSGuideCurve*>& roots)
{
    // check if every guidecurve consists of the same number of segments
    int numSegments_prev = 0;
    int numSegments = 0;
    for (int i =0; i<roots.size(); i++) {
        numSegments = 0;
        CCPACSGuideCurve* curCurve = roots[i];
        while (curCurve) {
            numSegments++;
            curCurve = curCurve->GetConnectedCurve();
        }
        if (i>0 && numSegments != numSegments_prev ) {
           throw(CTiglError("The guide curves to be connected do not have the same number of segments!"));
        }
        numSegments_prev = numSegments;
    }
}

void CTiglCurveConnector::CreatePartialCurves(guideCurveConnected& connectedCurve, CCPACSGuideCurve* current)
{
    connectedCurve.parts.push_back(guideCurvePart());

    while (current && !current->GetContinuity_choice1() ) {
        connectedCurve.parts.back().localGuides.push_back(current);
        current = current->GetConnectedCurve();
    }

    if (current) {
        CreatePartialCurves(connectedCurve, current);
    }

}

void CTiglCurveConnector::CreateInterpolationOrder (guideCurveConnected& connectedCurve)
{
    // this is essentially a topological sort

    size_t nparts = connectedCurve.parts.size();

    //compute in-degree of all partial curves
    std::vector<int> indegrees(nparts);
    for(int ipart=0; ipart<nparts; ipart++) {
        indegrees[ipart]=0;
        CCPACSGuideCurve* partRoot = connectedCurve.parts[ipart].localGuides[0];
        if ( partRoot->GetContinuity_choice1() ) {

            bool from =    partRoot->GetContinuity_choice1() == generated::C1_from_previous
                        || partRoot->GetContinuity_choice1() == generated::C2_from_previous;
            bool to   =    partRoot->GetContinuity_choice1() == generated::C1_to_previous
                        || partRoot->GetContinuity_choice1() == generated::C2_to_previous;
            if ( to ) {
                connectedCurve.parts[ipart].dependency = C2_to_previous;
                indegrees[ipart-1]++;
            } else if ( from ) {
                connectedCurve.parts[ipart].dependency = C2_from_previous;
                indegrees[ipart]++;
            }
        }
    }

    // add all partial curves with zero degree to the stack
    std::stack<int> stack;
    for(int ipart=0; ipart<nparts; ipart++) {
        if (indegrees[ipart]==0 ) {
            stack.push(ipart);
        }
    }

    // DFS through the dependency tree
    int counter = 0;
    while ( !stack.empty() ) {

        // top of stack is the next in the topo sort
        int idx = stack.top();
        stack.pop();
        connectedCurve.interpolationOrder.push_back(idx);

        // see if we can add neighbors to the stack
        if ( connectedCurve.parts[idx].dependency == C2_to_previous ) {
            indegrees[idx-1]--;
            if ( indegrees[idx-1]==0 ) {
                stack.push(idx-1);
            }
        }
        if ( connectedCurve.parts[idx+1].dependency == C2_from_previous ) {
            indegrees[idx+1]--;
            if ( indegrees[idx+1]==0 ) {
                stack.push(idx+1);
            }
        }

    } // while ( !stack.empty() )
}

void CTiglCurveConnector::InterpolateGuideCurvePart(guideCurveConnected& connectedCurve, int partIndex) {

    guideCurvePart& curvePart = connectedCurve.parts[partIndex];

    // interpolate guide curve points of all segments of the part with a B-Spline
    std::vector<gp_Pnt> points;
//    std::vector<double> params;
    std::vector<gp_Vec> tangents;
    std::vector<bool>   tangentFlags;

    // add first point of this partial curve to the point list
    points.push_back( curvePart.localGuides[0]->GetCurvePoints()[0] );

    // check if a tangent for the first point is prescribed in CPACS
    if ( curvePart.localGuides[0]->GetTangent_choice2() ) {
        generated::CPACSPointXYZ& tangent = *(curvePart.localGuides[0]->GetTangent_choice2());
        tangents[0] = gp_Vec( tangent.GetX(), tangent.GetY(), tangent.GetZ());
        tangentFlags[0] = true;
    }

    // check if we have a "from" dependency and prescribe tangent accordingly
    if ( partIndex > 0 && curvePart.dependency == C2_from_previous ) {
        guideCurvePart leftNeighbor = connectedCurve.parts[partIndex-1];
        // get tangent at last point of leftNeighbor.localCurve
        ShapeAnalysis_Edge edgeAnalyser;
        Handle(Geom_Curve) geomCurve;
        Standard_Real startParam, endParam;
        gp_Pnt endPoint;
        gp_Vec endTangent;
        edgeAnalyser.Curve3d(leftNeighbor.localCurve, geomCurve, startParam, endParam);
        geomCurve->D1(endParam, endPoint, endTangent);
        tangents[0] = endTangent;
        tangentFlags[0] = true;
    }

    // construct point list and prescribed tangents
    for (int isegment = 0; isegment< curvePart.localGuides.size(); isegment++) {

        // append point list with points of the given segment
        // igore the first point on the section to avoid duplicity
        std::vector<gp_Pnt> curPoints = curvePart.localGuides[isegment]->GetCurvePoints();
        points.insert( points.end(), curPoints.begin()+1, curPoints.end() );

        size_t idx_end = points.size();

        // no tangents given for the points by default
        std::vector<gp_Vec> curTangents(curPoints.size());
        std::fill(curTangents.begin(), curTangents.end(), gp_Vec(0, 0, 0));
        tangents.insert(tangents.end(), curTangents.begin(), curTangents.end());

        std::vector<bool> curTangentFlags(curPoints.size());
        std::fill(curTangentFlags.begin(), curTangentFlags.end(), false);
        tangentFlags.insert(tangentFlags.end(), curTangentFlags.begin(), curTangentFlags.end());

        // check if a tangent for the last point is prescribed in CPACS
        if ( curvePart.localGuides[isegment]->GetTangent() ) {
            generated::CPACSPointXYZ& tangent = *(curvePart.localGuides[isegment]->GetTangent());
            tangents[idx_end] = gp_Vec( tangent.GetX(), tangent.GetY(), tangent.GetZ());
            tangentFlags[idx_end] = true;
        }

    } // for all local guides

    // check if we have a "from" dependency and prescribe tangent accordingly
    if ( partIndex+1 < connectedCurve.parts.size() ) {
        guideCurvePart rightNeighbor = connectedCurve.parts[partIndex+1];
        if ( rightNeighbor.dependency == C2_to_previous ) {
            // get tangent at first point of rightNeighbor.localCurve
            ShapeAnalysis_Edge edgeAnalyser;
            Handle(Geom_Curve) geomCurve;
            Standard_Real startParam, endParam;
            gp_Pnt startPoint;
            gp_Vec startTangent;
            edgeAnalyser.Curve3d(rightNeighbor.localCurve, geomCurve, startParam, endParam);
            geomCurve->D1(startParam, startPoint, startTangent);
            tangents.back() = startTangent;
            tangentFlags.back() = true;
        }
    }

    int pointCount = (int)points.size();
    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, pointCount);
//    Handle(TColStd_HArray1OfReal) hparams = new TColStd_HArray1OfReal(1, pointCount);
    Handle(TColStd_HArray1OfBoolean) htangentFlags = new TColStd_HArray1OfBoolean(1, pointCount);
    TColgp_Array1OfVec htangents(1, pointCount);

    for (int j = 0; j < pointCount; j++) {
        hpoints->SetValue(j+1, points[j]);
//        hparams->SetValue(j+1, params[j]);
        htangents.SetValue(j+1, tangents[j]);
        htangentFlags->SetValue(j+1, tangentFlags[j]);
    }

//    GeomAPI_Interpolate interpol(hpoints, hparams, Standard_False, Precision::Confusion());
    GeomAPI_Interpolate interpol(hpoints, Standard_False, Precision::Confusion());
    interpol.Load(htangents, htangentFlags);
    interpol.Perform();
    Handle(Geom_BSplineCurve) hcurve = interpol.Curve();
    curvePart.localCurve = BRepBuilderAPI_MakeEdge(hcurve);
}


} // namespace tigl

