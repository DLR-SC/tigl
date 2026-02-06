


#include "CPACSProfileGeometry.h"
//#include "CPACSNacaProfile.h"
#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"
#include "Cache.h"
#include "geometry/CFunctionToBspline.h"
#include "NACA4Calculator.h"
#include "common/tiglcommonfunctions.h"


#include <vector>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>


namespace tigl
{
// Constructor

CTiglWingProfileNACA::CTiglWingProfileNACA(const CCPACSWingProfile& profile, const std::string& naca_code)
    : profileUID(profile.GetUID())
    , calculator(naca_code)
    , wireCache(*this, &CTiglWingProfileNACA::BuildWires)
{}

void CTiglWingProfileNACA::Invalidate() const
{
  
}

/*
void CTiglWingProfileNACA::BuildNaca(NacaCache& cache) const //NacaCache?
{
//LinspaceWithBreaks
//dumpshape
//auf die richtung achten
//mathfunc3d auch implementiern (abgelitete klassen machn mit get x)

    double max_camber;
    double max_camber_pos;
    double max_profile_thickenss;
    double te_thickness;
    NACA4Calculator naca4(max_camber, max_camber_pos, max_profile_thickenss, te_thickness);

    std::vector<double> point_distribution = LinspaceWithBreaks(0.0, 1.0, 20, {}); //anfangpkt, endpunkt, anzahlpkt, evtlbreaks

    std::vector<gp_Pnt> upper_points_vec, lower_points_vec;
    for (double x : point_distribution){
        gp_Vec2d upper_point = naca4.upper_curve(x);
        gp_Vec2d lower_point = naca4.lower_curve(x);

        upper_points_vec.emplace_back(upper_point.X(), upper_point.Y(), 0.0); //hier wird jeder erstellt punkt reinkopiert, das alle punkt egesichert werden
        lower_points_vec.emplace_back(lower_point.X(), lower_point.Y(), 0.0);
    }
    
    TopoDS_Edge  upper_edge = CFunctionToBspline(upper_points_vec).GetEdge();
    TopoDS_Edge lower_edge = CFunctionToBspline(lower_points_vec).GetEdge();

    cache.upper_edge = upper_edge;
    cache.lower = lower_edge;
}

*/
void CTiglWingProfileNACA::BuildWires(WireCache& cache) const
{
    //hier edges erstellen aus der upper_bspline vom calculator
    cache.upperWire = BRepBuilderAPI_MakeEdge(calculator.upper_bspline()); //hier fehln argumente
    
}



const std::vector<CTiglPoint>& CTiglWingProfileNACA::GetSamplePoints() const {
    static std::vector<CTiglPoint> dummy;
    return dummy;
}


const TopoDS_Edge& CTiglWingProfileNACA::GetUpperWire(TiglShapeModifier mod) const
{
    return wireCache->upperWire;
}

const TopoDS_Edge& CTiglWingProfileNACA::GetLowerWire(TiglShapeModifier mod) const
{
    return wireCache->lowerWire;
}

// gets the upper and lower wing profile into on edge
const TopoDS_Edge& CTiglWingProfileNACA::GetUpperLowerWire(TiglShapeModifier mod) const
{
    return wireCache->upperLowerEdge;
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
    return true; //TODO 

}
}//namespace tigl