

#include "OCC_Point.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
IMPLEMENT_STANDARD_HANDLE(OCC_Point,AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTI(OCC_Point,AIS_InteractiveObject)
//
// Foreach ancestors, we add a IMPLEMENT_STANDARD_SUPERTYPE and 
// a IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY macro.
// We must respect the order: from the direct ancestor class
// to the base class.
//
IMPLEMENT_STANDARD_TYPE(OCC_Point)
        IMPLEMENT_STANDARD_SUPERTYPE(AIS_InteractiveObject)
        IMPLEMENT_STANDARD_SUPERTYPE(SelectMgr_SelectableObject)
        IMPLEMENT_STANDARD_SUPERTYPE(PrsMgr_PresentableObject)
        IMPLEMENT_STANDARD_SUPERTYPE(MMgt_TShared)
//        IMPLEMENT_STANDARD_SUPERTYPE(Standard_Transient)
        IMPLEMENT_STANDARD_SUPERTYPE_ARRAY()
                IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY(AIS_InteractiveObject)
                IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY(SelectMgr_SelectableObject)
                IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY(PrsMgr_PresentableObject)
                IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY(MMgt_TShared)
//                IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY(Standard_Transient)
        IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_END()
IMPLEMENT_STANDARD_TYPE_END(OCC_Point)


#include "Graphic2d_CircleMarker.hxx"
#include "StdPrs_Point.hxx"
#include "Geom_CartesianPoint.hxx"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
OCC_Point::OCC_Point(Standard_Real X,Standard_Real Y ,Standard_Real Z)
:myPoint(gp_Pnt(X,Y,Z))
{

}

OCC_Point::OCC_Point(gp_Pnt2d& aPoint,Standard_Real Elevation)
:myPoint(gp_Pnt(aPoint.X(),aPoint.Y(),Elevation))
{

}

OCC_Point::OCC_Point(gp_Pnt& aPoint)
:myPoint(aPoint)
{

}

OCC_Point::~OCC_Point()
{

}

void OCC_Point::Compute(const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                             const Handle(Prs3d_Presentation)& aPresentation,
                             const Standard_Integer aMode)
{
    Handle(Geom_CartesianPoint) aGeomPoint = new Geom_CartesianPoint(myPoint);

    StdPrs_Point::Add(aPresentation,aGeomPoint,myDrawer);
 }


void OCC_Point::Compute(const Handle(Prs3d_Projector)& aProjector,
                             const Handle(Prs3d_Presentation)& aPresentation) 
 {
 }

void OCC_Point::Compute(const Handle(PrsMgr_PresentationManager2d)& aPresentationManager,
                         const Handle(Graphic2d_GraphicObject)& aGrObj, 
                         const Standard_Integer unMode)
{
  Handle(Graphic2d_CircleMarker) aCircleMarker;
  aCircleMarker = new Graphic2d_CircleMarker(aGrObj,myPoint.X(),myPoint.Y(),0,0,1);
}

void OCC_Point::ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,
				      const Standard_Integer unMode)
{
}

