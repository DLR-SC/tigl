/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerWidget.h 203 2012-09-25 08:47:55Z martinsiggel $
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

#ifndef TIGLVIEWERWIDGET_H
#define TIGLVIEWERWIDGET_H

#include <QtGui/QRubberBand>
#include <QtGui/QToolBar>

#include "TIGLViewer.h"


/** the key for multi selection */
#define MULTISELECTIONKEY  Qt::ShiftModifier   

/** The key for shortcut ( use to activate dynamic rotation, panning ) */
#define CASCADESHORTCUTKEY Qt::ControlModifier 

#define ValZWMin 1 /** For elastic bean selection */

class Handle_AIS_InteractiveContext;
class Handle_V3d_View;

class QOCC_DECLSPEC TIGLViewerWidget : public QWidget
{
	Q_OBJECT

public:

	enum CurrentAction3d {	CurAction3d_Undefined,
							CurAction3d_Nothing, 
							CurAction3d_Picking,
							CurAction3d_DynamicZooming,
							CurAction3d_WindowZooming, 
							CurAction3d_DynamicPanning,
							CurAction3d_GlobalPanning, 
							CurAction3d_DynamicRotation };
/*
	enum ViewAction {	ViewFitAllId, 
						ViewFitAreaId, 
						ViewZoomId, 
						ViewPanId, 
						ViewGlobalPanId,
						ViewFrontId, 
						ViewBackId, 
						ViewTopId, 
						ViewBottomId, 
						ViewLeftId, 
						ViewRightId,
						ViewAxoId, 
						ViewRotationId, 
						ViewResetId, 
						ViewHlrOffId, 
						ViewHlrOnId };
*/
public:
    TIGLViewerWidget(QWidget*);

	TIGLViewerWidget( const Handle_AIS_InteractiveContext& aContext = NULL,
					 QWidget *parent = NULL, 
					 Qt::WindowFlags wflags = 0 );

    ~TIGLViewerWidget();

    void setContext(const Handle_AIS_InteractiveContext& aContext){ myContext = aContext; }

	void initializeOCC(const Handle_AIS_InteractiveContext& aContext = NULL);

	Handle_AIS_InteractiveContext	getContext( void ) { return myContext; }
	Handle_V3d_View					getView( void )    { return myView; }

	//Overrides
	QPaintEngine* paintEngine() const;
	QToolBar*	  myToolBar;

	void redraw( bool isPainting = false );

signals:

	void initialized();
	void selectionChanged();
	void mouseMoved   ( V3d_Coordinate X, V3d_Coordinate Y, V3d_Coordinate Z );
	void pointClicked ( V3d_Coordinate X, V3d_Coordinate Y, V3d_Coordinate Z );
	void sendStatus   ( const QString aMessage );

	void error ( int errorCode, QString& errorDescription );

public slots:

	void idle();
	void fitExtents();
	void fitAll();
    void fitArea();
    void zoom();
    void zoomIn();
    void zoomOut();
    void pan();
    void globalPan();
    void rotation();
    void hiddenLineOn();
    void hiddenLineOff();
	void background();
	void viewFront();
    void viewBack();
    void viewTop();
    void viewBottom();
    void viewLeft();
    void viewRight();
    void viewAxo();
	void viewTopFront();
	void viewGrid();
	void viewReset();
	void setReset();
	void eraseSelected();
    void setTransparency();
    void setTransparency(int);
    void setObjectsWireframe();
    void setObjectsShading();
    void setObjectsColor();
    void setObjectsMaterial();

protected: // methods

    virtual void paintEvent        ( QPaintEvent* e );
    virtual void resizeEvent       ( QResizeEvent* e );
    virtual void mousePressEvent   ( QMouseEvent* e );
    virtual void mouseReleaseEvent ( QMouseEvent* e );
    virtual void mouseMoveEvent    ( QMouseEvent* e );
	virtual void wheelEvent        ( QWheelEvent* e );

	virtual void leaveEvent		   ( QEvent * );

private: // members

#ifdef WNT
    Handle_WNT_Window				myWindow;
#else
    Handle_Xw_Window				myWindow;
#endif // WNT

	Handle_V3d_View                 myView;
	Handle_V3d_Viewer               myViewer;
	Handle_AIS_InteractiveContext   myContext;
					
	Standard_Boolean				myViewResized;
	Standard_Boolean				myViewInitialized;
    CurrentAction3d                 myMode;
    Quantity_Factor                 myCurZoom;
    Standard_Boolean				myGridSnap;
	AIS_StatusOfDetection			myDetection;

	V3d_Coordinate					myV3dX,   
									myV3dY,   
									myV3dZ;
		
	QRubberBand*					myRubberBand;
	QPoint							myStartPoint;
	QPoint							myCurrentPoint;
	
	Standard_Real					myPrecision;
	Standard_Real					myViewPrecision;
	Standard_Boolean				myMapIsValid;
	Qt::KeyboardModifiers			myKeyboardFlags;
	Qt::MouseButton					myButtonFlags;
	QCursor							myCrossCursor;

private: // methods
    void initialize();

	void onLeftButtonDown  ( Qt::KeyboardModifiers nFlags, const QPoint point );
    void onMiddleButtonDown( Qt::KeyboardModifiers nFlags, const QPoint point );
    void onRightButtonDown ( Qt::KeyboardModifiers nFlags, const QPoint point );
    void onLeftButtonUp    ( Qt::KeyboardModifiers nFlags, const QPoint point );
    void onMiddleButtonUp  ( Qt::KeyboardModifiers nFlags, const QPoint point );
    void onRightButtonUp   ( Qt::KeyboardModifiers nFlags, const QPoint point );
    
	void onMouseMove  ( Qt::MouseButtons buttons, 
						Qt::KeyboardModifiers nFlags, const QPoint point );

    AIS_StatusOfPick		dragEvent ( const QPoint startPoint, const QPoint endPoint, const bool multi = false );
    AIS_StatusOfPick		inputEvent( const bool multi = false );
    AIS_StatusOfDetection	moveEvent ( const QPoint point );
    
    void setMode( const CurrentAction3d mode );
	
	Standard_Real precision( Standard_Real aReal );
	Standard_Real viewPrecision( bool resized = false );

	void drawRubberBand( const QPoint origin, const QPoint position );
	void showRubberBand( void );
	void hideRubberBand( void );

	Standard_Boolean convertToPlane(const Standard_Integer Xs, 
									const Standard_Integer Ys, 
										  Standard_Real& X,
										  Standard_Real& Y,
										  Standard_Real& Z);
										  
	void paintOCC();
	static int paintCallBack (Aspect_Drawable, 
							  void*, 
							  Aspect_GraphicCallbackStruct*);

public:

	bool dump(Standard_CString theFile);

};

#endif // TIGLVIEWERWIDGET_H
