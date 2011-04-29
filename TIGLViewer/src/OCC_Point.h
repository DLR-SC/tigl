


#if !defined(AFX_OCC_Point_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_)
#define AFX_OCC_Point_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <Standard_Macro.hxx>
#include <Standard_DefineHandle.hxx>
#include <AIS_InteractiveObject.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <AIS_Shape.hxx>
#include <gp_Lin.hxx>
#include <Geom_Line.hxx>
#include <gce_MakeLin.hxx>
#include <GC_MakeSegment.hxx>
#include "TopoDS.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "BRep_Tool.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "TopLoc_Location.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Builder.hxx"
#include "Poly_Array1OfTriangle.hxx"
#include "gp_Trsf.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepMesh.hxx"
#include "BRepTools.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "TopoDS_Compound.hxx"
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Real.hxx>


DEFINE_STANDARD_HANDLE(OCC_Point,AIS_InteractiveObject)
class OCC_Point : public AIS_InteractiveObject
{
public:
	OCC_Point();
    OCC_Point(Standard_Real X,Standard_Real Y ,Standard_Real Z);
    OCC_Point(gp_Pnt2d& aPoint,Standard_Real Elevation = 0);
    OCC_Point(gp_Pnt& aPoint);

	virtual ~OCC_Point();
    DEFINE_STANDARD_RTTI(OCC_Point)

private :

void Compute          (const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                       const Handle(Prs3d_Presentation)& aPresentation,
                       const Standard_Integer aMode);
void Compute          (const Handle(Prs3d_Projector)& aProjector,
                       const Handle(Prs3d_Presentation)& aPresentation) ;
void Compute          (const Handle(PrsMgr_PresentationManager2d)& aPresentationManager,
                       const Handle(Graphic2d_GraphicObject)& aGrObj,
                       const Standard_Integer unMode = 0) ;
void ComputeSelection (const Handle(SelectMgr_Selection)& aSelection,
                       const Standard_Integer unMode) ;

gp_Pnt myPoint;

};

#endif // !defined(AFX_OCC_Point_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_)
