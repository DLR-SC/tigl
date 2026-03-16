/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief builds the wing profile for NACA profiles based on the NACA4Calculator and returns the upper and lower wire, the trailing edge and the leading edge point
*/


#include "CPACSProfileGeometry.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "Cache.h"
#include "geometry/CFunctionToBspline.h"
#include "NACA4Calculator.h"
#include "common/tiglcommonfunctions.h"


#include <vector>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Line.hxx>


namespace tigl
{
// Constructor

CTiglWingProfileNACA::CTiglWingProfileNACA(const CCPACSWingProfile& profile, const generated::CPACSNacaProfile& nacadef)
    : profileUID(profile.GetUID())
    , wireCache(*this, &CTiglWingProfileNACA::BuildWires)
{
    double te_thickness = nacadef.GetTrailingEdgeThickness() ? *nacadef.GetTrailingEdgeThickness() : 0.0;
    if(auto const& naca4 = nacadef.GetNaca4DigitCode_choice1(); naca4){
        calculator = NACA4Calculator(*naca4, te_thickness);
    }
    else{
        throw CTiglError("ERROR in CTiglWingProfileNACA: Currently only 4 digit NACA codes implemented.");
    }

}

void CTiglWingProfileNACA::Invalidate() const
{
    wireCache.clear();
}

void CTiglWingProfileNACA::BuildWires(WireCache& cache) const
{

    auto upper_bspline = calculator.upper_bspline();
    auto lower_bspline = calculator.lower_bspline()->Reversed();
    cache.upperWire = BRepBuilderAPI_MakeEdge(upper_bspline); 
    cache.lowerWire = BRepBuilderAPI_MakeEdge(lower_bspline); 

    gp_Vec2d le_pnt = calculator.upper_curve(0);
    gp_Vec2d upper_coord = calculator.upper_curve(1);
    gp_Vec2d lower_coord = calculator.lower_curve(1);
    gp_Vec2d te_pnt = 0.5*(upper_coord + lower_coord);

    cache.lePoint = gp_Pnt(le_pnt.X(), 0.0, le_pnt.Y());
    cache.tePoint = gp_Pnt(te_pnt.X(), 0.0, te_pnt.Y());

    // build trailing edge
    if (HasBluntTE()) {

        double upper_x_coord = upper_coord.X();
        double lower_x_coord = lower_coord.X();
        double upper_y_coord = upper_coord.Y();
        double lower_y_coord = lower_coord.Y();

        gp_Pnt P1(upper_x_coord, 0.0, upper_y_coord);
        gp_Pnt P2(lower_x_coord, 0.0, lower_y_coord);
        
        cache.trailingEdge = BRepBuilderAPI_MakeEdge(P1, P2).Edge();
    } else {
        cache.trailingEdge = TopoDS_Edge();
    }
}


const std::vector<CTiglPoint>& CTiglWingProfileNACA::GetSamplePoints() const {
    static std::vector<CTiglPoint> dummy;
    return dummy;
}

// get upper wing profile wire
const TopoDS_Edge& CTiglWingProfileNACA::GetUpperWire(TiglShapeModifier mod) const
{
    return wireCache->upperWire;
}

// get lower wing profile wire
const TopoDS_Edge& CTiglWingProfileNACA::GetLowerWire(TiglShapeModifier mod) const
{
    return wireCache->lowerWire;
}

// gets the upper and lower wing profile into on edge
const TopoDS_Edge& CTiglWingProfileNACA::GetUpperLowerWire(TiglShapeModifier mod) const
{

    throw CTiglError("UpperLower wire is not implemented.");
    //  return wireCache->upperLowerEdge;
}

// get trailing edge
const TopoDS_Edge& CTiglWingProfileNACA::GetTrailingEdge(TiglShapeModifier mod) const
{

    return wireCache->trailingEdge;
}

// get leading edge point();
const gp_Pnt & CTiglWingProfileNACA::GetLEPoint() const
{
    return wireCache->lePoint;
}

// get trailing edge point();
const gp_Pnt & CTiglWingProfileNACA::GetTEPoint() const
{
    return wireCache->tePoint;
}

bool CTiglWingProfileNACA::HasBluntTE() const
{
    return calculator.get_trailing_edge_thickness() > 0.;

}
}//namespace tigl