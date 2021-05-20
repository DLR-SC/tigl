/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2012-08-02 Martin Siggel <Martin.Siggel@dlr.de>
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
//////////////////////////////////////////////////////////////////////
//
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

#include <OSD_Environment.hxx>
#include <SelectMgr_Selection.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_Presentation.hxx>
#include <PrsMgr_PresentationManager3d.hxx>
#include <gp_Pnt.hxx>
#include <TCollection_ExtendedString.hxx>

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
                  const Standard_Real            anAngle,     // = 0.0
                  const Standard_Real            aslant,      // = 0.0
                  const Standard_Integer         aColorIndex, // = 0
                  const Standard_Integer         aFontIndex,  // = 1
                  const Standard_Real            aScale)      // = 1
                  :AIS_InteractiveObject(),MyText(aText),MyX(anX),MyY(anY),MyZ(aZ),
                  MyAngle(anAngle),MySlant(aslant),
                  MyColorIndex(aColorIndex), MyFontIndex(aFontIndex), MyScale(aScale)
{}

ISession_Text::ISession_Text
                 (const TCollection_AsciiString& aText, 
                  gp_Pnt&                        aPoint,
                  const Standard_Real            anAngle,     // = 0.0
                  const Standard_Real            aslant,      // = 0.0
                  const Standard_Integer         aColorIndex, // = 0
                  const Standard_Integer         aFontIndex,  // = 1
                  const Standard_Real            aScale)      // = 1
                  :AIS_InteractiveObject(),MyText(aText),MyX(aPoint.X()),MyY(aPoint.Y()),MyZ(aPoint.Z()),
                  MyAngle(anAngle),MySlant(aslant),
                  MyColorIndex(aColorIndex), MyFontIndex(aFontIndex), MyScale(aScale)
{}


ISession_Text::~ISession_Text()
{

}

void ISession_Text::Compute(const Handle(PrsMgr_PresentationManager3d)& mgr,
                            const Handle(Prs3d_Presentation)& aPresentation,
                            const Standard_Integer /*aMode*/)
{

#if OCC_VERSION_HEX >= 0x070200
    Handle(Prs3d_Drawer) greenStyle = new Prs3d_Drawer();
    greenStyle->SetColor(Quantity_NOC_GREEN);
    mgr->Color(this, greenStyle);
#elif OCC_VERSION_HEX >= 0x070100
    Handle(Graphic3d_HighlightStyle) greenStyle = new Graphic3d_HighlightStyle();
    greenStyle->SetColor(Quantity_NOC_GREEN);
    mgr->Color(this, greenStyle);
#else
    mgr->Color(this, Quantity_NOC_GREEN);
#endif

#if OCC_VERSION_HEX < 0x070300
    Prs3d_Text::Draw(aPresentation ,myDrawer, MyText, gp_Pnt(MyX, MyY, MyZ));
#else
    Prs3d_Text::Draw(Prs3d_Root::CurrentGroup(aPresentation), myDrawer->TextAspect(), MyText, gp_Pnt( MyX, MyY, MyZ));
#endif
}

void ISession_Text::ComputeSelection(const Handle(SelectMgr_Selection)& /*aSelection*/,
                                     const Standard_Integer /*unMode*/)
{
}

