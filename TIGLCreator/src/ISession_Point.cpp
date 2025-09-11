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
// ISession_Point.cpp: implementation of the ISession_Point class.
//
//////////////////////////////////////////////////////////////////////

#include "ISession_Point.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif
IMPLEMENT_STANDARD_HANDLE(ISession_Point,AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTIEXT(ISession_Point,AIS_InteractiveObject)

#if 0
#include "Graphic2d_CircleMarker.hxx"
#endif
#include "StdPrs_Point.hxx"
#include "Geom_CartesianPoint.hxx"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ISession_Point::ISession_Point(Standard_Real X,Standard_Real Y ,Standard_Real Z)
:myPoint(gp_Pnt(X,Y,Z))
{
}

ISession_Point::ISession_Point(gp_Pnt2d& aPoint,Standard_Real Elevation)
:myPoint(gp_Pnt(aPoint.X(),aPoint.Y(),Elevation))
{
}

ISession_Point::ISession_Point(gp_Pnt& aPoint)
:myPoint(aPoint)
{
}

ISession_Point::~ISession_Point()
{
}

void ISession_Point::Compute(const Handle(PrsMgr_PresentationManager3d)& /*aPresentationManager*/,
                             const Handle(Prs3d_Presentation)& aPresentation,
                             const Standard_Integer /*aMode*/)
{
    Handle(Geom_CartesianPoint) aGeomPoint = new Geom_CartesianPoint(myPoint);

    StdPrs_Point::Add(aPresentation,aGeomPoint,myDrawer);
}


#if 0
void ISession_Point::Compute(const Handle(PrsMgr_PresentationManager2d)& /*aPresentationManager*/,
                         const Handle(Graphic2d_GraphicObject)& aGrObj, 
                         const Standard_Integer /*unMode*/)
{
    Handle(Graphic2d_CircleMarker) aCircleMarker;
    aCircleMarker = new Graphic2d_CircleMarker(aGrObj,myPoint.X(),myPoint.Y(),0,0,1);
}
#endif

void ISession_Point::ComputeSelection(const Handle(SelectMgr_Selection)& /*aSelection*/,
                                      const Standard_Integer /*unMode*/)
{
}

