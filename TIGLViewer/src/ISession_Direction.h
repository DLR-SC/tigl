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
// ISession_Direction.h: interface for the ISession_Direction class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_ISession_Direction_H__767C0DB3_A3B0_11D1_8DA3_0800369C8A03__INCLUDED_
#define AFX_ISession_Direction_H__767C0DB3_A3B0_11D1_8DA3_0800369C8A03__INCLUDED_

#include "gp_Dir2d.hxx"

#include <AIS_InteractiveObject.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Prs3d_LineAspect.hxx>
#include "occt_compat.h"

DEFINE_STANDARD_HANDLE(ISession_Direction,AIS_InteractiveObject)
class ISession_Direction : public AIS_InteractiveObject
{
public:
    TCollection_ExtendedString myText;
    void SetText (TCollection_ExtendedString& aText);
    void SetText (Standard_CString aText);
    void SetLineAspect (const Handle(Prs3d_LineAspect)& theAspect);
    ISession_Direction();
    ISession_Direction (const gp_Pnt& aPnt,const gp_Dir& aDir,Standard_Real aLength=1,Standard_Real anArrowLength=1);
    ISession_Direction (const gp_Pnt& aPnt,const gp_Vec& aVec,Standard_Real anArrowLength=1);

    ISession_Direction (const gp_Pnt2d& aPnt2d,const gp_Dir2d& aDir2d,Standard_Real aLength=1);
    ISession_Direction (const gp_Pnt2d& aPnt2d,const gp_Vec2d& aVec2d);

    ~ISession_Direction() override;
    DEFINE_STANDARD_RTTIEXT(ISession_Direction,AIS_InteractiveObject)

private:
    void Compute         (const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                          const Handle(Prs3d_Presentation)& aPresentation,
                          const Standard_Integer aMode) override;
    // void Compute         (const Handle(Prs3d_Projector)& aProjector,
    //                       const Handle(Prs3d_Presentation)& aPresentation) override;
    void ComputeSelection(const Handle(SelectMgr_Selection)& aSelection,const Standard_Integer aMode) override;

    gp_Pnt myPnt;
    gp_Dir myDir;
    Standard_Real myLength;
    Standard_Real myArrowLength;
};

#endif // AFX_ISession_Direction_H__767C0DB3_A3B0_11D1_8DA3_0800369C8A03__INCLUDED_
