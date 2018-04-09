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
#include <map>

#include <CCPACSGuideCurve.h>
#include <CPACSPointXYZ.h>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <Precision.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColStd_HArray1OfBoolean.hxx>

namespace tigl {

CTiglCurveConnector::CTiglCurveConnector(std::vector<CCPACSGuideCurve*> roots)
    : m_roots(roots)
{}

void CTiglCurveConnector::Invalidate()
{
    m_result.Nullify();
    m_isBuilt = false;
}

TopoDS_Compound CTiglCurveConnector::GetGuideCurves()
{
    if( m_isBuilt ) {
        return m_result;
    }

    Invalidate();
    BRep_Builder builder;
    builder.MakeCompound(m_result);

    std::vector<CCPACSGuideCurve*>::iterator it;
    for (it = m_roots.begin(); it != m_roots.end(); ++it) {

        CCPACSGuideCurve* curCurve = *it;
        TopoDS_Wire connectedGuide = GetInterpolatedCurveFromRoot(curCurve);
        builder.Add(m_result, connectedGuide);

    } // for roots

    m_isBuilt = true;
    return m_result;
}

TopoDS_Wire CTiglCurveConnector::GetInterpolatedCurveFromRoot(CCPACSGuideCurve* curCurve)
{
    // interpolate guide curve points of all segments with a B-Spline
    std::vector<gp_Pnt> points;
    std::vector<gp_Vec> tangents;
    std::vector<bool>   tangentFlags;

    // add root point of the guide curve
    points.push_back( curCurve->GetCurvePoints()[0] );

    size_t idx_start = 1;
    size_t idx_end = 0;
    while (curCurve) {

        // get points of current curve and append point list
        std::vector<gp_Pnt> curPoints = curCurve->GetCurvePoints();

        // check if the first point is too close to the last one
        if ( points.back().Distance(curPoints[0]) < Precision::Confusion() ) {
            // ignore first point to avoid duplicity
            idx_end = idx_start + curPoints.size() - 2;
            points.insert( points.end(), curPoints.begin()+1, curPoints.end() );
        }
        else {
            idx_end = idx_start + curPoints.size() - 1;
            points.insert( points.end(), curPoints.begin(), curPoints.end() );
        }

        // no tangents given for the points by default
        std::vector<gp_Vec> curTangents(curPoints.size());
        std::fill(curTangents.begin(), curTangents.end(), gp_Vec(0, 0, 0));
        tangents.insert(tangents.end(), curTangents.begin(), curTangents.end());

        std::vector<bool> curTangentFlags(curPoints.size());
        std::fill(curTangentFlags.begin(), curTangentFlags.end(), false);
        tangentFlags.insert(tangentFlags.end(), curTangentFlags.begin(), curTangentFlags.end());

        // check if a tangent for the first point is given (only at root).
        if ( curCurve->GetTangent_choice2() ) {
            generated::CPACSPointXYZ& tangent = *(curCurve->GetTangent_choice2());
            tangents[idx_start] = gp_Vec( tangent.GetX(), tangent.GetY(), tangent.GetZ());
            tangentFlags[idx_start] = true;
        }

        // check if a tangent for the last point is given
        if ( curCurve->GetTangent() ) {
            generated::CPACSPointXYZ& tangent = *(curCurve->GetTangent());
            tangents[idx_end] = gp_Vec( tangent.GetX(), tangent.GetY(), tangent.GetZ());
            tangentFlags[idx_end] = true;
        }

        curCurve = curCurve->GetConnectedCurve();
        idx_start = idx_end+1;
    }

    int pointCount = (int)idx_start;
    Handle(TColgp_HArray1OfPnt) hpoints = new TColgp_HArray1OfPnt(1, pointCount);
    Handle(TColStd_HArray1OfBoolean) htangentFlags = new TColStd_HArray1OfBoolean(1, pointCount);
    TColgp_Array1OfVec htangents(1, pointCount);

    for (int j = 0; j < pointCount; j++) {
        hpoints->SetValue(j+1, points[j]);
        htangents.SetValue(j+1, tangents[j]);
        htangentFlags->SetValue(j+1, tangentFlags[j]);
    }

    GeomAPI_Interpolate interpol(hpoints, Standard_False, Precision::Confusion());
    interpol.Load(htangents, htangentFlags);
    interpol.Perform();

    Handle(Geom_BSplineCurve) hcurve = interpol.Curve();
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hcurve);
    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(edge);
    return wireMaker.Wire();
}

} // namespace tigl

