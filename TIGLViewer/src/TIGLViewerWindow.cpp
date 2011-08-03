/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <QtGui/QtGui>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopoDS_Vertex.hxx>
#include <Handle_AIS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Aspect_RectangularGrid.hxx>
#include <V3d_View.hxx>

#include "TIGLViewerWindow.h"
#include "CommandLineParameters.h"

void ShowOrigin ( Handle_AIS_InteractiveContext theContext );
void AddVertex  ( double x, double y, double z, Handle_AIS_InteractiveContext theContext );


void AddVertex (double x, double y, double z, Handle_AIS_InteractiveContext theContext)
{
	TopoDS_Vertex aVertex=BRepBuilderAPI_MakeVertex( gp_Pnt(x,y,z) );
	Handle(AIS_Shape) AISVertex = new AIS_Shape(aVertex);
	// context is the handle to an AIS_InteractiveContext object.
	theContext->Display(AISVertex);
}

void ShowOrigin ( Handle_AIS_InteractiveContext theContext )
{
	AddVertex ( 0.0, 0.0, 0.0, theContext);
}


TIGLViewerWindow::TIGLViewerWindow()
	: myLastFolder(tr(""))
{
    myVC  = new TIGLViewerContext();
	myOCC = new TIGLViewerWidget(myVC->getContext(), this);
	this->setCentralWidget(myOCC);

	cpacsConfiguration = new TIGLViewerDocument(this, myOCC->getContext());

    createActions();
    createMenus();

    statusBar()->showMessage(tr("A context menu is available by right-clicking"));

    setWindowTitle(tr(PARAMS.windowTitle.toAscii().data()));
    setMinimumSize(160, 160);
	showMaximized();
}


void TIGLViewerWindow::setInitialCpacsFileName(QString filename)
{
	cpacsFileName = filename;
	cpacsConfiguration->openCpacsConfiguration(filename);
}


void TIGLViewerWindow::newFile()
{
    statusBar()->showMessage(tr("Invoked File|New"));
	//myOCC->getView()->ColorScaleErase();
	myVC->deleteAllObjects();
}

void TIGLViewerWindow::open()
{
	QString		fileName;
	QString		fileType;
	QFileInfo	fileInfo;

	TIGLViewerInputOutput::FileFormat format;
	TIGLViewerInputOutput reader;

    statusBar()->showMessage(tr("Invoked File|Open"));

	fileName = QFileDialog::getOpenFileName (	this,
				  								tr("Open File"),
												myLastFolder,
												tr( "CPACS (*.xml);;"
													"Other drawing types (*.brep *.rle *.igs *iges *.stp *.step);;"
													"BREP (*.brep *.rle);;"
													"STEP (*.step *.stp);;"
													"IGES (*.iges *.igs)" ) );
	if (!fileName.isEmpty())
	{
		fileInfo.setFile(fileName);
		fileType = fileInfo.suffix();
		if (fileType.toLower() == tr("brep") || fileType.toLower() == tr("rle"))
		{
			format = TIGLViewerInputOutput::FormatBREP;
		}
		if (fileType.toLower() == tr("step") || fileType.toLower() == tr("stp"))
		{
			format = TIGLViewerInputOutput::FormatSTEP;
		}
		if (fileType.toLower() == tr("iges") || fileType.toLower() == tr("igs"))
		{
			format = TIGLViewerInputOutput::FormatIGES;
		}
		if (fileType.toLower() == tr("xml"))
		{
			cpacsConfiguration->openCpacsConfiguration(fileInfo.absoluteFilePath());
		}

		myLastFolder = fileInfo.absolutePath();
		reader.importModel ( fileInfo.absoluteFilePath(), format, myOCC->getContext() );
	}

	myOCC->fitAll();
}


TIGLViewerWidget* TIGLViewerWindow::getMyOCC()
{
    return myOCC;
}


void TIGLViewerWindow::save()
{
    statusBar()->showMessage(tr("Invoked File|Save"));
}

void TIGLViewerWindow::print()
{
    statusBar()->showMessage(tr("Invoked File|Print"));
}

void TIGLViewerWindow::undo()
{
    statusBar()->showMessage(tr("Invoked Edit|Undo"));
}

void TIGLViewerWindow::redo()
{
    statusBar()->showMessage(tr("Invoked Edit|Redo"));
}

void TIGLViewerWindow::cut()
{
    statusBar()->showMessage(tr("Invoked Edit|Cut"));
}

void TIGLViewerWindow::copy()
{
    statusBar()->showMessage(tr("Invoked Edit|Copy"));
}

void TIGLViewerWindow::paste()
{
    statusBar()->showMessage(tr("Invoked Edit|Paste"));
}

void TIGLViewerWindow::about()
{
	char text[200];
	sprintf(text, "The <b>TIGLViewer</b> allows you to view CPACS geometries.<br> \
				   Copyright (C) 2007-2011 German Aerospace Center (DLR/SC) <br><br> \
				   TIXI version: %s <br> \
				   TIGL version: %s", tixiGetVersion(), tiglGetVersion());
	QString tixiVersion(tixiGetVersion());
    statusBar()->showMessage(tr("Invoked Help|About"));
    QMessageBox::about(this, tr("About Menu"), tr(text));
}

void TIGLViewerWindow::aboutQt()
{
    statusBar()->showMessage(tr("Invoked Help|About Qt"));
}


void TIGLViewerWindow::xyzPosition (V3d_Coordinate X,
							  V3d_Coordinate Y,
							  V3d_Coordinate Z)
{
	QString aString;
	QTextStream ts(&aString);
	ts << X << "," << Y << "," << Z;
	statusBar()->showMessage(aString);
}

void TIGLViewerWindow::addPoint (V3d_Coordinate X,
						   V3d_Coordinate Y,
						   V3d_Coordinate Z)
{
	AddVertex ( X, Y, Z, myVC->getContext() );
}

void TIGLViewerWindow::statusMessage (const QString aMessage)
{
	statusBar()->showMessage(aMessage);
}


void TIGLViewerWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(tr("Ctrl+N"));
    newAction->setStatusTip(tr("Create a new file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(tr("&Open..."), this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    printAction = new QAction(tr("&Print..."), this);
    printAction->setShortcut(tr("Ctrl+P"));
    printAction->setStatusTip(tr("Print the document"));
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));

    exitAction = new QAction(tr("Exit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcut(tr("Ctrl+Z"));
    undoAction->setStatusTip(tr("Undo the last operation"));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));

    redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcut(tr("Ctrl+Y"));
    redoAction->setStatusTip(tr("Redo the last operation"));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));


    // our slots
    fitAction = new QAction(tr("&Fit Window"), this);
	fitAction->setShortcut(tr("Ctrl+F"));
    fitAction->setStatusTip(tr("Fit to window"));
    connect(fitAction, SIGNAL(triggered()), myOCC, SLOT(fitExtents()));

	fitAllAction = new QAction(tr("Fit &All"), this);
	fitAllAction->setShortcut(tr("Ctrl+A"));
    fitAllAction->setStatusTip(tr("Fit contents to viewport"));
    connect(fitAllAction, SIGNAL(triggered()), myOCC, SLOT(fitAll()));

	zoomAction = new QAction(tr("&Zoom"), this);
	zoomAction->setStatusTip(tr("Zoom in window"));
    connect(zoomAction, SIGNAL(triggered()), myOCC, SLOT(fitArea()));

	panAction = new QAction(tr("&Pan"), this);
    panAction->setStatusTip(tr("Window panning"));
    connect(panAction, SIGNAL(triggered()), myOCC, SLOT(pan()));

	rotAction = new QAction(tr("&Rotate"), this);
	rotAction->setShortcut(tr("Ctrl+R"));
    rotAction->setStatusTip(tr("Window rotation"));
    connect(rotAction, SIGNAL(triggered()), myOCC, SLOT(rotation()));

	gridOnAction = new QAction(tr("&Grid On"), this);
	gridOnAction->setShortcut(tr("Ctrl+G"));
    gridOnAction->setStatusTip(tr("Turn the grid on"));
    connect(gridOnAction, SIGNAL(triggered()), myVC, SLOT(gridOn()));

	gridOffAction = new QAction(tr("Gri&d Off"), this);
	gridOffAction->setShortcut(tr("Ctrl+D"));
    gridOffAction->setStatusTip(tr("Turn the grid on"));
    connect(gridOffAction, SIGNAL(triggered()), myVC, SLOT(gridOff()));

	gridXYAction = new QAction(tr("XY Grid"), this);
    gridXYAction->setStatusTip(tr("Grid on XY Plane"));
	gridOffAction->setShortcut(tr("Ctrl+F"));
    connect(gridXYAction, SIGNAL(triggered()), myVC, SLOT(gridXY()));

	gridXZAction = new QAction(tr("XZ Grid"), this);
    gridXZAction->setStatusTip(tr("Grid on XZ Plane"));
	gridXZAction->setShortcut(tr("Ctrl+Y"));
    connect(gridXZAction, SIGNAL(triggered()), myVC, SLOT(gridXZ()));

	gridYZAction = new QAction(tr("YZ Grid"), this);
	gridYZAction->setStatusTip(tr("Grid on YZ Plane"));
	gridOffAction->setShortcut(tr("Ctrl+G"));
    connect(gridYZAction, SIGNAL(triggered()), myVC, SLOT(gridYZ()));

	gridRectAction = new QAction(tr("Rectangular"), this);
	gridRectAction->setStatusTip(tr("Retangular grid"));
	gridOffAction->setShortcut(tr("Ctrl+H"));
    connect(gridRectAction, SIGNAL(triggered()), myVC, SLOT(gridRect()));

	gridCircAction = new QAction(tr("Circular"), this);
	gridCircAction->setStatusTip(tr("Circular grid"));
	gridOffAction->setShortcut(tr("Ctrl+J"));
    connect(gridCircAction, SIGNAL(triggered()), myVC, SLOT(gridCirc()));

	// Standard View
	viewFrontAction = new QAction(tr("Front"), this);
	viewFrontAction->setStatusTip(tr("View From Front"));
    connect(viewFrontAction, SIGNAL(triggered()), myOCC, SLOT(viewFront()));

	viewBackAction = new QAction(tr("Back"), this);
	viewBackAction->setStatusTip(tr("View From Back"));
    connect(viewBackAction, SIGNAL(triggered()), myOCC, SLOT(viewBack()));

	viewTopAction = new QAction(tr("Top"), this);
	viewTopAction->setStatusTip(tr("View From Top"));
    connect(viewTopAction, SIGNAL(triggered()), myOCC, SLOT(viewTop()));

	viewBottomAction = new QAction(tr("Bottom"), this);
	viewBottomAction->setStatusTip(tr("View From Bottom"));
    connect(viewBottomAction, SIGNAL(triggered()), myOCC, SLOT(viewBottom()));

	viewLeftAction = new QAction(tr("Left"), this);
	viewLeftAction->setStatusTip(tr("View From Left"));
    connect(viewLeftAction, SIGNAL(triggered()), myOCC, SLOT(viewLeft()));

	viewRightAction = new QAction(tr("Right"), this);
	viewRightAction->setStatusTip(tr("View From Right"));
    connect(viewRightAction, SIGNAL(triggered()), myOCC, SLOT(viewRight()));

	viewAxoAction = new QAction(tr("Axonometric"), this);
	viewAxoAction->setStatusTip(tr("Axonometric view"));
    connect(viewAxoAction, SIGNAL(triggered()), myOCC, SLOT(viewAxo()));

	viewGridAction = new QAction(tr("Grid"), this);
	viewGridAction->setStatusTip(tr("View from grid"));
    connect(viewGridAction, SIGNAL(triggered()), myOCC, SLOT(viewGrid()));

	viewResetAction = new QAction(tr("Reset"), this);
	viewResetAction->setStatusTip(tr("Reset the view"));
    connect(viewResetAction, SIGNAL(triggered()), myOCC, SLOT(viewReset()));

	backgroundAction = new QAction( tr("&Background"), this );
	backgroundAction->setStatusTip(tr("Change the background colour"));
	connect(backgroundAction, SIGNAL(triggered()), myOCC, SLOT(background()));


	// CPACS Wing Actions
	drawWingProfilesAction = new QAction( tr("Show Wing Profiles"), this );
	drawWingProfilesAction->setStatusTip(tr("Show the profiles of a wing."));
	connect(drawWingProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingProfiles()));

	drawWingOverlayCPACSProfilePointsAction = new QAction( tr("Show Wing overlay profile points"), this );
	drawWingOverlayCPACSProfilePointsAction->setStatusTip(tr("Show the profiles point of a wing profile."));
	connect(drawWingOverlayCPACSProfilePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingOverlayProfilePoints()));

	drawWingsAction = new QAction( tr("Show Wings"), this );
	drawWingsAction->setStatusTip(tr("Shows a selected wing."));
	connect(drawWingsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWing()));

	drawWingTriangulationAction = new QAction( tr("Show Wing triangulation"), this );
	drawWingTriangulationAction->setStatusTip(tr("Shows the triangulation of a wing."));
	connect(drawWingTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingTriangulation()));


	drawWingSamplePointsAction = new QAction( tr("Show Sample Wing points"), this );
	drawWingSamplePointsAction->setStatusTip(tr("Show sample points on a wing."));
	connect(drawWingSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingSamplePoints()));



	// CPACS Fuselage Actions
	drawFuselageProfilesAction = new QAction( tr("Show Fuselage Profiles"), this );
	drawFuselageProfilesAction->setStatusTip(tr("Show the profiles of a fuselage."));
	connect(drawFuselageProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageProfiles()));

	drawFuselageAction = new QAction( tr("Show Fuselage"), this );
	drawFuselageAction->setStatusTip(tr("Show a fuselage."));
	connect(drawFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselage()));

	drawFuselageTriangulationAction = new QAction( tr("Show Fuselage triangulation"), this );
	drawFuselageTriangulationAction->setStatusTip(tr("Shows the triangulation of a fuselage."));
	connect(drawFuselageTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageTriangulation()));



	// The co-ordinates from the view
	connect( myOCC, SIGNAL(mouseMoved(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
		     this,   SLOT(xyzPosition(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

	// Add a point from the view
	connect( myOCC, SIGNAL(pointClicked(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
		     this,   SLOT (addPoint    (V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

	connect( myOCC, SIGNAL(sendStatus(const QString)),
		     this,  SLOT  (statusMessage(const QString)) );

}

void TIGLViewerWindow::createMenus()
{
    fileMenu = menuBar()->addMenu( tr("&File") );
		fileMenu->addAction( newAction );
		fileMenu->addAction( openAction );
		fileMenu->addAction( saveAction );
		fileMenu->addAction( printAction );

		fileMenu->addSeparator();
		fileMenu->addAction( exitAction );

   	editMenu = menuBar()->addMenu( tr("&Edit") );
		editMenu->addAction( undoAction );
		editMenu->addAction( redoAction );
		editMenu->addSeparator();
		editMenu->addAction( cutAction );
		editMenu->addAction( copyAction );
		editMenu->addAction( pasteAction );

		// CPACS menu
	cpacsMenu = menuBar()->addMenu( tr("&CPACS") );
		cpacsWingMenu = cpacsMenu->addMenu( tr("&Wing Methods") );
			cpacsWingMenu->addAction( drawWingsAction );
			cpacsWingMenu->addAction( drawWingProfilesAction );
			cpacsWingMenu->addAction( drawWingOverlayCPACSProfilePointsAction );
			cpacsWingMenu->addAction( drawWingTriangulationAction );
			cpacsWingMenu->addAction( drawWingSamplePointsAction );

		cpacsFuselageMenu = cpacsMenu->addMenu( tr("&Fuselage Methods") );
			cpacsFuselageMenu->addAction( drawFuselageProfilesAction );
			cpacsFuselageMenu->addAction( drawFuselageAction );
			cpacsFuselageMenu->addAction( drawFuselageTriangulationAction );



	viewMenu = menuBar()->addMenu( tr("&View") );
		viewDisplayMenu = viewMenu->addMenu( tr("&Display") );
			viewDisplayMenu->addAction( viewFrontAction );
			viewDisplayMenu->addAction( viewBackAction );
			viewDisplayMenu->addAction( viewTopAction );
			viewDisplayMenu->addAction( viewBottomAction );
			viewDisplayMenu->addAction( viewLeftAction );
			viewDisplayMenu->addAction( viewRightAction );
			viewDisplayMenu->addSeparator();
			viewDisplayMenu->addAction( viewAxoAction );
			viewDisplayMenu->addAction( viewGridAction );
			viewDisplayMenu->addSeparator();
			viewDisplayMenu->addAction( viewResetAction );
			viewDisplayMenu->addSeparator();
			viewDisplayMenu->addAction( backgroundAction );

		viewActionsMenu = viewMenu->addMenu( tr("&Actions") );
			viewActionsMenu->addAction( fitAction );
			viewActionsMenu->addAction( fitAllAction );
			viewActionsMenu->addAction( zoomAction );
			viewActionsMenu->addAction( panAction );
			viewActionsMenu->addAction( rotAction );

		gridMenu = viewMenu->addMenu( tr("&Grid") );
			gridMenu->addAction( gridOnAction );
			gridMenu->addAction( gridOffAction );
			gridMenu->addSeparator();
			gridMenu->addAction( gridXYAction );
			gridMenu->addAction( gridXZAction );
			gridMenu->addAction( gridYZAction );
			gridMenu->addSeparator();
			gridMenu->addAction( gridRectAction );
			gridMenu->addAction( gridCircAction );

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}
