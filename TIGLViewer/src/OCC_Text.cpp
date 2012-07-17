
#include "OCC_Text.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
IMPLEMENT_STANDARD_HANDLE(OCC_Text,AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTI(OCC_Text,AIS_InteractiveObject)
//
// Foreach ancestors, we add a IMPLEMENT_STANDARD_SUPERTYPE and 
// a IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_ENTRY macro.
// We must respect the order: from the direct ancestor class
// to the base class.
//
IMPLEMENT_STANDARD_TYPE(OCC_Text)
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
IMPLEMENT_STANDARD_TYPE_END(OCC_Text)


#include <Graphic2d_Text.hxx>
#include <Select2D_SensitiveBox.hxx>
#include <Graphic2d_Segment.hxx>
#include <OSD_Environment.hxx>
#include <Graphic2d_View.hxx>
#include <Graphic2d_Drawer.hxx>
#include "PrsMgr_PresentationManager2d.hxx"
#include "SelectMgr_Selection.hxx"
#include "Graphic2d_Array1OfVertex.hxx"
#include "Graphic2d_Polyline.hxx"
#include "Graphic2d_Vertex.hxx"
#include "Graphic2d_DisplayList.hxx"
#include "Prs3d_Text.hxx"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OCC_Text::OCC_Text()
{

}


OCC_Text::OCC_Text
                 (const TCollection_AsciiString& aText, 
                  const Standard_Real            anX ,        // = 0
                  const Standard_Real            anY ,        // = 0
                  const Standard_Real            aZ  ,        // = 0
			      const Aspect_TypeOfText        aType,       // = SOLID,
			      const Quantity_PlaneAngle      anAngle,     // = 0.0
			      const Standard_Real            aslant,      // = 0.0
			      const Standard_Integer         aColorIndex, // = 0
			      const Standard_Integer         aFontIndex,  // = 1
			      const Quantity_Factor          aScale)      // = 1
                  :AIS_InteractiveObject(),MyText(aText),MyX(anX),MyY(anY),MyZ(aZ),
                  MyTypeOfText(aType),MyAngle(anAngle),MySlant(aslant),MyFontIndex(aFontIndex),
                  MyColorIndex(aColorIndex),MyScale(aScale),MyWidth(0),MyHeight(0)
{}

OCC_Text::OCC_Text
                 (const TCollection_AsciiString& aText, 
                  gp_Pnt&                        aPoint,
			      const Aspect_TypeOfText        aType,       // = SOLID,
			      const Quantity_PlaneAngle      anAngle,     // = 0.0
			      const Standard_Real            aslant,      // = 0.0
			      const Standard_Integer         aColorIndex, // = 0
			      const Standard_Integer         aFontIndex,  // = 1
			      const Quantity_Factor          aScale)      // = 1
                  :AIS_InteractiveObject(),MyText(aText),MyX(aPoint.X()),MyY(aPoint.Y()),MyZ(aPoint.Z()),
                  MyTypeOfText(aType),MyAngle(anAngle),MySlant(aslant),MyFontIndex(aFontIndex),
                  MyColorIndex(aColorIndex),MyScale(aScale),MyWidth(0),MyHeight(0)
{}



OCC_Text::~OCC_Text()
{

}

void OCC_Text::Compute(const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                             const Handle(Prs3d_Presentation)& aPresentation,
                             const Standard_Integer aMode)
{
    Prs3d_Text::Draw(aPresentation,myDrawer,MyText,gp_Pnt(  MyX ,MyY,MyZ ));
}

void OCC_Text::Compute(const Handle(Prs3d_Projector)& aProjector,
                             const Handle(Prs3d_Presentation)& aPresentation) 
 {
 }

void OCC_Text::Compute(const Handle(PrsMgr_PresentationManager2d)& aPresentationManager,
                         const Handle(Graphic2d_GraphicObject)& aGrObj, 
                         const Standard_Integer unMode)
{
  Handle(Graphic2d_Text) text;
  text = new Graphic2d_Text(aGrObj, MyText, MyX, MyY, MyAngle,MyTypeOfText,MyScale);
  text->SetFontIndex(MyFontIndex);
 
  text->SetColorIndex(MyColorIndex);
 
  text->SetSlant(MySlant);
  text->SetUnderline(Standard_False);
  text->SetZoomable(Standard_True);
  aGrObj->Display();
  Quantity_Length anXoffset,anYoffset;
  text->TextSize(MyWidth, MyHeight,anXoffset,anYoffset);

}

void OCC_Text::ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,
				      const Standard_Integer unMode)
{
}

