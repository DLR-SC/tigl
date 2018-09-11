/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
/**
* @file
* @brief  Implementation of CPACS wing profile as a CST profile
*
* The wing profile is defined by the (C)lass function / (S)hape function (T)ransformation
* geometry representation method.
*/

#include "CCPACSWingProfileCST.h"

#include "CCSTCurveBuilder.h"

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglTransformation.h"
#include "CWireToCurve.h"
#include "math.h"

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Standard_Version.hxx>


namespace tigl
{
// Constructor
CCPACSWingProfileCST::CCPACSWingProfileCST()
    : wireCache(*this, &CCPACSWingProfileCST::BuildWires)
{
}

void CCPACSWingProfileCST::Invalidate()
{
    wireCache.clear();
}

// Builds the wing profile wire. The returned wire is already transformed by the
// wing profile element transformation.
void CCPACSWingProfileCST::BuildWires(WireCache& cache) const
{
    gp_Trsf yzSwitch;
    yzSwitch.SetMirror(gp_Ax2(gp_Pnt(0.,0.,0.), gp_Dir(0.,-1.,1.)));
    
    // Build upper wire
    CCSTCurveBuilder upperBuilder(m_upperN1, m_upperN2, m_upperB.AsVector());
    Handle(Geom_BSplineCurve) upperCurve = upperBuilder.Curve();
    upperCurve->Transform(yzSwitch);
    cache.upperWire = BRepBuilderAPI_MakeEdge(upperCurve);
    
    // Build lower curve
    std::vector<double> binv = m_lowerB.AsVector();
    for (unsigned int i = 0; i < binv.size(); ++i) {
        binv[i] = -binv[i];
    }
    
    CCSTCurveBuilder lowerBuilder(m_lowerN1, m_lowerN2, binv);
    Handle(Geom_BSplineCurve) lowerCurve = lowerBuilder.Curve();
    lowerCurve->Transform(yzSwitch);
    lowerCurve->Reverse();
    cache.lowerWire = BRepBuilderAPI_MakeEdge(lowerCurve);
    
    BRepBuilderAPI_MakeWire upperLowerWireMaker(cache.lowerWire, cache.upperWire);
    TopoDS_Wire upperLowerWire = upperLowerWireMaker.Wire();
    
    // conatenate wire
    Handle(Geom_Curve) upperLowerCurve = CWireToCurve(upperLowerWire).curve();
    cache.upperLowerEdge = BRepBuilderAPI_MakeEdge(upperLowerCurve);
    
    cache.tePoint = gp_Pnt(1,0,0);
    cache.lePoint = gp_Pnt(0,0,0);
}

// Returns sample points
std::vector<CTiglPoint>& CCPACSWingProfileCST::GetSamplePoints() {
    static std::vector<CTiglPoint> dummy;
    return dummy;
}

const std::vector<CTiglPoint>& CCPACSWingProfileCST::GetSamplePoints() const {
    static std::vector<CTiglPoint> dummy;
    return dummy;
}

// get upper wing profile wire
const TopoDS_Edge& CCPACSWingProfileCST::GetUpperWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        return wireCache->upperWire;
    default:
        throw CTiglError("GetUpperWire with profile modifications not implemented for CCPACSWingProfileCST yet!");
    }
}

// get lower wing profile wire
const TopoDS_Edge& CCPACSWingProfileCST::GetLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        return wireCache->lowerWire;
    default:
        throw CTiglError("GetLowerWire with profile modifications not implemented for CCPACSWingProfileCST yet!");
    }
}

// gets the upper and lower wing profile into on edge
const TopoDS_Edge& CCPACSWingProfileCST::GetUpperLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        return wireCache->upperLowerEdge;
    default:
        throw CTiglError("GetUpperLowerWire with profile modifications not implemented for CCPACSWingProfileCST yet!");
    }
}

// get trailing edge
const TopoDS_Edge& CCPACSWingProfileCST::GetTrailingEdge(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        return wireCache->trailingEdge;
    default:
        throw CTiglError("GetTrailingEdge with profile modifications not implemented for CCPACSWingProfileCST yet!");
    }
}

// get leading edge point();
const gp_Pnt & CCPACSWingProfileCST::GetLEPoint() const
{
    return wireCache->lePoint;
}
        
// get trailing edge point();
const gp_Pnt & CCPACSWingProfileCST::GetTEPoint() const
{
    return wireCache->tePoint;
}

} // end namespace tigl
