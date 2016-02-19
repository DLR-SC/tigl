#ifndef CCONTROLSURFACEBOARDERBUILDER_H
#define CCONTROLSURFACEBOARDERBUILDER_H

#include <gp_Pln.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <CTiglPoint.h>

namespace tigl
{

class CSCoordSystem
{
public:
    CSCoordSystem(gp_Pnt le, gp_Pnt te, gp_Vec yDir);
    
    gp_Vec getNormal() const;
    gp_Pnt getLe() const;
    gp_Pnt getTe() const;
    gp_Vec getXDir() const;
    gp_Vec getYDir() const;
    
    gp_Pln getPlane() const;
    
private:
    gp_Pnt _le, _te;
    gp_Vec _ydir;
};

class CControlSurfaceBoarderBuilder
{
public:
    CControlSurfaceBoarderBuilder(const CSCoordSystem& coords, TopoDS_Shape wingShape);
    ~CControlSurfaceBoarderBuilder();
    
    TopoDS_Wire boarderWithLEShape(double rLEHeight, double xsiUpper, double xsiLower);
    TopoDS_Wire boarderSimple();
    
    gp_Pnt2d upperPoint();
    gp_Pnt2d lowerPoint();
    
    gp_Vec2d upperTangent();
    gp_Vec2d lowerTangent();
    
private:
    CControlSurfaceBoarderBuilder();
    
    /// Cuts the wing shape with the plane defined in _coords
    void computeWingCutWire();
    
    /// Computes the point on the wing skin and its tangents
    void computeSkinPoint(double xsi, bool getUpper, gp_Pnt2d& pnt, gp_Vec2d& tangent);
    
    TopoDS_Wire _wingCutWire;
    TopoDS_Face _wingCutFace;
    
    TopoDS_Shape _wingShape;
    const CSCoordSystem& _coords;
    
    gp_Pnt2d _up2d, _lp2d, _le2d, _te2d;
    gp_Vec2d _upTan2d, _loTan2d;
};


}
#endif // CCONTROLSURFACEBOARDERBUILDER_H
