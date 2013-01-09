/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerWidget.cpp 226 2012-10-23 19:18:29Z martinsiggel $
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
#include <algorithm>
#include <cmath>
#include <iostream>

#ifdef WIN32
  #include <windows.h>
  #include <WNT_WDriver.hxx>
  #include <WNT_Window.hxx>
#elif defined __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <Cocoa_Window.hxx>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glx.h>
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <X11/Xatom.h>
  #include <X11/Xmu/StdCmap.h>
  #undef QT_CLEAN_NAMESPACE
  #include <Xw_Window.hxx>
#endif

#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QtGui/QInputEvent>
#include <QtGui/QColorDialog>
#include <QtGui/QPlastiqueStyle>
#include <QMessageBox>
#include <QInputDialog>

#include "TIGLViewerWidget.h"
#include "TIGLViewerInternal.h"
#include "TIGLViewerDocument.h"

#include "Visual3d_Layer.hxx"


// 10% zoom per wheel or key event
#define TIGLVIEWER_ZOOM_STEP 1.10

TIGLViewerWidget::TIGLViewerWidget(QWidget * parent)
    : QWidget(parent) {
    initialize();
}

TIGLViewerWidget::TIGLViewerWidget( const Handle_AIS_InteractiveContext& aContext,
								QWidget *parent, 
                                Qt::WindowFlags f ) :
	myView            ( NULL ),
    myViewer          ( NULL ),
    myLayer           ( NULL ),
    myViewResized	  ( Standard_False ),
    myViewInitialized ( Standard_False ),
	myMode			  ( CurAction3d_Undefined ),
	myGridSnap        ( Standard_False ),
    myDetection		  ( AIS_SOD_Nothing ),
    myRubberBand      ( NULL ),
	myPrecision		  ( 0.001 ),
	myViewPrecision   ( 0.0 ),
	myKeyboardFlags   ( Qt::NoModifier ),
	myButtonFlags	  ( Qt::NoButton ){
    initialize();
    myContext = aContext;
}

void TIGLViewerWidget::initialize(){
    myView            = NULL;
    myViewer          = NULL;
    myLayer           = NULL;
    myRubberBand      = NULL;
    myMode			  = CurAction3d_Undefined;
    myGridSnap        = Standard_False;
    myViewResized	  = Standard_False;
    myViewInitialized = Standard_False;
    myPrecision		  = 0.001;
    myViewPrecision   = 0.0;
    myDetection		  = AIS_SOD_Nothing;
    myKeyboardFlags   = Qt::NoModifier;
    myButtonFlags	  = Qt::NoButton;

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

    // Create a rubber band box for later mouse activity
    myRubberBand = new QRubberBand( QRubberBand::Rectangle, this );
    if (myRubberBand)
    {
        // If you don't set a style, QRubberBand doesn't work properly
        // take this line out if you don't believe me.
        myRubberBand->setStyle( (QStyle*) new QPlastiqueStyle() );
    }
}


TIGLViewerWidget::~TIGLViewerWidget()
{
	if ( myRubberBand )
	{
		delete myRubberBand;
	}
}


void TIGLViewerWidget::initializeOCC(const Handle_AIS_InteractiveContext& aContext)
{
	Aspect_RenderingContext rc = 0;
	myContext = aContext;
	myViewer  = myContext->CurrentViewer();
	myView    = myViewer->CreateView();
	
	int windowHandle = (int) winId();
    short lo = (short)   windowHandle;
    short hi = (short) ( windowHandle >> 16 );

#if defined WNT
	// rc = (Aspect_RenderingContext) wglGetCurrentContext();
    myWindow = new WNT_Window( Handle(Graphic3d_WNTGraphicDevice)
							   ::DownCast( myContext->CurrentViewer()->Device() ) ,
							   (int) hi, (int) lo );
	// Turn off background erasing in OCC's window
	myWindow->SetFlags( WDF_NOERASEBKGRND );
#elif defined __APPLE__
    myWindow = new Cocoa_Window(reinterpret_cast<NSView *>(winId()));
#else
	// rc = (Aspect_RenderingContext) glXGetCurrentContext(); // Untested!
    myWindow = new Xw_Window( Handle(Graphic3d_GraphicDevice)
									::DownCast( myContext->CurrentViewer()->Device() ),
							  (int) hi, (int) lo, Xw_WQ_SAMEQUALITY, Quantity_NOC_BLACK );
#endif // WNT
	
	if (!myView.IsNull())
	{
		// Set my window (Hwnd) into the OCC view
	    myView->SetWindow( myWindow, rc , paintCallBack, this  );
		// Set up axes (Trihedron) in lower left corner.
		myView->SetScale( 2 );			// Choose a "nicer" initial scale

		// Set up axes (Trihedron) in lower left corner.
#ifdef OCC_PATCHED
		myView->TriedronDisplay( Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.1, V3d_ZBUFFER );
#else
		myView->TriedronDisplay( Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.1, V3d_WIREFRAME );
#endif
        //myView->SetAntialiasingOn();

		//myView->ColorScaleDisplay();

		// Map the window
		if (!myWindow->IsMapped())
		{
			myWindow->Map();
		}
		// Force a redraw to the new window on next paint event
		myViewResized = Standard_True;
		// Set default cursor as a cross
		setMode( CurAction3d_Nothing );
		// This is to signal any connected slots that the view is ready.
		myViewInitialized = Standard_True;

		myLayer   = new Visual3d_Layer (myViewer->Viewer(), Aspect_TOL_OVERLAY, Standard_True /*aSizeDependant*/);

#ifdef OCC_BG
        myView->SetBgGradientColors ( Quantity_Color(245./256., 245./256., 252./256. , Quantity_TOC_RGB), Quantity_Color(49./256., 49./256., 49./256. , Quantity_TOC_RGB), Aspect_GFM_VER, Standard_False);
#endif


		emit initialized();
	}
}


QPaintEngine* TIGLViewerWidget::paintEngine() const
{
	return NULL;
}


void TIGLViewerWidget::paintEvent ( QPaintEvent * /* e */)
{
	if ( !myViewInitialized )
	{
		if ( winId() )
		{
			initializeOCC( myContext );
		}
	}
	if ( !myViewer.IsNull() )
	{
		redraw( true );	
	}
}



void TIGLViewerWidget::resizeEvent ( QResizeEvent * /* e */ )
{
	myViewResized = Standard_True;
}	


void TIGLViewerWidget::mousePressEvent( QMouseEvent* e )
{
	myButtonFlags = e->button();

	// Cache the keyboard flags for the whole gesture
	myKeyboardFlags = e->modifiers();

	// The button mappings can be used as a mask. This code prevents conflicts
	// when more than one button pressed simultaneously.
	if ( e->button() & Qt::LeftButton )
	{
		onLeftButtonDown  ( myKeyboardFlags, e->pos() );
	}
	else if ( e->button() & Qt::RightButton )
	{
		onRightButtonDown ( myKeyboardFlags, e->pos() );
	}
	else if ( e->button() & Qt::MidButton )
	{
		onMiddleButtonDown( myKeyboardFlags, e->pos() );
	}
}



void TIGLViewerWidget::mouseReleaseEvent(QMouseEvent* e)
{
	myButtonFlags = Qt::NoButton;
	redraw();							// Clears up screen when menu selected but not used.
	hideRubberBand();
	if ( e->button() & Qt::LeftButton )
	{
		onLeftButtonUp  ( myKeyboardFlags, e->pos() );
	}
	else if ( e->button() & Qt::RightButton )
	{
		onRightButtonUp ( myKeyboardFlags, e->pos() );
	}
	else if ( e->button() & Qt::MidButton )
	{
		onMiddleButtonUp( myKeyboardFlags, e->pos() );
	}
}



void TIGLViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
	Standard_Real X, Y, Z;
	
	myCurrentPoint = e->pos();
	//Check if the grid is active and that we're snapping to it
	if( myContext->CurrentViewer()->Grid()->IsActive() && myGridSnap )
	{
		myView->ConvertToGrid( myCurrentPoint.x(),
							   myCurrentPoint.y(),
							   myV3dX,
							   myV3dY,
							   myV3dZ );
		emit mouseMoved( myV3dX, myV3dY, myV3dZ );
	}
	else //	this is the standard case
	{
		if (convertToPlane( myCurrentPoint.x(),
							myCurrentPoint.y(),
							X, Y, Z ) )
		{
			myV3dX = precision( X );
			myV3dY = precision( Y );
			myV3dZ = precision( Z );
			emit mouseMoved( myV3dX, myV3dY, myV3dZ );
		}
		else
		{
			emit sendStatus ( tr("Indeterminate Point") );
		}
	}

	onMouseMove( e->buttons(), myKeyboardFlags, e->pos() );
}



void TIGLViewerWidget::leaveEvent ( QEvent* /* e */ )
{
	myButtonFlags = Qt::NoButton;
}


void TIGLViewerWidget::wheelEvent ( QWheelEvent* e )
{
	if ( !myView.IsNull() )
	{
		Standard_Real currentScale = myView->Scale();
		if (e->delta() > 0)
		{
            currentScale *= TIGLVIEWER_ZOOM_STEP; // +10%
		}
		else
		{
            currentScale /= TIGLVIEWER_ZOOM_STEP; // -10%
		}
		myView->SetScale( currentScale );
	}
	else
	{
		e->ignore();
	}
}


void TIGLViewerWidget::idle( )
{
	setMode( CurAction3d_Nothing );
}


void TIGLViewerWidget::redraw( bool isPainting )
{
	if ( !myView.IsNull() )					// Defensive test.
	{
		if ( myViewResized )
		{
			myView->MustBeResized();
			viewPrecision( true );
		}
		else
		{	
			// Don't repaint if we are already redrawing
			// elsewhere due to a keypress or mouse gesture
			if ( !isPainting || ( isPainting && myButtonFlags == Qt::NoButton ) )	
			{												
				myView->Redraw();
			}
		}
	}
	myViewResized = Standard_False;
}



void TIGLViewerWidget::fitExtents( void )
{
	if (!myView.IsNull())
	{
		myView->FitAll();
		viewPrecision( true );
	}
}



void TIGLViewerWidget::fitAll( void )
{
	if (!myView.IsNull())
	{
		myView->ZFitAll();
		myView->FitAll();
		viewPrecision( true );
	}
}



void TIGLViewerWidget::fitArea( void )
{
    setMode( CurAction3d_WindowZooming );
}



void TIGLViewerWidget::zoom( void )
{
    setMode( CurAction3d_DynamicZooming );
}

void TIGLViewerWidget::zoomIn()
{
    if( !myView.IsNull() ){
       myView->SetScale( myView->Scale() * TIGLVIEWER_ZOOM_STEP);
    }
}

void TIGLViewerWidget::zoomOut()
{
    if( !myView.IsNull() ){
       myView->SetScale( myView->Scale() / TIGLVIEWER_ZOOM_STEP);
    }
}

void TIGLViewerWidget::pan( void )
{
    setMode( CurAction3d_DynamicPanning );
}



void TIGLViewerWidget::rotation( void )
{
    setMode( CurAction3d_DynamicRotation );
}


void TIGLViewerWidget::selecting( void )
{
    setMode( CurAction3d_Nothing );
}


void TIGLViewerWidget::globalPan()
{
	if (!myView.IsNull())
	{
		// save the current zoom value
		myCurZoom = myView->Scale();
		// Do a Global Zoom
		myView->FitAll();
		viewPrecision( true );
		// Set the mode
		setMode( CurAction3d_GlobalPanning );
	}
}



void TIGLViewerWidget::viewGrid()
{
	if(!myView.IsNull())
	{
		myView->SetFront();
	}
}



void TIGLViewerWidget::viewFront()
{
	if(!myView.IsNull())
	{
        myView->SetProj( V3d_Xneg );
	}
}


void TIGLViewerWidget::viewBack()
{
	if(!myView.IsNull())
	{
        myView->SetProj( V3d_Xpos );
	}
}



void TIGLViewerWidget::viewTop()
{
	if(!myView.IsNull())
	{
	    myView->SetProj( V3d_Zpos );
	}
}



void TIGLViewerWidget::viewBottom()
{
	if(!myView.IsNull())
	{
	    myView->SetProj( V3d_Zneg );
	}
}


void TIGLViewerWidget::viewLeft()
{
	if(!myView.IsNull())
	{
        myView->SetProj( V3d_Yneg );
	}
}


void TIGLViewerWidget::viewRight()
{
	if(!myView.IsNull())
	{
        myView->SetProj( V3d_Ypos );
	}
}



void TIGLViewerWidget::viewAxo()
{
	if(!myView.IsNull())
	{
	    myView->SetProj( V3d_XnegYnegZpos );
	}
}


void TIGLViewerWidget::viewTopFront()
{
	if(!myView.IsNull())
	{
		myView->SetProj( V3d_YnegZpos );
	}
}


void TIGLViewerWidget::viewReset()
{
	if(!myView.IsNull())
	{
	    myView->Reset();
	}
}

void TIGLViewerWidget::hiddenLineOff()
{
	if(!myView.IsNull())
	{
	    QApplication::setOverrideCursor( Qt::WaitCursor );
		myView->SetComputedMode( Standard_False );
		QApplication::restoreOverrideCursor();
	}
}

void TIGLViewerWidget::hiddenLineOn()
{
	if(!myView.IsNull())
	{
	    QApplication::setOverrideCursor( Qt::WaitCursor );
		myView->SetComputedMode( Standard_True );
		QApplication::restoreOverrideCursor();
	}
}

void TIGLViewerWidget::background()
{
    QColor aColor ;
    Standard_Real R1;
    Standard_Real G1;
    Standard_Real B1;
#ifdef OCC_BG
    Quantity_Color c1, c2;
    myView->GradientBackgroundColors(c1,c2);
    R1 = c1.Red(); G1 = c1.Green(); B1 = c1.Blue();
#else
    myView->BackgroundColor(Quantity_TOC_RGB,R1,G1,B1);
#endif
    aColor.setRgb(( int )R1*255, ( int )G1*255, ( int )B1*255);

    QColor aRetColor = QColorDialog::getColor(aColor);

    if( aRetColor.isValid() )
	{
        R1 = aRetColor.red()/255.;
        G1 = aRetColor.green()/255.;
        B1 = aRetColor.blue()/255.;
#ifdef OCC_BG
        // Disable provious gradient
        myView->SetBgGradientColors ( Quantity_NOC_BLACK , Quantity_NOC_BLACK, Aspect_GFM_NONE, Standard_False);

        float fu = 2;
        float fd = 0.2;

        Quantity_Color up  (R1*fu > 1 ? 1. : R1*fu, G1*fu > 1 ? 1. : G1*fu, B1*fu > 1 ? 1. : B1*fu, Quantity_TOC_RGB);
        Quantity_Color down(R1*fd > 1 ? 1. : R1*fd, G1*fd > 1 ? 1. : G1*fd, B1*fd > 1 ? 1. : B1*fd, Quantity_TOC_RGB);

        myView->SetBgGradientColors( up, down, Aspect_GFM_VER, Standard_False);

#else
        myView->SetBackgroundColor(Quantity_TOC_RGB,R1,G1,B1);
#endif
    }
    redraw();
}

void TIGLViewerWidget::setReset ()
{
	if(!myView.IsNull())
	{
		myView->SetViewOrientationDefault() ;
		viewPrecision( true );
	}
}

void TIGLViewerWidget::setBGImage(const QString& filename){
    if(!myView.IsNull()){
        myView->SetBackgroundImage(filename.toStdString().c_str(), Aspect_FM_CENTERED,Standard_False);
        redraw();
    }
}

void TIGLViewerWidget::eraseSelected()
{
    if(!myView.IsNull())
    {
        for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent())
            myContext->Erase(myContext->Current(), Standard_True, Standard_False);

        myContext->ClearCurrents();
    }
}


void TIGLViewerWidget::setTransparency()
{
    bool ok;
    int transparency;

    QInputDialog *dialog = new QInputDialog(this);
    dialog->setInputMode(QInputDialog::IntInput);
    connect(dialog, SIGNAL(intValueChanged(int)), this, SLOT(setTransparency(int)));
    transparency = dialog->getInt(this, tr("Select transparency level"), tr("Transparency:"), 25, 0, 100, 1, &ok);
    setTransparency(transparency);
}

void TIGLViewerWidget::setTransparency(int tr)
{
    Standard_Real transparency = 0.1;
    if ( (tr < 0) || (tr > 100)) {
        return;
    }
    transparency = tr * 0.01;

    if(!myView.IsNull())
    {
        for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent()) {
            myContext->SetTransparency(myContext->Current(), transparency, Standard_True);
        }
    }
}

void TIGLViewerWidget::setObjectsWireframe()
{
    if(!myView.IsNull())
    {
        for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent()) {
            myContext->SetDisplayMode(myContext->Current(), 0);
        }
    }
}

void TIGLViewerWidget::setObjectsShading()
{
    if(!myView.IsNull())
    {
        for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent()) {
            myContext->SetDisplayMode(myContext->Current(), 1);
        }
    }
}

void TIGLViewerWidget::setObjectsColor()
{
    QColor color = QColorDialog::getColor(Qt::green, this);
     if (color.isValid()) {
         for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent()) {
             myContext->SetColor (myContext->Current(),Quantity_Color(color.red()/255., color.green()/255., color.blue()/255., Quantity_TOC_RGB));
          }
     }
}

void TIGLViewerWidget::setObjectsMaterial()
{
    bool ok;
    QStringList items;

    QMap<QString, Graphic3d_NameOfMaterial> materialMap;
    materialMap["Brass"] = Graphic3d_NOM_BRASS;
    materialMap["Bronze"] = Graphic3d_NOM_BRONZE;
    materialMap["Copper"] = Graphic3d_NOM_COPPER;
    materialMap["Gold"] = Graphic3d_NOM_GOLD;
    materialMap["Pewter"] = Graphic3d_NOM_PEWTER;
    materialMap["Plaster"] = Graphic3d_NOM_PLASTER;
    materialMap["Plastic"] = Graphic3d_NOM_PLASTIC;
    materialMap["Silver"] = Graphic3d_NOM_SILVER;
    materialMap["Steel"] = Graphic3d_NOM_STEEL;
    materialMap["Stone"] = Graphic3d_NOM_STONE;
    materialMap["Shiny Plastic"] = Graphic3d_NOM_SHINY_PLASTIC;
    materialMap["Satin"] = Graphic3d_NOM_SATIN;
    materialMap["Metalized"] = Graphic3d_NOM_METALIZED;
    materialMap["Neon GNC"] = Graphic3d_NOM_NEON_GNC;
    materialMap["Chrome"] = Graphic3d_NOM_CHROME;
    materialMap["Aluminium"] = Graphic3d_NOM_ALUMINIUM;
    materialMap["Obsidian"] = Graphic3d_NOM_OBSIDIAN;
    materialMap["Neon PHC"] = Graphic3d_NOM_NEON_PHC;
    materialMap["Jade"] = Graphic3d_NOM_JADE;
    materialMap["Default"] = Graphic3d_NOM_DEFAULT;

    QMapIterator<QString, Graphic3d_NameOfMaterial> i(materialMap);
    while (i.hasNext()) {
        i.next();
        items << i.key();
    }
    QString item = QInputDialog::getItem(this, tr("Select Material"), tr("Material:"), items, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        for (myContext->InitCurrent(); myContext->MoreCurrent(); myContext->NextCurrent()) {
             myContext->SetMaterial (myContext->Current(),  materialMap[item]);
        }
    }
}


void TIGLViewerWidget::onLeftButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
    setFocus(Qt::MouseFocusReason);
    myStartPoint = point;
    if ( nFlags & CASCADESHORTCUTKEY )
    {
		setMode( CurAction3d_DynamicZooming );
    }
    else
    {
        switch ( myMode )
        {
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


void TIGLViewerWidget::onMiddleButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
	myStartPoint = point;
    if ( nFlags & CASCADESHORTCUTKEY )
	{
		setMode( CurAction3d_DynamicPanning );
	}
	else
	{
		setMode( CurAction3d_DynamicRotation );
		myView->StartRotation( myStartPoint.x(), myStartPoint.y() );
	}
}



void TIGLViewerWidget::onRightButtonDown(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
	myStartPoint = point;
//    if ( nFlags & CASCADESHORTCUTKEY )
//    {
//		setMode( CurAction3d_DynamicRotation );
//		myView->StartRotation( point.x(), point.y() );
//    }
//    else
//    {
//        emit popupMenu ( this, point );
//    }
}



void TIGLViewerWidget::onLeftButtonUp(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
	myCurrentPoint = point;
	if ( nFlags & CASCADESHORTCUTKEY )
    {
		// Deactivates dynamic zooming
		setMode( CurAction3d_Nothing );
	}
	else
	{
		switch( myMode )
		{
			case CurAction3d_Nothing:
				if ( myCurrentPoint == myStartPoint )
				{
					inputEvent( nFlags & MULTISELECTIONKEY );
				}
				else
				{
					dragEvent( myStartPoint,
						       myCurrentPoint,
							   nFlags & MULTISELECTIONKEY );
				}
				break;

			case CurAction3d_DynamicZooming:
				viewPrecision( true );
				break;

			case CurAction3d_WindowZooming:
				if ( (abs( myCurrentPoint.x() - myStartPoint.x() ) > ValZWMin ) ||
					 (abs( myCurrentPoint.y() - myStartPoint.y() ) > ValZWMin ) )
				{
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



void TIGLViewerWidget::onMiddleButtonUp(  Qt::KeyboardModifiers /* nFlags */, const QPoint /* point */ )
{
    setMode( CurAction3d_Nothing );
}



void TIGLViewerWidget::onRightButtonUp(  Qt::KeyboardModifiers nFlags, const QPoint point )
{
	myCurrentPoint = point;
	if ( nFlags & CASCADESHORTCUTKEY )
    {
		setMode( CurAction3d_Nothing );
	}
	else
	{
		if ( myMode == CurAction3d_Nothing )
		{
//			emit popupMenu ( this, point );
		}
		else
		{
			setMode( CurAction3d_Nothing );
		}
	}
}


void TIGLViewerWidget::onMouseMove( Qt::MouseButtons buttons,
								   Qt::KeyboardModifiers nFlags,
								   const QPoint point )
{
	myCurrentPoint = point;

	if ( buttons & Qt::LeftButton  || buttons & Qt::RightButton || buttons & Qt::MidButton )
	{
		switch ( myMode )
		{
			case CurAction3d_Nothing:
				drawRubberBand ( myStartPoint, myCurrentPoint );
				dragEvent( myStartPoint, myCurrentPoint, nFlags & MULTISELECTIONKEY );
				break;

			case CurAction3d_DynamicZooming:
				myView->Zoom(	myStartPoint.x(),
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
	else
	{
        moveEvent( myCurrentPoint );
    }
}


AIS_StatusOfDetection TIGLViewerWidget::moveEvent( QPoint point )
{
	AIS_StatusOfDetection status;
	status = myContext->MoveTo( point.x(), point.y(), myView );
	return status;
}



AIS_StatusOfPick TIGLViewerWidget::dragEvent( const QPoint startPoint, const QPoint endPoint, const bool multi )
{
	using namespace std;
	AIS_StatusOfPick pick = AIS_SOP_NothingSelected;
	if (multi)
	{
		pick = myContext->ShiftSelect( min (startPoint.x(), endPoint.x()),
									   min (startPoint.y(), endPoint.y()),
									   max (startPoint.x(), endPoint.x()),
									   max (startPoint.y(), endPoint.y()),
									   myView );
	}
	else
	{
		pick = myContext->Select( min (startPoint.x(), endPoint.x()),
								  min (startPoint.y(), endPoint.y()),
								  max (startPoint.x(), endPoint.x()),
								  max (startPoint.y(), endPoint.y()),
								  myView );
	}
    emit selectionChanged();
	return pick;
}



AIS_StatusOfPick TIGLViewerWidget::inputEvent( bool multi )
{
	AIS_StatusOfPick pick = AIS_SOP_NothingSelected;

	if (multi)
	{
		pick = myContext->ShiftSelect();
	}
	else
	{
	    pick = myContext->Select();
	}
	if ( pick != AIS_SOP_NothingSelected )
	{
		emit selectionChanged();
	}
	return pick;
}

bool TIGLViewerWidget::dump(Standard_CString theFile)
{
  redraw();
  return myView->Dump(theFile);
}



void TIGLViewerWidget::setMode( const CurrentAction3d mode )
{
	if ( mode != myMode )
	{
		switch( mode )
		{
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



Standard_Real TIGLViewerWidget::precision( Standard_Real aReal )
{
	using namespace std;
	Standard_Real preciseReal;
	Standard_Real thePrecision = max (myPrecision, viewPrecision());
	
	if ( myPrecision != 0.0 )
	{
		preciseReal =  SIGN(aReal) * floor((std::abs(aReal) + thePrecision * 0.5) / thePrecision) * thePrecision;
	}
	else
	{
		preciseReal = aReal;
	}
	return preciseReal;
}


Standard_Boolean TIGLViewerWidget::convertToPlane(const Standard_Integer Xs,
											     const Standard_Integer Ys,
												 Standard_Real& X,
												 Standard_Real& Y,
												 Standard_Real& Z)
{
	Standard_Real Xv, Yv, Zv;
	Standard_Real Vx, Vy, Vz;
	gp_Pln aPlane(myView->Viewer()->PrivilegedPlane());

#ifdef OCC_PATCHED
	myView->Convert( Xs, Ys, Xv, Yv, Zv ); 
#else
	// The + 1 overcomes a fault in OCC, in "OpenGl_togl_unproject_raster.c",
	// which transforms the Y axis ordinate. The function uses the height of the
	// window, not the Y maximum which is (height - 1).
	myView->Convert( Xs, Ys + 1, Xv, Yv, Zv ); 
#endif 

	myView->Proj( Vx, Vy, Vz );
	gp_Lin aLine(gp_Pnt(Xv, Yv, Zv), gp_Dir(Vx, Vy, Vz));
	IntAna_IntConicQuad theIntersection( aLine, aPlane, Precision::Angular() );
	if (theIntersection.IsDone())
	{
		if (!theIntersection.IsParallel())
		{
			if (theIntersection.NbPoints() > 0)
			{
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


void TIGLViewerWidget::drawRubberBand( const QPoint origin, const QPoint position )
{
	if ( myRubberBand )
	{
		redraw();
		hideRubberBand();
		myRubberBand->setGeometry( QRect( origin, position ).normalized() );
		showRubberBand();
	}
}


void TIGLViewerWidget::showRubberBand( void )
{
	if ( myRubberBand )
	{
		myRubberBand->show();
	}
}


void TIGLViewerWidget::hideRubberBand( void )
{
	if ( myRubberBand )
	{
		myRubberBand->hide();
	}
}


int TIGLViewerWidget::paintCallBack (Aspect_Drawable /* drawable */,
                                  void* aPointer,
                                  Aspect_GraphicCallbackStruct* /* data */)
{
  TIGLViewerWidget *aWidget = (TIGLViewerWidget *) aPointer;
  aWidget->paintOCC();
  return 0;
}


// TODO: this routine prevents setting the background color
// either we should set it back here or we should skip it.
// Also, this code confuses the ftgl renderer introduced in
// OCC 6.4.0. Thus we should only use it with old OCC
void TIGLViewerWidget::paintOCC( void )
{
#ifndef OCC_BG
	glDisable( GL_LIGHTING ); 
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
	glPushMatrix();
    glLoadIdentity();

	GLfloat left   = -1.0f;
	GLfloat right  =  1.0f;
	GLfloat bottom = -1.0f;
	GLfloat top    =  1.0f;
	GLfloat depth  =  1.0f;

    glOrtho( left, right, bottom, top, 1.0, -1.0 );
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

#ifndef OCC_PATCHED
	glEnable(GL_BLEND);
	if (myView->ColorScaleIsDisplayed())
	{
		// Not needed on patched OCC 6.2 versions, but is the lowest
		// common denominator working code on collaborators OpenGL
		// graphics cards.
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}
#endif

    // purple gradient background
    glBegin( GL_QUADS);
    {
      glColor4f  (  0.1f, 0.1f, 0.1f, 1.0f );
      glVertex3d (  left, bottom, depth );
      glVertex3d ( right, bottom, depth );
      glColor4f  (  0.8f, 0.8f, 0.9f, 1.0f );
      glVertex3d ( right,    top, depth );
      glVertex3d (  left,    top, depth );
    }
    glEnd();

	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
#endif
}


Standard_Real TIGLViewerWidget::viewPrecision( bool resized )
{

	Standard_Real X1, Y1, Z1;
	Standard_Real X2, Y2, Z2;

	if (resized || myViewPrecision == 0.0)
	{
		myView->Convert( 0, 0, X1, Y1, Z1 ); 
		myView->Convert( 1, 0, X2, Y2, Z2 ); 
		Standard_Real pixWidth = X2 - X1;
		if ( pixWidth != 0.0 )
		{
			// Return the precision as the next highest decade above the pixel width
			myViewPrecision = std::pow (10.0, std::floor(std::log10( pixWidth ) + 1.0));
		}
		else
		{
			// Return the user precision if window not defined
			myViewPrecision = myPrecision; 
		}
	}
	return myViewPrecision;
}
