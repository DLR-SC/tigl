/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerContext.cpp 203 2012-09-25 08:47:55Z martinsiggel $
*
* Version: $Revision: 203 $
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

#include <qnamespace.h>
#include "TIGLViewerContext.h"
#include "TIGLViewerInternal.h"

#ifdef WNT
#include <Graphic3d_WNTGraphicDevice.hxx>
#else
#ifndef __APPLE__
#include <Xw_GraphicDevice.hxx>
#endif
#include <Graphic3d_GraphicDevice.hxx>
#endif

TIGLViewerContext::TIGLViewerContext()
{
	// Create the OCC Viewers
	TCollection_ExtendedString a3DName("Visual3D");
	myViewer = createViewer( "DISPLAY", a3DName.ToExtString(), "", 1000.0 );
	myViewer->Init();
	myViewer->SetZBufferManagment(Standard_False);
	myViewer->SetDefaultViewProj( V3d_Zpos );	// Top view
 	myContext = new AIS_InteractiveContext( myViewer );

	myGridType       = Aspect_GT_Rectangular;
	myGridMode       = Aspect_GDM_Lines;
	myGridColor      = Quantity_NOC_RED4;
	myGridTenthColor = Quantity_NOC_GRAY90;

	myContext->SetHilightColor(Quantity_NOC_WHITE) ;

	setGridOffset (0.0);
	gridXY();
	gridOn();
}


TIGLViewerContext::~TIGLViewerContext()
{

}

Handle_V3d_Viewer& TIGLViewerContext::getViewer()
{ 
	return myViewer; 
}

Handle_AIS_InteractiveContext& TIGLViewerContext::getContext()
{ 
	return myContext; 
}

Handle_V3d_Viewer TIGLViewerContext::createViewer(	const Standard_CString aDisplay,
													const Standard_ExtString aName,
													const Standard_CString aDomain,
													const Standard_Real ViewSize )
{
#ifndef WNT
    static Handle(Graphic3d_GraphicDevice) defaultdevice;
	
    if( defaultdevice.IsNull() )
	{
		defaultdevice = new Graphic3d_GraphicDevice( getenv(aDisplay) );
	}

    return new V3d_Viewer(	defaultdevice,
							aName,
							aDomain,
							ViewSize,
							V3d_XposYnegZpos,
							Quantity_NOC_BLACK,
                    		V3d_ZBUFFER,
							V3d_GOURAUD,
							V3d_WAIT );
#else
    static Handle( Graphic3d_WNTGraphicDevice ) defaultdevice;
    if( defaultdevice.IsNull() )
	{
        defaultdevice = new Graphic3d_WNTGraphicDevice();
	}

    return new V3d_Viewer(	defaultdevice,
							aName,
							aDomain,
							ViewSize,
							V3d_XposYnegZpos,
							Quantity_NOC_BLACK,
                    		V3d_ZBUFFER,
							V3d_GOURAUD,
							V3d_WAIT );
#endif  // WNT
}
/*! 
\brief	Deletes all objects.

		This function deletes all dispayed objects from the AIS context.
		No parameters.
*/
void TIGLViewerContext::deleteAllObjects()
{
	AIS_ListOfInteractive aList;
	myContext->DisplayedObjects( aList );
	AIS_ListIteratorOfListOfInteractive aListIterator;
	for ( aListIterator.Initialize( aList ); aListIterator.More(); aListIterator.Next() )
	{
		myContext->Remove( aListIterator.Value(), Standard_False);
	}
}
/*! 
\brief	Sets the privileged plane to the XY Axis.  
*/
void TIGLViewerContext::gridXY  ( void )
{
	myGridColor      = Quantity_NOC_RED4;
	myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
	gp_Ax3 aPlane(gp_Pnt( 0., 0., 0. ),gp_Dir(0., 0., 1.));
	myViewer->SetPrivilegedPlane( aPlane );
}
/*!
\brief	Sets the privileged plane to the XZ Axis.

		Note the negative direction of the Y axis.
		This is corrrect for a right-handed co-ordinate set.
*/
void TIGLViewerContext::gridXZ  ( void )
{
	myGridColor      = Quantity_NOC_BLUE4;
	myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
	gp_Ax3 aPlane( gp_Pnt(0., 0., 0.),gp_Dir(0., -1., 0.) );
	myViewer->SetPrivilegedPlane( aPlane );
}
/*! 
\brief	Sets the privileged plane to the XY Axis.
 */
void TIGLViewerContext::gridYZ  ( void )
{
	myGridColor      = Quantity_NOC_GREEN4;
	myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
	gp_Ax3 aPlane( gp_Pnt( 0., 0., 0.), gp_Dir( 1., 0., 0. ) );
	myViewer->SetPrivilegedPlane( aPlane );
}

/*!
 * \brief Toggles the grid on and off
 */
void TIGLViewerContext::toggleGrid(bool gridIsOn){
    if(gridIsOn)
        gridOn();
    else
        gridOff();
}

/*!
\brief	Turn the grid on.
 */
void TIGLViewerContext::gridOn  ( void )
{
	myViewer->ActivateGrid( myGridType , myGridMode );
	myViewer->SetGridEcho ( Standard_True );
}

/*! 
\brief	Turn the grid off.
*/
void TIGLViewerContext::gridOff ( void )
{
	myViewer->DeactivateGrid();
	myViewer->SetGridEcho( Standard_False );
}

/*!
\brief	Select a rectangular grid
*/
void TIGLViewerContext::gridRect ( void )
{
	myGridType = Aspect_GT_Rectangular;
	myViewer->ActivateGrid( myGridType , myGridMode );
	myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
}
/*! 
\brief	Select a circular grid.
*/
void TIGLViewerContext::gridCirc ( void )
{
	myGridType = Aspect_GT_Circular;
	myViewer->ActivateGrid( myGridType , myGridMode );
	myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
}

/*!
 * \brief Toggles wireframe view.
 * \param wireframe True, if wireframes should be printed
 */
void TIGLViewerContext::wireFrame(bool wireframe){
    if(!myContext.IsNull()){
        if(wireframe)
            myContext->SetDisplayMode(AIS_WireFrame);
        else
            myContext->SetDisplayMode(AIS_Shaded);
    }
}

void TIGLViewerContext::setGridOffset (Quantity_Length offset)
{
	Quantity_Length radius;
	Quantity_Length xSize, ySize;
	Quantity_Length oldOffset;
	
	myViewer->CircularGridGraphicValues( radius, oldOffset );
	myViewer->SetCircularGridGraphicValues( radius, offset);

	myViewer->RectangularGridGraphicValues(xSize, ySize, oldOffset);
	myViewer->SetRectangularGridGraphicValues(xSize, ySize, offset);
}


