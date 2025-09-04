/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLCreatorWidget.cpp 226 2012-10-23 19:18:29Z martinsiggel $
*
* Version: $Revision: 226 $
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

#include "TIGLCreatorWidget.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <QApplication>
#include <QBitmap>
#include <QPainter>
#include <QInputEvent>
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QAction>
#include <QMenu>

#include "TIGLCreatorInternal.h"
#include "TIGLQAspectWindow.h"
#include "TIGLCreatorContext.h"
#include "TIGLCreatorSettings.h"
#include "TIGLCreatorMaterials.h"
#include "ISession_Point.h"
#include "ISession_Direction.h"
#include "ISession_Text.h"
#include "CTiglLogging.h"
#include "tiglcommonfunctions.h"
#include "TIGLSliderDialog.h"

#include <OpenGl_GraphicDriver.hxx>

#if defined _WIN32 || defined __WIN32__
  #include <WNT_Window.hxx>
  #include <gl/GL.h>
  #include <gl/GLU.h>
#elif defined __APPLE__
  #include <Cocoa_Window.hxx>
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <Xw_Window.hxx>
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif


#if OCC_VERSION_HEX < 0x070000
#include "Visual3d_Layer.hxx"
#else
#include "AIS_RubberBand.hxx"
#endif

#include "V3d_DirectionalLight.hxx"
#include "V3d_AmbientLight.hxx"

// 10% zoom per wheel or key event
#define TIGLCREATOR_ZOOM_STEP 1.10

TIGLCreatorWidget::TIGLCreatorWidget(QWidget * parent)
    : QWidget(parent),
    myView            ( NULL ),
    myViewer          ( NULL ),
#if OCC_VERSION_HEX < 0x070000
    myLayer           ( NULL ),
#else
    whiteRect         ( NULL ),
    blackRect         ( NULL ),
#endif
    myViewResized     ( Standard_False ),
    myViewInitialized ( Standard_False ),
    myMode            ( CurAction3d_Undefined ),
    myGridSnap        ( Standard_False ),
    myDetection       ( AIS_SOD_Nothing ),
    myPrecision       ( 0.001 ),
    myViewPrecision   ( 0.0 ),
    myKeyboardFlags   ( Qt::NoModifier ),
    myButtonFlags     ( Qt::NoButton ),
    viewerContext     (nullptr)
{
    initialize();
}


void TIGLCreatorWidget::initialize()
{
    myView            = nullptr;
    myViewer          = nullptr;
    
#if OCC_VERSION_HEX < 0x070000
    myLayer           = nullptr;
#else
    whiteRect = new AIS_RubberBand (Quantity_Color(Quantity_NOC_WHITE), Aspect_TOL_DOT, 1.0);
    blackRect = new AIS_RubberBand (Quantity_Color(Quantity_NOC_BLACK), Aspect_TOL_DOT, 1.0);
#endif
    myMode              = CurAction3d_Undefined;
    myGridSnap        = Standard_False;
    myViewResized      = Standard_False;
    myViewInitialized = Standard_False;
    myPrecision          = 0.001;
    myViewPrecision   = 0.0;
    myDetection          = AIS_SOD_Nothing;
    myKeyboardFlags   = Qt::NoModifier;
    myButtonFlags      = Qt::NoButton;
    myBGColor = QColor(255,235,163);

    // Needed to generate mouse events
    setMouseTracking( true );

    // Avoid Qt background clears to improve resizing speed,
    // along with a couple of other attributes
    setAutoFillBackground( false );
    setAttribute( Qt::WA_NoSystemBackground );

    // This next attribute seems to be the secret of allowing OCC on Win32
    // to "own" the window, even though its only supposed to work on X11.
    setAttribute( Qt::WA_PaintOnScreen );

    // Here's a modified pick point cursor from AutoQ3D
    QBitmap curb1( 48, 48 );
    QBitmap curb2( 48, 48 );
    curb1.fill( QColor( 255, 255, 255 ) );
    curb2.fill( QColor( 255, 255, 255 ) );
    QPainter p;

    p.begin( &curb1 );
        p.drawLine( 24,  0, 24, 47 );
        p.drawLine(  0, 24, 47, 24 );
        p.setBrush( Qt::NoBrush );
        p.drawRect( 18, 18, 12, 12 );
    p.end();
    myCrossCursor = QCursor( curb2, curb1, 24, 24 );

    // create win id, this is required on qt 5
    // without winid, the view can't be properly initialized
    winId();
}


void TIGLCreatorWidget::setContext(TIGLCreatorContext* aContext)
{
    viewerContext = aContext;
}


void TIGLCreatorWidget::initializeOCC(const Handle(AIS_InteractiveContext)& aContext)
{
    if (myView.IsNull()) {
        myViewer  = aContext->CurrentViewer();
        myView    = myViewer->CreateView();

        Handle_Aspect_Window myWindow;
        
#if OCC_VERSION_HEX >= 0x060800
        myWindow = new TIGLQAspectWindow(this);
#else
  #if defined _WIN32 || defined __WIN32__
        myWindow = new WNT_Window((Aspect_Handle)winId());
  #elif defined __APPLE__
        myWindow = new Cocoa_Window((NSView *)winId());
  #else
        Aspect_Handle windowHandle = (Aspect_Handle)winId();
        myWindow = new Xw_Window(myContext->CurrentViewer()->Driver()->GetDisplayConnection(),
                                 windowHandle);
  #endif
#endif // OCC_VERSION_HEX >= 0x060800

        // Set my window (Hwnd) into the OCC view
        myView->SetWindow( myWindow );
        // Set up axes (Trihedron) in lower left corner.
        myView->SetScale( 2 );            // Choose a "nicer" initial scale

        // Set up axes (Trihedron) in lower left corner.
        myView->TriedronDisplay( Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.1, V3d_ZBUFFER );
        //myView->SetAntialiasingOn();

        //myView->ColorScaleDisplay();

#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,1,0)
        // enable textures
        myView->SetSurfaceDetail(V3d_TEX_ALL);
#endif

        // Map the window
        if (!myWindow->IsMapped()) {
            myWindow->Map();
        }
        // Force a redraw to the new window on next paint event
        myViewResized = Standard_True;
        // Set default cursor as a cross
        setMode( CurAction3d_Nothing );

        myViewInitialized = Standard_True;

#if OCC_VERSION_HEX < 0x070000
        myLayer   = new Visual3d_Layer (myViewer->Viewer(), Aspect_TOL_OVERLAY, Standard_True /*aSizeDependant*/);
#endif

        setBackgroundGradient(myBGColor.red(), myBGColor.green(), myBGColor.blue());
        
        // This is to signal any connected slots that the view is ready.
        emit initialized();
    }
}


QPaintEngine* TIGLCreatorWidget::paintEngine() const
{
    return nullptr;
}


void TIGLCreatorWidget::paintEvent ( QPaintEvent * /* e */)
{
    if ( !myViewInitialized ) {
        if ( winId() ) {
            initializeOCC( viewerContext->getContext() );
        }
    }
    if ( !myViewer.IsNull() ) {
        redraw( true );
    }
}



void TIGLCreatorWidget::resizeEvent ( QResizeEvent * /* e */ )
{
    myViewResized = Standard_True;
}    


void TIGLCreatorWidget::mousePressEvent( QMouseEvent* e )
{
    myButtonFlags = e->button();

    // Cache the keyboard flags for the whole gesture
    myKeyboardFlags = e->modifiers();

    // The button mappings can be used as a mask. This code prevents conflicts
    // when more than one button pressed simultaneously.
    if ( e->button() & Qt::LeftButton ) {
        onLeftButtonDown  ( myKeyboardFlags, e->pos() );
    }
    else if ( e->button() & Qt::RightButton ) {
        onRightButtonDown ( myKeyboardFlags, e->pos() );
    }
    else if ( e->button() & Qt::MidButton ) {
        onMiddleButtonDown( myKeyboardFlags, e->pos() );
    }
}



void TIGLCreatorWidget::mouseReleaseEvent(QMouseEvent* e)
{
    myButtonFlags = Qt::NoButton;
    redraw();                            // Clears up screen when menu selected but not used.
    hideRubberBand();
    if ( e->button() & Qt::LeftButton ) {
        onLeftButtonUp  ( myKeyboardFlags, e->pos() );
    }
    else if ( e->button() & Qt::RightButton ) {
        onRightButtonUp ( myKeyboardFlags, e->pos() );
    }
    else if ( e->button() & Qt::MidButton ) {
        onMiddleButtonUp( myKeyboardFlags, e->pos() );
    }
}



void TIGLCreatorWidget::mouseMoveEvent(QMouseEvent* e)
{
    Standard_Real X, Y, Z;
    
    setCurrentPoint(e->pos());
    Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();
    //Check if the grid is active and that we're snapping to it
    if ( myContext->CurrentViewer()->Grid()->IsActive() && myGridSnap ) {
        myView->ConvertToGrid( myCurrentPoint.x(),
                               myCurrentPoint.y(),
                               myV3dX,
                               myV3dY,
                               myV3dZ );
        emit mouseMoved( myV3dX, myV3dY, myV3dZ );
    }
    else {
        bool success = convertToPlane( myCurrentPoint.x(),
                                       myCurrentPoint.y(),
                                       X, Y, Z );
        //    this is the standard case
        if (success) {

            myV3dX = precision( X );
            myV3dY = precision( Y );
            myV3dZ = precision( Z );
            emit mouseMoved( myV3dX, myV3dY, myV3dZ );
        }
        else {
            emit sendStatus ( tr("Indeterminate Point") );
        }
    }

    onMouseMove( e->buttons(), myKeyboardFlags, e->pos() );
}



void TIGLCreatorWidget::leaveEvent ( QEvent* /* e */ )
{
    myButtonFlags = Qt::NoButton;
}


void TIGLCreatorWidget::wheelEvent ( QWheelEvent* e )
{
    if ( !myView.IsNull() ) {
        Standard_Real currentScale = myView->Scale();
        if (e->delta() > 0) {
            currentScale *= TIGLCREATOR_ZOOM_STEP; // +10%
        }
        else {
            currentScale /= TIGLCREATOR_ZOOM_STEP; // -10%
        }
        myView->SetScale( currentScale );
    }
    else {
        e->ignore();
    }
}

void TIGLCreatorWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Delete) {
        viewerContext->eraseSelected();
    }
    else {
        QWidget::keyPressEvent(e);
    }
}


void TIGLCreatorWidget::idle( )
{
    setMode( CurAction3d_Nothing );
}


void TIGLCreatorWidget::redraw( bool isPainting )
{
    if ( !myView.IsNull() ) {
        // Defensive test.
        if ( myViewResized ) {
            myView->MustBeResized();
            viewPrecision( true );
        }
        else {
            // Don't repaint if we are already redrawing
            // elsewhere due to a keypress or mouse gesture
            if ( !isPainting || ( isPainting && myButtonFlags == Qt::NoButton ) ) {
                myView->Redraw();
            }
        }
    }
    myViewResized = Standard_False;
}



void TIGLCreatorWidget::fitExtents()
{
    if (!myView.IsNull()) {
        myView->FitAll();
        viewPrecision( true );
    }
}



void TIGLCreatorWidget::fitAll()
{
    if (!myView.IsNull()) {
        myView->ZFitAll();
        myView->FitAll();
        viewPrecision( true );
    }
}



void TIGLCreatorWidget::fitArea()
{
    setMode( CurAction3d_WindowZooming );
}



void TIGLCreatorWidget::zoom()
{
    setMode( CurAction3d_DynamicZooming );
}

void TIGLCreatorWidget::zoomIn()
{
    if ( !myView.IsNull() ){
       myView->SetScale( myView->Scale() * TIGLCREATOR_ZOOM_STEP);
    }
}

void TIGLCreatorWidget::zoomOut()
{
    if ( !myView.IsNull() ) {
       myView->SetScale( myView->Scale() / TIGLCREATOR_ZOOM_STEP);
    }
}

void TIGLCreatorWidget::zoom(double scale)
{
    if ( !myView.IsNull() ) {
       myView->SetScale( scale);
    }
}

void TIGLCreatorWidget::pan()
{
    setMode( CurAction3d_DynamicPanning );
}



void TIGLCreatorWidget::rotation()
{
    setMode( CurAction3d_DynamicRotation );
}


void TIGLCreatorWidget::selecting()
{
    setMode( CurAction3d_Nothing );
}


void TIGLCreatorWidget::globalPan()
{
    if (!myView.IsNull()) {
        // save the current zoom value
        myCurZoom = myView->Scale();
        // Do a Global Zoom
        myView->FitAll();
        viewPrecision( true );
        // Set the mode
        setMode( CurAction3d_GlobalPanning );
    }
}



void TIGLCreatorWidget::viewGrid()
{
    if (!myView.IsNull()) {
        myView->SetFront();
    }
}



void TIGLCreatorWidget::viewFront()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Xneg );
    }
}


void TIGLCreatorWidget::viewBack()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Xpos );
    }
}



void TIGLCreatorWidget::viewTop()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Zpos );
    }
}



void TIGLCreatorWidget::viewBottom()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Zneg );
    }
}


void TIGLCreatorWidget::viewLeft()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Yneg );
    }
}


void TIGLCreatorWidget::viewRight()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_Ypos );
    }
}



void TIGLCreatorWidget::viewAxo()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_XnegYnegZpos );
    }
}


void TIGLCreatorWidget::viewTopFront()
{
    if (!myView.IsNull()) {
        myView->SetProj( V3d_YnegZpos );
    }
}


void TIGLCreatorWidget::viewReset()
{
    if (!myView.IsNull()) {
        myView->Reset();
    }
}

void TIGLCreatorWidget::setLookAtPosition(double x, double y, double z)
{
    if (!myView.IsNull()) {
        myView->SetAt(x, y, z);
    }
}

void TIGLCreatorWidget::setCameraPosition(double x, double y, double z)
{
    if (!myView.IsNull()) {
        myView->SetEye(x, y, z);
    }
}

void TIGLCreatorWidget::setCameraPosition(double elevationAngleDegree, double azimuthAngleDegree)
{
    if (!myView.IsNull()) {

        double elevationAngle = elevationAngleDegree / 180. * M_PI;
        double azimuthAngle = azimuthAngleDegree / 180. * M_PI;

        // retrieve look-at position
        double xl = 0., yl = 0., zl = 0.;
        myView->At(xl, yl, zl);

        // retrieve current camera position
        double xc = 0., yc = 0., zc = 0.;
        myView->Eye(xc, yc, zc);

        // get distance
        double dist = sqrt((xc-xl)*(xc-xl) + (yc - yl)*(yc - yl) + (zc - zl)*(zc - zl));

        myView->SetEye(
            dist * cos(elevationAngle) * cos(azimuthAngle) + xl,
            dist * cos(elevationAngle) * sin(azimuthAngle) + yl,
            dist * sin(elevationAngle) + zl
        );
    }
}

void TIGLCreatorWidget::setCameraUpVector(double x, double y, double z)
{
    if (!myView.IsNull()) {
        myView->SetUp(x, y, z);
    }
}

void TIGLCreatorWidget::hiddenLineOff()
{
    if (!myView.IsNull()) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        myView->SetComputedMode( Standard_False );
        QApplication::restoreOverrideCursor();
    }
}

void TIGLCreatorWidget::hiddenLineOn()
{
    if (!myView.IsNull()) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        myView->SetComputedMode( Standard_True );
        QApplication::restoreOverrideCursor();
    }
}

void TIGLCreatorWidget::setBackgroundGradient(int r, int g, int b)
{
    myBGColor = QColor(r,g,b);
    if (!myView.IsNull()) {
        Standard_Real R1 = r/255.;
        Standard_Real G1 = g/255.;
        Standard_Real B1 = b/255.;

        // Disable provious gradient
        myView->SetBgGradientColors ( Quantity_NOC_BLACK , Quantity_NOC_BLACK, Aspect_GFM_NONE, Standard_False);

        Standard_Real fu = 2.;
        Standard_Real fd = 0.2;

        Quantity_Color up  (R1*fu > 1 ? 1. : R1*fu, G1*fu > 1 ? 1. : G1*fu, B1*fu > 1 ? 1. : B1*fu, Quantity_TOC_RGB);
        Quantity_Color down(R1*fd > 1 ? 1. : R1*fd, G1*fd > 1 ? 1. : G1*fd, B1*fd > 1 ? 1. : B1*fd, Quantity_TOC_RGB);

        myView->SetBgGradientColors( up, down, Aspect_GFM_VER, Standard_False);
    } 
    redraw();
}

void TIGLCreatorWidget::setBackgroundColor(int r, int g, int b)
{
    if (!myView.IsNull()) {
        // Disable provious gradient
        myView->SetBgGradientColors ( Quantity_NOC_BLACK , Quantity_NOC_BLACK, Aspect_GFM_NONE, Standard_False);
        myView->SetBackgroundColor(Quantity_TOC_RGB, r/255., g/255., b/255.);
        redraw();
    }
}

void TIGLCreatorWidget::setReset ()
{
    if (!myView.IsNull()) {
        myView->SetViewOrientationDefault() ;
        viewPrecision( true );
    }
}

void TIGLCreatorWidget::setBGImage(const QString& filename)
{
    if (!myView.IsNull()) {
        myView->SetBackgroundImage(filename.toStdString().c_str(), Aspect_FM_CENTERED,Standard_False);
        redraw();
    }
}

void TIGLCreatorWidget::setTransparency()
{
    auto* dialog = new TIGLSliderDialog(this);

    // Move the slider to the mouse position
    QPoint mPos = QCursor::pos();
    dialog->move(mPos.x() - dialog->size().width()/2, mPos.y() - dialog->size().height());

    connect(dialog, SIGNAL(intValueChanged(int)), this, SLOT(setTransparency(int)));
    dialog->setIntValue(30);
    dialog->setIntRange(0, 100);

    dialog->show();
    dialog->activateWindow();
}

void TIGLCreatorWidget::setTransparency(int transparency)
{
    viewerContext->setTransparency(transparency);
}

void TIGLCreatorWidget::setObjectsColor()
{
    QColor color = QColorDialog::getColor(Qt::green, this);
    viewerContext->setObjectsColor(color);
}

void TIGLCreatorWidget::setObjectsMaterial()
{
    bool ok = false;

    auto i = tiglMaterials::materialMap.begin();
    QStringList items;
    while (i != tiglMaterials::materialMap.end()) {
        items << i->first;
        i++;
    }
    QString item = QInputDialog::getItem(this, tr("Select Material"), tr("Material:"), items, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        Graphic3d_NameOfMaterial material = tiglMaterials::materialMap[item];
        viewerContext->setObjectsMaterial(material);
    }
}

void TIGLCreatorWidget::setObjectsTexture()
{
    // open file dialog
    QString fileName = QFileDialog::getOpenFileName (this,
                                                     tr("Choose texture image"),
                                                     QString(),
                                                     tr("Images (*.png *.jpeg *.jpg *.bmp);") );

    if (!fileName.isEmpty()) {
        viewerContext->setObjectsTexture(fileName);
    }
}

void TIGLCreatorWidget::onLeftButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setFocus(Qt::MouseFocusReason);
    setStartPoint(point);
    if ( nFlags & CASCADESHORTCUTKEY ) {
        setMode( CurAction3d_DynamicZooming );
    }
    else {
        switch ( myMode ) {
        case CurAction3d_Nothing:
            break;

        case CurAction3d_Picking:
            break;

        case CurAction3d_DynamicZooming:
            break;

        case CurAction3d_WindowZooming:
            break;

        case CurAction3d_DynamicPanning:
            break;

        case CurAction3d_GlobalPanning:
            break;

        case CurAction3d_DynamicRotation:
            myView->StartRotation( myStartPoint.x(), myStartPoint.y() );
            break;

        default:
            Standard_Failure::Raise( "Incompatible Current Mode" );
            break;
        }
    }
}


void TIGLCreatorWidget::onMiddleButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setStartPoint(point);
    if ( nFlags & CASCADESHORTCUTKEY ) {
        setMode( CurAction3d_DynamicPanning );
    }
    else {
        setMode( CurAction3d_DynamicRotation );
        myView->StartRotation( myStartPoint.x(), myStartPoint.y() );
    }
}



void TIGLCreatorWidget::onRightButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setStartPoint(point);
//    if ( nFlags & CASCADESHORTCUTKEY )
//    {
//        setMode( CurAction3d_DynamicRotation );
//        myView->StartRotation( point.x(), point.y() );
//    }
//    else
//    {
//        emit popupMenu ( this, point );
//    }
}



void TIGLCreatorWidget::onLeftButtonUp(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setCurrentPoint(point);
    if ( nFlags & CASCADESHORTCUTKEY ) {
        // Deactivates dynamic zooming
        setMode( CurAction3d_Nothing );
    }
    else {
        switch( myMode ) {
        case CurAction3d_Nothing:
            if ( myCurrentPoint == myStartPoint ) {
                inputEvent( nFlags & MULTISELECTIONKEY );
            }
            else {
                dragEvent( myStartPoint,
                           myCurrentPoint,
                           nFlags & MULTISELECTIONKEY );
            }
            break;

        case CurAction3d_DynamicZooming:
            viewPrecision( true );
            break;

        case CurAction3d_WindowZooming:
            if ( (abs( myCurrentPoint.x() - myStartPoint.x() ) > VALZWMIN ) ||
                 (abs( myCurrentPoint.y() - myStartPoint.y() ) > VALZWMIN ) ) {

                myView->WindowFitAll( myStartPoint.x(),
                                      myStartPoint.y(),
                                      myCurrentPoint.x(),
                                      myCurrentPoint.y() );
            }
            viewPrecision( true );
            break;

        case CurAction3d_DynamicPanning:
            break;

        case CurAction3d_GlobalPanning :
            myView->Place( myCurrentPoint.x(), myCurrentPoint.y(), myCurZoom );
            break;

        case CurAction3d_DynamicRotation:
            break;

        default:
            Standard_Failure::Raise(" Incompatible Current Mode ");
            break;
        }
    }
    emit selectionChanged();
}



void TIGLCreatorWidget::onMiddleButtonUp(  Qt::KeyboardModifiers /* nFlags */, const QPoint /* point */ )
{
    setMode( CurAction3d_Nothing );
}



void TIGLCreatorWidget::onRightButtonUp(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setCurrentPoint(point);
    if ( nFlags & CASCADESHORTCUTKEY ) {
        setMode( CurAction3d_Nothing );
    }
    else {
        if ( myMode == CurAction3d_Nothing ) {
//            emit popupMenu ( this, point );
        }
        else {
            setMode( CurAction3d_Nothing );
        }
    }
}


void TIGLCreatorWidget::onMouseMove( Qt::MouseButtons buttons,
                                    Qt::KeyboardModifiers nFlags,
                                    const QPoint point )
{
    if (myView.IsNull()) {
        return;
    }
    setCurrentPoint(point);

    if ( buttons & Qt::LeftButton  || buttons & Qt::RightButton || buttons & Qt::MidButton ) {
        switch ( myMode ) {
        case CurAction3d_Nothing:
            drawRubberBand ( myStartPoint, myCurrentPoint );
            dragEvent( myStartPoint, myCurrentPoint, nFlags & MULTISELECTIONKEY );
            break;

        case CurAction3d_DynamicZooming:
            myView->Zoom(    myStartPoint.x(),
                            myStartPoint.y(),
                            myCurrentPoint.x(),
                            myCurrentPoint.y() );
            viewPrecision( true );
            myStartPoint = myCurrentPoint;
            break;

        case CurAction3d_WindowZooming:
            drawRubberBand ( myStartPoint, myCurrentPoint );
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan( myCurrentPoint.x() - myStartPoint.x(),
                            myStartPoint.y() - myCurrentPoint.y() );
            myStartPoint = myCurrentPoint;
            break;

        case CurAction3d_GlobalPanning:
            break;

        case CurAction3d_DynamicRotation:
            myView->Rotation( myCurrentPoint.x(), myCurrentPoint.y() );
            break;

        default:
            Standard_Failure::Raise( "Incompatible Current Mode" );
            break;
        }
    }
    else {
        moveEvent( myCurrentPoint );
    }
}


AIS_StatusOfDetection TIGLCreatorWidget::moveEvent( QPoint point )
{
    AIS_StatusOfDetection status = AIS_SOD_Error;
    if (!viewerContext) {
        return status;
    }

    Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();
    status = myContext->MoveTo( point.x(), point.y(), myView, true );
    return status;
}



AIS_StatusOfPick TIGLCreatorWidget::dragEvent( const QPoint startPoint, const QPoint endPoint, const bool multi )
{
    using namespace std;
    AIS_StatusOfPick pick = AIS_SOP_NothingSelected;

    if (!viewerContext) {
        return pick;
    }
    Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();

    if (multi) {
        pick = myContext->ShiftSelect( min (startPoint.x(), endPoint.x()),
                                       min (startPoint.y(), endPoint.y()),
                                       max (startPoint.x(), endPoint.x()),
                                       max (startPoint.y(), endPoint.y()),
                                       myView, true );
    }
    else {
        pick = myContext->Select( min (startPoint.x(), endPoint.x()),
                                  min (startPoint.y(), endPoint.y()),
                                  max (startPoint.x(), endPoint.x()),
                                  max (startPoint.y(), endPoint.y()),
                                  myView, true );
    }
    emit selectionChanged();
    return pick;
}



AIS_StatusOfPick TIGLCreatorWidget::inputEvent( bool multi )
{
    AIS_StatusOfPick pick = AIS_SOP_NothingSelected;

    if (!viewerContext) {
        return pick;
    }
    Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();

    if (multi) {
        pick = myContext->ShiftSelect(true);
    }
    else {
        pick = myContext->Select(true);
    }
    if ( pick != AIS_SOP_NothingSelected ) {
        emit selectionChanged();
    }
    return pick;
}

void TIGLCreatorWidget::setMode( const CurrentAction3d mode )
{
    if ( mode != myMode ) {
        switch ( mode ) {
        case CurAction3d_DynamicPanning:
            setCursor( Qt::SizeAllCursor );
            break;
        case CurAction3d_DynamicZooming:
            setCursor( Qt::PointingHandCursor );
            break;
        case CurAction3d_DynamicRotation:
            setCursor( Qt::CrossCursor );
            break;
        case CurAction3d_GlobalPanning:
            setCursor( Qt::CrossCursor );
            break;
        case CurAction3d_WindowZooming:
            setCursor( Qt::PointingHandCursor );
            break;
        case CurAction3d_Nothing:
            //setCursor( myCrossCursor );
            setCursor( Qt::CrossCursor );
            break;
        default:
            setCursor( Qt::ArrowCursor );
            break;
        }
        myMode = mode;
    }
}



Standard_Real TIGLCreatorWidget::precision( Standard_Real aReal )
{
    using namespace std;
    Standard_Real preciseReal;
    Standard_Real thePrecision = max (myPrecision, viewPrecision());
    
    if ( myPrecision != 0.0 ) {
        preciseReal =  SIGN(aReal) * floor((std::abs(aReal) + thePrecision * 0.5) / thePrecision) * thePrecision;
    }
    else {
        preciseReal = aReal;
    }
    return preciseReal;
}


Standard_Boolean TIGLCreatorWidget::convertToPlane(Standard_Integer Xs,
                                                  Standard_Integer Ys,
                                                  Standard_Real& X,
                                                  Standard_Real& Y,
                                                  Standard_Real& Z)
{
    Standard_Real Xv, Yv, Zv;
    Standard_Real Vx, Vy, Vz;

    if (myView.IsNull() || myViewer.IsNull()) {
        return Standard_False;
    }

    gp_Pln aPlane(myViewer->PrivilegedPlane());

    myView->Convert( Xs, Ys, Xv, Yv, Zv ); 

    myView->Proj( Vx, Vy, Vz );
    gp_Lin aLine(gp_Pnt(Xv, Yv, Zv), gp_Dir(Vx, Vy, Vz));
    IntAna_IntConicQuad theIntersection( aLine, aPlane, Precision::Angular() );
    if (theIntersection.IsDone()) {
        if (!theIntersection.IsParallel()) {
            if (theIntersection.NbPoints() > 0) {
                gp_Pnt theSolution(theIntersection.Point(1));
                X = theSolution.X();
                Y = theSolution.Y();
                Z = theSolution.Z();
                return Standard_True;
            }
        }
    }
    return Standard_False;
}


void TIGLCreatorWidget::drawRubberBand( const QPoint origin, const QPoint position )
{
    double left   = origin.x();
    double right  = position.x();
    double top    = origin.y();
    double bottom = position.y();

    // Layers were removed in OCCT 7. Instead, the platform
    // independent AIS_RubberBand was introduced
    //
    // Therefore are we using layers in OCCT 6x and
    // AIS_Rubberband in OCCT 7x
#if OCC_VERSION_HEX < 0x070000
    if ( !myLayer.IsNull() && !myView.IsNull() ) {

        int width, height;
        myView->Window()->Size(width, height);
        
        myLayer->Clear(); 
        // The -1 is a hack from the opencascade forums to avoid clipping
        // of the coordinates. This way it behaves identically to opengl
        myLayer->SetOrtho(0, width, height, 0, (Aspect_TypeOfConstraint) -1);
        
        myLayer->Begin();
        myLayer->SetTransparency(1.0);
        myLayer->SetLineAttributes(Aspect_TOL_DOT, 1.);

        // Draw black-white dotted, imitate a shadowy look
        // This makes it possible to draw even on white or
        // black backgrounds
        myLayer->SetColor(Quantity_NOC_WHITE);
        myLayer->BeginPolyline();
        myLayer->AddVertex(left,  top);
        myLayer->AddVertex(right, top);
        myLayer->AddVertex(right, bottom);
        myLayer->AddVertex(left,  bottom);
        myLayer->AddVertex(left,  top);
        myLayer->ClosePrimitive();

        myLayer->SetColor(Quantity_NOC_BLACK);
        myLayer->BeginPolyline();
        myLayer->AddVertex(left+1,  top+1);
        myLayer->AddVertex(right+1, top+1);
        myLayer->AddVertex(right+1, bottom+1);
        myLayer->AddVertex(left+1,  bottom+1);
        myLayer->AddVertex(left+1,  top+1);
        myLayer->ClosePrimitive();
        myLayer->End();
    }
#else
    auto scale = devicePixelRatioF();
    if (viewerContext) {
        Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();
        // Draw black-white dotted, imitate a shadowy look
        // This makes it possible to draw even on white or
        // black backgrounds
        whiteRect->SetRectangle(left, height()*scale-bottom, right, height()*scale-top);
        blackRect->SetRectangle(left+1, height()*scale-bottom-1, right+1, height()*scale-top-1);

        if (!myContext->IsDisplayed (whiteRect)) {
            myContext->Display (whiteRect, Standard_False);
            myContext->Display (blackRect, Standard_False);
        }
        else {
            myContext->Redisplay (whiteRect, Standard_False);
            myContext->Redisplay (blackRect, Standard_False);
        }

        myContext->CurrentViewer()->RedrawImmediate();
    }
#endif
}


void TIGLCreatorWidget::hideRubberBand( void )
{
#if OCC_VERSION_HEX < 0x070000
    if (!myLayer.IsNull() ) {
        myLayer->Clear();
    }
#else
    if (viewerContext) {
        Handle(AIS_InteractiveContext) myContext = viewerContext->getContext();
        myContext->Remove (whiteRect, Standard_False);
        myContext->Remove (blackRect, Standard_False);
        myContext->CurrentViewer()->RedrawImmediate();
    }
#endif
}


Standard_Real TIGLCreatorWidget::viewPrecision( bool resized )
{

    Standard_Real X1, Y1, Z1;
    Standard_Real X2, Y2, Z2;

    if (resized || myViewPrecision == 0.0) {
        myView->Convert( 0, 0, X1, Y1, Z1 ); 
        myView->Convert( 1, 0, X2, Y2, Z2 ); 
        Standard_Real pixWidth = X2 - X1;
        if ( pixWidth != 0.0 ) {
            // Return the precision as the next highest decade above the pixel width
            myViewPrecision = std::pow (10.0, std::floor(std::log10( pixWidth ) + 1.0));
        }
        else {
            // Return the user precision if window not defined
            myViewPrecision = myPrecision; 
        }
    }
    return myViewPrecision;
}

/** 
 * Note:This function with custom width and height seems to be working only on windows
 * If it fails, we are switching back to the current screen resolution.
 */
bool TIGLCreatorWidget::makeScreenshot(const QString& filename, bool whiteBGEnabled, int width, int height, int quality)
{
    if (!myView) {
        return false;
    }

    if (whiteBGEnabled) {
        myView->SetBgGradientColors ( Quantity_NOC_BLACK , Quantity_NOC_BLACK, Aspect_GFM_NONE, Standard_False);
        myView->SetBackgroundColor(Quantity_NOC_WHITE);
        myView->Redraw();
    }

    // get window size
    // we could also use a higher resolution if we want
    if (width == 0 || height == 0) {
        myView->Window()->Size(width, height);
    }

    // write screenshot to pixmap
    Image_PixMap pixmap;
    bool success = myView->ToPixMap(pixmap, width, height, Graphic3d_BT_RGB);
    if (!success) {
        // use size of the window
        LOG(WARNING) << "Error changing image size to " << width << "x" << height 
                     << " for screenshot. Using current resolution.";
        myView->Window()->Size(width, height);
        success = myView->ToPixMap(pixmap, width, height, Graphic3d_BT_RGB);
    }

    if (whiteBGEnabled) {
        // reset color
        setBackgroundGradient(myBGColor.red(), myBGColor.green(), myBGColor.blue());
    }

    // copy to qimage which supports a variety of file formats
    QImage img(QSize((int) pixmap.Width(), (int) pixmap.Height()), QImage::Format_RGB888);
    for (unsigned int aRow = 0; aRow <  pixmap.Height(); ++aRow) {
      for (unsigned int aCol = 0; aCol < pixmap.Width(); ++aCol) {
        // extremely SLOW but universal (implemented for all supported pixel formats)
        Quantity_Color aColor = pixmap.PixelColor ((Standard_Integer )aCol, (Standard_Integer )aRow)
#if OCC_VERSION_HEX >= 0x070200
          .GetRGB()
#endif
          ;
        QColor qcol(
                static_cast<int>(aColor.Red()*255),
                static_cast<int>(aColor.Green()*255),
                static_cast<int>(aColor.Blue()*255));
        img.setPixel(aCol, aRow, qcol.rgb());
      }
    }

    // Tigl Icon for brand marking
    QImage tiglIcon(":gfx/logo-trans.png");

    // scale image to a reasonable size compared to the screenshot
    tiglIcon = tiglIcon.scaledToWidth(width/8);

    // watermark padding distance to corner
    static constexpr unsigned int padding = 20;
    int xPos = width - tiglIcon.width() - padding;
    int yPos = height - tiglIcon.height() - padding;

    // only draw watermark if there is enough space on the image to draw it
    if (xPos > 0 && yPos >= 0 && xPos + tiglIcon.width() < width && yPos + tiglIcon.height() < height) {
        QPainter p(&img);
        p.setOpacity(0.5);

        // draw watermark
        p.drawImage(xPos, yPos, tiglIcon);
    }

    if (!img.save(filename, nullptr, quality)) {
        LOG(ERROR) << "Unable to save screenshot to file '" + filename.toStdString() + "'";
        return false;
    }
    else {
        return true;
    }

}

void TIGLCreatorWidget::contextMenuEvent(QContextMenuEvent *event)
 {

     if (viewerContext->hasSelectedShapes()) {
        QMenu menu(this);

        QAction *transparencyAct;
        transparencyAct = new QAction(tr("&Transparency"), this);
        transparencyAct->setStatusTip(tr("Component Transparency"));
        menu.addAction(transparencyAct);
        connect(transparencyAct, SIGNAL(triggered()), this, SLOT(setTransparency()));

        QMenu* renderingModeMenu = new QMenu("&Rendering mode", this);
        menu.addMenu(renderingModeMenu);

        QAction *wireframeAct;
        wireframeAct = new QAction(tr("&Wireframe"), this);
        wireframeAct->setStatusTip(tr("Component Wireframe"));
        renderingModeMenu->addAction(wireframeAct);
        connect(wireframeAct, SIGNAL(triggered()), viewerContext, SLOT(setObjectsWireframe()));

        QAction *shadingAct;
        shadingAct = new QAction(tr("&Shaded"), this);
        shadingAct->setStatusTip(tr("Component Shading"));
        renderingModeMenu->addAction(shadingAct);
        connect(shadingAct, SIGNAL(triggered()), viewerContext, SLOT(setObjectsShading()));

        QAction *textureAct;
        textureAct = new QAction(tr("&Textured"), this);
        textureAct->setStatusTip(tr("Apply a texture image to the shape"));
        renderingModeMenu->addAction(textureAct);
        connect(textureAct, SIGNAL(triggered()), this, SLOT(setObjectsTexture()));

        QAction *colorAct;
        colorAct = new QAction(tr("&Color"), this);
        colorAct->setStatusTip(tr("Component Color"));
        menu.addAction(colorAct);
        connect(colorAct, SIGNAL(triggered()), this, SLOT(setObjectsColor()));

        QAction *materialAct;
        materialAct = new QAction(tr("&Material"), this);
        materialAct->setStatusTip(tr("Component Material"));
        menu.addAction(materialAct);
        connect(materialAct, SIGNAL(triggered()), this, SLOT(setObjectsMaterial()));

        menu.addSeparator();

        QAction *eraseAct;
        eraseAct = new QAction(tr("&Erase"), this);
        eraseAct->setStatusTip(tr("Erase selected components"));
        eraseAct->setIcon(QIcon(":/gfx/document-close.png"));
        menu.addAction(eraseAct);
        connect(eraseAct, SIGNAL(triggered()), viewerContext, SLOT(eraseSelected()));

        menu.exec(event->globalPos());
     }

 }

void TIGLCreatorWidget::setStartPoint(const QPoint& p)
{
    auto scale = devicePixelRatioF();
    myStartPoint = QPoint(p.x()*scale, p.y()*scale);
}

void TIGLCreatorWidget::setCurrentPoint(const QPoint& p)
{
    auto scale = devicePixelRatioF();
    myCurrentPoint = QPoint(p.x()*scale, p.y()*scale);
}
