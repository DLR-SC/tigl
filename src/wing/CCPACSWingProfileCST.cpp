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
    
    double teThickness = HasBluntTE() ? *m_trailingEdgeThickness : 0.;
    
    // Build upper wire
    CCSTCurveBuilder upperBuilderOpened(m_upperN1, m_upperN2, m_upperB.AsVector(), teThickness/2.);
    Handle(Geom_BSplineCurve) upperCurve = upperBuilderOpened.Curve();
    upperCurve->Transform(yzSwitch);
    cache.upperWireOpened = BRepBuilderAPI_MakeEdge(upperCurve);
    
    CCSTCurveBuilder upperBuilderClosed(m_upperN1, m_upperN2, m_upperB.AsVector(), 0.); 
    Handle(Geom_BSplineCurve) upperCurveClosed = upperBuilderClosed.Curve();
    upperCurveClosed->Transform(yzSwitch);
    cache.upperWireClosed = BRepBuilderAPI_MakeEdge(upperCurveClosed);
    
    // Build lower curve
    std::vector<double> binv = m_lowerB.AsVector();
    for (unsigned int i = 0; i < binv.size(); ++i) {
        binv[i] = -binv[i];
    }
    
    CCSTCurveBuilder lowerBuilderOpened(m_lowerN1, m_lowerN2, binv, -teThickness/2.);
    Handle(Geom_BSplineCurve) lowerCurve = lowerBuilderOpened.Curve();
    lowerCurve->Transform(yzSwitch);
    lowerCurve->Reverse();
    cache.lowerWireOpened = BRepBuilderAPI_MakeEdge(lowerCurve);

    CCSTCurveBuilder lowerBuilderClosed(m_lowerN1, m_lowerN2, binv, 0.);
    Handle(Geom_BSplineCurve) lowerCurveClosed = lowerBuilderClosed.Curve();
    lowerCurveClosed->Transform(yzSwitch);
    lowerCurveClosed->Reverse();
    cache.lowerWireClosed = BRepBuilderAPI_MakeEdge(lowerCurveClosed);

    BRepBuilderAPI_MakeWire upperLowerWireMaker(cache.lowerWireOpened, cache.upperWireOpened);
    TopoDS_Wire upperLowerWire = upperLowerWireMaker.Wire();
    
    BRepBuilderAPI_MakeWire upperLowerWireMakerClosed(cache.lowerWireClosed, cache.upperWireClosed);
    TopoDS_Wire upperLowerWireClosed = upperLowerWireMaker.Wire();
    
    // conatenate wire
    Handle(Geom_Curve) upperLowerCurve = CWireToCurve(upperLowerWire).curve();
    cache.upperLowerEdgeOpened = BRepBuilderAPI_MakeEdge(upperLowerCurve);

    Handle(Geom_Curve) upperLowerCurveClosed = CWireToCurve(upperLowerWireClosed).curve();
    cache.upperLowerEdgeClosed = BRepBuilderAPI_MakeEdge(upperLowerCurveClosed);
    
    cache.tePoint = gp_Pnt(1,0,0);
    cache.lePoint = gp_Pnt(0,0,0);
    
    if (teThickness > 0) {
        gp_Pnt te_up, te_down;
        te_up = upperCurve->EndPoint();
        te_down = lowerCurve->StartPoint();
        cache.trailingEdgeOpened = BRepBuilderAPI_MakeEdge(te_up,te_down);
    }
    else {
        cache.trailingEdgeOpened.Nullify();
    }

    cache.trailingEdgeClosed.Nullify();
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
        if (!HasBluntTE()) {
            return wireCache->upperWireClosed;
        }
        else {
            return wireCache->upperWireOpened;
        }
    case SHARP_TRAILINGEDGE:
        return wireCache->upperWireClosed;
    case BLUNT_TRAILINGEDGE:
        return wireCache->upperWireOpened;
    }
    throw CTiglError("Unknown TiglShapeModifier passed to CCPACSWingProfileCST::GetUpperWire");
}

// get lower wing profile wire
const TopoDS_Edge& CCPACSWingProfileCST::GetLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (!HasBluntTE()) {
            return wireCache->lowerWireClosed;
        }
        else {
            return wireCache->lowerWireOpened;
        }
    case SHARP_TRAILINGEDGE:
        return wireCache->lowerWireClosed;
    case BLUNT_TRAILINGEDGE:
        return wireCache->lowerWireOpened;
    }
    throw CTiglError("Unknown TiglShapeModifier passed to CCPACSWingProfileCST::GetLowerWire");
}

// gets the upper and lower wing profile into on edge
const TopoDS_Edge& CCPACSWingProfileCST::GetUpperLowerWire(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (!HasBluntTE()) {
            return wireCache->upperLowerEdgeClosed;
        }
        else {
            return wireCache->upperLowerEdgeOpened;
        }
        break;
    case SHARP_TRAILINGEDGE:
        return wireCache->upperLowerEdgeClosed;
    case BLUNT_TRAILINGEDGE:
        return wireCache->upperLowerEdgeOpened;
    }
    throw CTiglError("Unknown TiglShapeModifier passed to CCPACSWingProfileCST::GetUpperLowerWire");
}

// get trailing edge
const TopoDS_Edge& CCPACSWingProfileCST::GetTrailingEdge(TiglShapeModifier mod) const
{
    switch (mod) {
    case UNMODIFIED_SHAPE:
        if (!HasBluntTE()) {
            return wireCache->trailingEdgeClosed;
        }
        else {
            return wireCache->trailingEdgeOpened;
        }
    case SHARP_TRAILINGEDGE:
        return wireCache->trailingEdgeClosed;
    case BLUNT_TRAILINGEDGE:
        return wireCache->trailingEdgeOpened;
    }
    throw CTiglError("Unknown TiglShapeModifier passed to CCPACSWingProfileCST::GetTrailingEdge");
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

bool CCPACSWingProfileCST::HasBluntTE() const
{
    return m_trailingEdgeThickness && *m_trailingEdgeThickness > 0 ? true : false;
}

} // end namespace tigl
