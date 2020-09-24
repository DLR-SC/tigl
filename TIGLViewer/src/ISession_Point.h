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
// ISession_Point.h: interface for the ISession_Point class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_ISESSION_POINT_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_
#define AFX_ISESSION_POINT_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_


#include <Standard_Macro.hxx>
#include <Standard_DefineHandle.hxx>


#include <AIS_InteractiveObject.hxx>
#include <gp_Pnt.hxx>
#include <occt_compat.h>
#include "tigl_internal.h"

DEFINE_STANDARD_HANDLE(ISession_Point,AIS_InteractiveObject)
class ISession_Point : public AIS_InteractiveObject  
{
public:
    ISession_Point();
    ISession_Point(Standard_Real X,Standard_Real Y ,Standard_Real Z);
    ISession_Point(gp_Pnt2d& aPoint,Standard_Real Elevation = 0);
    ISession_Point(gp_Pnt& aPoint);

    ~ISession_Point() override;
    DEFINE_STANDARD_RTTIEXT(ISession_Point,AIS_InteractiveObject)

private :

     void Compute          (const Handle(PrsMgr_PresentationManager3d)& aPresentationManager,
                            const Handle(Prs3d_Presentation)& aPresentation,
                            const Standard_Integer aMode) override;
    //  void Compute          (const Handle(Prs3d_Projector)& aProjector,
    //                         const Handle(Prs3d_Presentation)& aPresentation) override;
     #if 0
     void Compute          (const Handle(PrsMgr_PresentationManager2d)& aPresentationManager,
                            const Handle(Graphic2d_GraphicObject)& aGrObj,
                            const Standard_Integer unMode = 0) ;
     #endif
     void ComputeSelection (const Handle(SelectMgr_Selection)& aSelection,
                            const Standard_Integer unMode) override;
     
     gp_Pnt myPoint;

};

#endif // !defined(AFX_ISESSION_POINT_H__A9B277C3_A69E_11D1_8DA4_0800369C8A03__INCLUDED_)
