// ISession_Text.cpp: implementation of the ISession_Text class.
//
//////////////////////////////////////////////////////////////////////

#include "ISession_Text.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
IMPLEMENT_STANDARD_HANDLE(ISession_Text,AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTIEXT(ISession_Text,AIS_InteractiveObject)

#if 0
#include <Graphic2d_Text.hxx>
#include <Select2D_SensitiveBox.hxx>
#include <Graphic2d_Segment.hxx>
#include <Graphic2d_View.hxx>
#include <Graphic2d_Drawer.hxx>
#include "PrsMgr_PresentationManager2d.hxx"
#include "Graphic2d_Array1OfVertex.hxx"
#include "Graphic2d_Polyline.hxx"
#include "Graphic2d_Vertex.hxx"
#include "Graphic2d_DisplayList.hxx"
#endif

#include <OSD_Environment.hxx>
#include <SelectMgr_Selection.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_Presentation.hxx>
#include <gp_Pnt.hxx>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ISession_Text::ISession_Text()
{

}


ISession_Text::ISession_Text
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
                  MyTypeOfText(aType),MyAngle(anAngle),MySlant(aslant),MyColorIndex(aColorIndex),
                  MyFontIndex(aFontIndex),MyScale(aScale),MyWidth(0),MyHeight(0)
{}

ISession_Text::ISession_Text
                 (const TCollection_AsciiString& aText, 
                  gp_Pnt&                        aPoint,
			      const Aspect_TypeOfText        aType,       // = SOLID,
			      const Quantity_PlaneAngle      anAngle,     // = 0.0
			      const Standard_Real            aslant,      // = 0.0
			      const Standard_Integer         aColorIndex, // = 0
			      const Standard_Integer         aFontIndex,  // = 1
			      const Quantity_Factor          aScale)      // = 1
                  :AIS_InteractiveObject(),MyText(aText),MyX(aPoint.X()),MyY(aPoint.Y()),MyZ(aPoint.Z()),
                  MyTypeOfText(aType),MyAngle(anAngle),MySlant(aslant),MyColorIndex(aColorIndex),
                  MyFontIndex(aFontIndex),MyScale(aScale),MyWidth(0),MyHeight(0)
{}



ISession_Text::~ISession_Text()
{

}

void ISession_Text::Compute(const Handle(PrsMgr_PresentationManager3d)&,
                             const Handle(Prs3d_Presentation)& aPresentation,
                             const Standard_Integer /*aMode*/)
{
    aPresentation->Color(Quantity_NOC_GREEN);
    Prs3d_Text::Draw(aPresentation,myDrawer,MyText,gp_Pnt(  MyX ,MyY,MyZ ));
}

void ISession_Text::Compute(const Handle(Prs3d_Projector)& ,
                             const Handle(Prs3d_Presentation)& )
 {
 }

#if 0
void ISession_Text::Compute(const Handle(PrsMgr_PresentationManager2d)& ,
                         const Handle(Graphic2d_GraphicObject)& aGrObj, 
                         const Standard_Integer /* unmode*/)
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
#endif

void ISession_Text::ComputeSelection(const Handle(SelectMgr_Selection)& /*aSelection*/,
                      const Standard_Integer /*unMode*/)
{
}

