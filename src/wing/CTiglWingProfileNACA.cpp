


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

    auto upper_bspline = calculator.upper_bspline();
    auto lower_bspline = calculator.lower_bspline()->Reversed();
    //hier edges erstellen aus der upper_bspline vom calculator
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

    throw CTiglError("UpperLower wire is not implemented.");
    //  return wireCache->upperLowerEdge;
}

// get trailing edge
const TopoDS_Edge& CTiglWingProfileNACA::GetTrailingEdge(TiglShapeModifier mod) const
{

    return wireCache->trailingEdge;
    //obere und unterekoordinate bei x = 1 und dann dazwischen eine edge erstellen
    //im constructor die te thickness noch mit übergeben
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