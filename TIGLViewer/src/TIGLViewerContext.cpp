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
#include "tigl_config.h"

#include <qnamespace.h>
#include "TIGLViewerContext.h"
#include "TIGLViewerInternal.h"
#include "TIGLViewerSettings.h"
#include "tiglcommonfunctions.h"

#include "ISession_Point.h"
#include "ISession_Text.h"
#include "ISession_Direction.h"

#include <OpenGl_GraphicDriver.hxx>

TIGLViewerContext::TIGLViewerContext()
{
    // Create the OCC Viewers
    TCollection_ExtendedString a3DName("Visual3D");
    myViewer = createViewer( a3DName.ToExtString(), "", 1000.0 );
    myViewer->SetDefaultLights();
    myViewer->SetDefaultViewProj( V3d_Zpos );    // Top view
    myContext = new AIS_InteractiveContext( myViewer );

    myGridType       = Aspect_GT_Rectangular;
    myGridMode       = Aspect_GDM_Lines;
    myGridColor      = Quantity_NOC_RED4;
    myGridTenthColor = Quantity_NOC_GRAY90;

    myContext->SetHilightColor(Quantity_NOC_WHITE);
    myContext->SetIsoNumber(0);

    setGridOffset (0.0);
    gridXY();
    gridOn();
}


TIGLViewerContext::~TIGLViewerContext()
{

}

Handle(V3d_Viewer)& TIGLViewerContext::getViewer()
{ 
    return myViewer; 
}

Handle(AIS_InteractiveContext)& TIGLViewerContext::getContext()
{ 
    return myContext; 
}

Handle(V3d_Viewer) TIGLViewerContext::createViewer( const Standard_ExtString aName,
                                                    const Standard_CString aDomain,
                                                    const Standard_Real ViewSize )
{
    static Handle(OpenGl_GraphicDriver) deviceHandle;

    if (deviceHandle.IsNull()) {
      Handle(Aspect_DisplayConnection) aDisplayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
      aDisplayConnection = new Aspect_DisplayConnection (qgetenv ("DISPLAY").constData());
#endif
      deviceHandle = new OpenGl_GraphicDriver (aDisplayConnection);
    }

    return new V3d_Viewer(  deviceHandle,
                            aName,
                            aDomain,
                            ViewSize,
                            V3d_XposYnegZpos,
                            Quantity_NOC_BLACK,
                            V3d_ZBUFFER,
                            V3d_GOURAUD,
                            V3d_WAIT );
}
/*! 
\brief    Deletes all objects.

        This function deletes all dispayed objects from the AIS context.
        No parameters.
*/
void TIGLViewerContext::deleteAllObjects()
{
    AIS_ListOfInteractive aList;
    myContext->DisplayedObjects( aList );
    AIS_ListIteratorOfListOfInteractive aListIterator;
    for ( aListIterator.Initialize( aList ); aListIterator.More(); aListIterator.Next() ) {
        myContext->Remove( aListIterator.Value(), Standard_False);
    }
    myContext->UpdateCurrentViewer();
}
/*! 
\brief    Sets the privileged plane to the XY Axis.  
*/
void TIGLViewerContext::gridXY  ( void )
{
    myGridColor      = Quantity_NOC_RED4;
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
    gp_Ax3 aPlane(gp_Pnt( 0., 0., 0. ),gp_Dir(0., 0., 1.));
    myViewer->SetPrivilegedPlane( aPlane );
}
/*!
\brief    Sets the privileged plane to the XZ Axis.

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
\brief    Sets the privileged plane to the XY Axis.
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
void TIGLViewerContext::toggleGrid(bool gridIsOn) 
{
    if (gridIsOn) {
        gridOn();
    }
    else {
        gridOff();
    }
}

/*!
\brief    Turn the grid on.
 */
void TIGLViewerContext::gridOn  ( void )
{
    myViewer->ActivateGrid( myGridType , myGridMode );
    myViewer->SetGridEcho ( Standard_True );
}

/*! 
\brief    Turn the grid off.
*/
void TIGLViewerContext::gridOff ( void )
{
    myViewer->DeactivateGrid();
    myViewer->SetGridEcho( Standard_False );
}

/*!
\brief    Select a rectangular grid
*/
void TIGLViewerContext::gridRect ( void )
{
    myGridType = Aspect_GT_Rectangular;
    myViewer->ActivateGrid( myGridType , myGridMode );
    myViewer->Grid()->SetColors( myGridColor, myGridTenthColor );
}
/*! 
\brief    Select a circular grid.
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
void TIGLViewerContext::wireFrame(bool wireframe) 
{
    if (!myContext.IsNull()){
        if (wireframe) {
            myContext->SetDisplayMode(AIS_WireFrame);
        }
        else {
            myContext->SetDisplayMode(AIS_Shaded);
        }
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

// a small helper when we just want to display a shape
void TIGLViewerContext::displayShape(const TopoDS_Shape& loft, Quantity_Color color, double transparency)
{
    TIGLViewerSettings& settings = TIGLViewerSettings::Instance();
    Handle(AIS_Shape) shape = new AIS_Shape(loft);
    myContext->SetMaterial(shape, Graphic3d_NOM_METALIZED, Standard_False);
    myContext->SetColor(shape, color, Standard_False);
    myContext->SetTransparency(shape, transparency, Standard_False);
    shape->SetOwnDeviationCoefficient(settings.tesselationAccuracy());
    myContext->Display(shape, Standard_True);
    
    if (settings.enumerateFaces()) {
        TopTools_IndexedMapOfShape shapeMap;
        TopExp::MapShapes(loft, TopAbs_FACE, shapeMap);
        for (int i = 1; i <= shapeMap.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(shapeMap(i));
            gp_Pnt p = GetCentralFacePoint(face);
            QString s = QString("%1").arg(i);
            displayPoint(p, s.toStdString().c_str(), false, 0., 0., 0., 10.);
        }
    }
}

// Displays a point on the screen
void TIGLViewerContext::displayPoint(const gp_Pnt& aPoint,
                                     const char* aText,
                                     Standard_Boolean UpdateViewer,
                                     Standard_Real anXoffset,
                                     Standard_Real anYoffset,
                                     Standard_Real aZoffset,
                                     Standard_Real TextScale)
{
    Handle(ISession_Point) aGraphicPoint = new ISession_Point(aPoint.X(), aPoint.Y(), aPoint.Z());
    myContext->Display(aGraphicPoint,UpdateViewer);
    Handle(ISession_Text) aGraphicText = new ISession_Text(aText, aPoint.X() + anXoffset,
                                                 aPoint.Y() + anYoffset,
                                                 aPoint.Z() + aZoffset);
    aGraphicText->SetScale(TextScale);
    myContext->Display(aGraphicText,UpdateViewer);
}

// convenience wrapper
void TIGLViewerContext::drawPoint(double x, double y, double z)
{
    displayPoint(gp_Pnt(x,y,z), "", Standard_True, 0,0,0, 1.0);
}

// Displays a vector on the screen
void TIGLViewerContext::displayVector(const gp_Pnt& aPoint,
                                      const gp_Vec& aVec,
                                      const char* aText,
                                      Standard_Boolean UpdateViewer,
                                      Standard_Real anXoffset,
                                      Standard_Real anYoffset,
                                      Standard_Real aZoffset,
                                      Standard_Real TextScale)
{
    Handle(ISession_Direction) aGraphicDirection = new ISession_Direction(aPoint, aVec);
    myContext->Display(aGraphicDirection,UpdateViewer);
    Handle(ISession_Text) aGraphicText = new ISession_Text(aText, aPoint.X() + anXoffset,
                                                 aPoint.Y() + anYoffset,
                                                 aPoint.Z() + aZoffset);
    aGraphicText->SetScale(TextScale);
    myContext->Display(aGraphicText,UpdateViewer);
}

// convenience wrapper
void TIGLViewerContext::drawVector(double x, double y, double z, double dirx, double diry, double dirz)
{
    displayVector(gp_Pnt(x,y,z), gp_Vec(dirx, diry, dirz), "", Standard_True, 0,0,0, 1.0);
}


