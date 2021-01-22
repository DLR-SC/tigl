/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-09-05 Martin Siggel <Martin.Siggel@dlr.de>
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
// This file is taken from opencascade examples
// ISession_Direction.cpp: implementation of the ISession_Direction class.
//
//////////////////////////////////////////////////////////////////////

#include "tigl_internal.h"

#include "ISession_Direction.h"
#include <Prs3d_ArrowAspect.hxx>
#include <Prs3d_Arrow.hxx>
#include <DsgPrs_LengthPresentation.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_Text.hxx>
#include <Graphic3d_Group.hxx>
#include <Standard_Version.hxx>

#if OCC_VERSION_HEX < VERSION_HEX_CODE(6,9,0)
#include <AIS_Drawer.hxx>
#endif

IMPLEMENT_STANDARD_RTTIEXT(ISession_Direction,AIS_InteractiveObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ISession_Direction::ISession_Direction()
{
}

ISession_Direction::ISession_Direction (const gp_Pnt& aPnt,
                                        const gp_Dir& aDir,
                                        Standard_Real aLength,
                                        Standard_Real anArrowLength)
    : myPnt (aPnt),
      myDir (aDir),
      myLength (aLength),
      myArrowLength (anArrowLength)
{}

ISession_Direction::ISession_Direction (const gp_Pnt& aPnt,
                                        const gp_Vec& aVec,
                                        Standard_Real anArrowLength)
    : myPnt (aPnt),
      myDir (aVec),
      myArrowLength (anArrowLength)
{
    myLength = aVec.Magnitude();
}

ISession_Direction::ISession_Direction (const gp_Pnt2d& aPnt2d,
                                        const gp_Dir2d& aDir2d,
                                        Standard_Real aLength)
    : myPnt (gp_Pnt(aPnt2d.X(),aPnt2d.Y(),0.0)),
      myDir (gp_Dir(aDir2d.X(),aDir2d.Y(),0.0)),
      myLength (aLength)
{
    myArrowLength = myDrawer->ArrowAspect()->Length();
}

ISession_Direction::ISession_Direction (const gp_Pnt2d& aPnt2d,
                                        const gp_Vec2d& aVec2d)
    : myPnt (gp_Pnt (aPnt2d.X(), aPnt2d.Y(), 0.0)),
      myDir (gp_Dir(aVec2d.X(), aVec2d.Y(), 0.0))
{
    myLength = aVec2d.Magnitude();
    myArrowLength = myDrawer->ArrowAspect()->Length();
}


ISession_Direction::~ISession_Direction()
{
}

void ISession_Direction::Compute (const Handle(PrsMgr_PresentationManager3d)& /*aPresentationManager*/,
                                  const Handle(Prs3d_Presentation)& aPresentation,
                                  const Standard_Integer /*aMode*/)
{
    // Set style for arrow
    Handle(Prs3d_ArrowAspect) anArrowAspect = myDrawer->ArrowAspect();
    anArrowAspect->SetLength (myArrowLength);

    gp_Pnt aLastPoint = myPnt;
    aLastPoint.Translate (myLength*gp_Vec(myDir));

    // Draw Line
    Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments (2);
    aPrims->AddVertex (myPnt);
    aPrims->AddVertex (aLastPoint);
    Prs3d_Root::CurrentGroup(aPresentation)->SetPrimitivesAspect (myDrawer->LineAspect()->Aspect());
    Prs3d_Root::CurrentGroup(aPresentation)->AddPrimitiveArray (aPrims);
    // Draw arrow
#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
    Prs3d_Arrow::Draw (aPresentation,
                       aLastPoint,
                       myDir,
                       anArrowAspect->Angle(),
                       anArrowAspect->Length());
#else
    Prs3d_Arrow::Draw (aPresentation->CurrentGroup(),
                       aLastPoint,
                       myDir,
                       anArrowAspect->Angle(),
                       anArrowAspect->Length());
#endif

    // Draw text
    if (myText.Length() != 0) {
        gp_Pnt aTextPosition = aLastPoint;
#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
        Prs3d_Text::Draw (aPresentation,
                          myDrawer->TextAspect(),
                          myText,
                          aTextPosition);
#else
        Prs3d_Text::Draw (aPresentation->CurrentGroup(),
                          myDrawer->TextAspect(),
                          myText,
                          aTextPosition);
#endif
    }
}


void ISession_Direction::ComputeSelection (const Handle(SelectMgr_Selection)& /*aSelection*/,
                                           const Standard_Integer /*aMode*/)
{
}

void ISession_Direction::SetText (TCollection_ExtendedString& theText)
{
    myText = theText;
}

void ISession_Direction::SetText (Standard_CString theText)
{
    myText = theText;
}

void ISession_Direction::SetLineAspect (const Handle(Prs3d_LineAspect)& theAspect)
{
    myDrawer->SetLineAspect (theAspect);
}
