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
	statusBar()->showMessage(tr("Hello and welcome to TIGLViewer :)"));
	myOCC->viewAxo();
	myOCC->fitAll();
	watcher = new QFileSystemWatcher();
	watcher->addPath(filename);
	QObject::connect(watcher, SIGNAL(fileChanged(QString)), cpacsConfiguration, SLOT(updateConfiguration()));
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
			watcher = new QFileSystemWatcher();
			watcher->addPath(fileInfo.absoluteFilePath());
			QObject::connect(watcher, SIGNAL(fileChanged(QString)), cpacsConfiguration, SLOT(updateConfiguration()));
		}

		myLastFolder = fileInfo.absolutePath();
		reader.importModel ( fileInfo.absoluteFilePath(), format, myOCC->getContext() );
	}

	myOCC->viewAxo();
	myOCC->fitAll();
}


TIGLViewerWidget* TIGLViewerWindow::getMyOCC()
{
    return myOCC;
}


void TIGLViewerWindow::save()
{
	QString 	fileName;
	QString		fileType;
	QFileInfo	fileInfo;

	TIGLViewerInputOutput::FileFormat format;
	TIGLViewerInputOutput writer;

    statusBar()->showMessage(tr("Invoked File|Save"));

    fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), myLastFolder, tr("Geometry Export (*.iges *.brep *.step *.stl *.vrml)"));

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
		if (fileType.toLower() == tr("stl"))
		{
			format = TIGLViewerInputOutput::FormatSTL;
		}

		myLastFolder = fileInfo.absolutePath();
		writer.exportModel ( fileInfo.absoluteFilePath(), format, myOCC->getContext() );
	}
}

void TIGLViewerWindow::print()
{
	QMessageBox msgBox;
	QString text = 	"Printing is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

	statusBar()->showMessage(tr("Invoked File|Print"));

	msgBox.setWindowTitle("Undo not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::undo()
{
	QMessageBox msgBox;
	QString text = 	"Undo is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

	statusBar()->showMessage(tr("Invoked Edit|Undo"));

	msgBox.setWindowTitle("Undo not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::redo()
{
	QMessageBox msgBox;
	QString text = 	"Redo is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Redo"));

	msgBox.setWindowTitle("Redo not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::cut()
{
	QMessageBox msgBox;
	QString text = 	"Cut is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Cut"));

	msgBox.setWindowTitle("Cut not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::copy()
{
	QMessageBox msgBox;
	QString text = 	"Copy is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Copy"));

	msgBox.setWindowTitle("Copy not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::paste()
{
	QMessageBox msgBox;
	QString text = 	"Paste is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Paste"));

	msgBox.setWindowTitle("Paste not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();
}

void TIGLViewerWindow::about()
{
	QString text;
	QString tixiVersion(tixiGetVersion());
	QString tiglVersion(tiglGetVersion());

	text = 	"The <b>TIGLViewer</b> allows you to view CPACS geometries.<br> \
				   Copyright (C) 2007-2011 German Aerospace Center (DLR/SC) <br><br>";
	text += "TIXI version: " + tixiVersion + "<br>";
	text += "TIGL version: " + tiglVersion + "<br><br>";
	text += "Visit the TIGLViewer project page at <a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>";

    statusBar()->showMessage(tr("Invoked Help|About"));
    QMessageBox::about(this, tr("About Menu"), text);
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
	//statusBar()->showMessage(aString); // do not bother user with x,y,z crap
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


void TIGLViewerWindow::displayErrorMessage (const QString aMessage, QString aHeader = "TIGL Error")
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::information(this, aHeader, aMessage);
	if (reply == QMessageBox::Ok) {
		// Ok pressed
	}
	else {
		// Escape pressed
	}
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
	viewAxoAction->setShortcut(tr("Ctrl+D"));
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

	drawWingsAction = new QAction( tr("Show Wing"), this );
	drawWingsAction->setStatusTip(tr("Shows a wing."));
	connect(drawWingsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWing()));

	drawWingTriangulationAction = new QAction( tr("Show Wing triangulation"), this );
	drawWingTriangulationAction->setStatusTip(tr("Shows the triangulation of a wing."));
	connect(drawWingTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingTriangulation()));

	drawWingSamplePointsAction = new QAction( tr("Show Sample Wing points"), this );
	drawWingSamplePointsAction->setStatusTip(tr("Show sample points on a wing."));
	connect(drawWingSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingSamplePoints()));

	drawFusedWingAction = new QAction( tr("Show Fused Wing"), this );
	drawFusedWingAction->setStatusTip(tr("Shows a fused wing."));
	connect(drawFusedWingAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedWing()));

	drawWingComponentSegmentAction = new QAction( tr("Show Wing Component Segment"), this );
	drawWingComponentSegmentAction->setStatusTip(tr("Shows a wing component segment."));
	connect(drawWingComponentSegmentAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingComponentSegment()));


	// CPACS Aircraft Actions
	showAllWingsAndFuselagesAction = new QAction( tr("Show all Fuselages and Wings"), this );
	showAllWingsAndFuselagesAction->setStatusTip(tr("Show all Fuselages and Wings."));
	connect(showAllWingsAndFuselagesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWings()));

	showAllWingsAndFuselagesSurfacePointsAction = new QAction( tr("Show sample Surface points on Fuselages and Wings"), this );
	showAllWingsAndFuselagesSurfacePointsAction->setStatusTip(tr("Show sample Surface points on Fuselages and Wings."));
	connect(showAllWingsAndFuselagesSurfacePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWingsSurfacePoints()));

	drawFusedAircraftAction = new QAction( tr("Show the complete Aircraft fused"), this );
	drawFusedAircraftAction->setStatusTip(tr("Shows the complete Aircraft fused."));
	connect(drawFusedAircraftAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedAircraft()));

	drawWingFuselageIntersectionLineAction = new QAction( tr("Show Intersection Line Between Wing and Fuselage"), this );
	drawWingFuselageIntersectionLineAction->setStatusTip(tr("Show Intersection Line Between Wing and Fuselage"));
	connect(drawWingFuselageIntersectionLineAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingFuselageIntersectionLine()));


	// CPACS Fuselage Actions
	drawFuselageProfilesAction = new QAction( tr("Show Fuselage Profiles"), this );
	drawFuselageProfilesAction->setStatusTip(tr("Show the profiles of a fuselage."));
	connect(drawFuselageProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageProfiles()));

	drawFuselageAction = new QAction( tr("Show Fuselage"), this );
	drawFuselageAction->setStatusTip(tr("Shows a fuselage."));
	connect(drawFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselage()));

	drawFuselageTriangulationAction = new QAction( tr("Show Fuselage triangulation"), this );
	drawFuselageTriangulationAction->setStatusTip(tr("Shows the triangulation of a fuselage."));
	connect(drawFuselageTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageTriangulation()));

	drawFuselageSamplePointsAction = new QAction( tr("Show Sample Fuselage points"), this );
	drawFuselageSamplePointsAction->setStatusTip(tr("Show sample points on a fuselage."));
	connect(drawFuselageSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePoints()));

	drawFuselageSamplePointsAngleAction = new QAction( tr("Show Sample Fuselage point at 45°"), this );
	drawFuselageSamplePointsAngleAction->setStatusTip(tr("Show sample point on a fuselage at 45° and eta = 0.5"));
	connect(drawFuselageSamplePointsAngleAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePointsAngle()));

	drawFusedFuselageAction = new QAction( tr("Show Fused Fuselage"), this );
	drawFusedFuselageAction->setStatusTip(tr("Show a fused fuselage."));
	connect(drawFusedFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedFuselage()));



	// TIGL Actions
	tiglInterpolateBsplineWireAction = new QAction( tr("Use Bspline Interpolation"), this );
	tiglInterpolateBsplineWireAction->setStatusTip(tr("Use a BSpline interpolation between the points of a wire"));
	connect(tiglInterpolateBsplineWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglInterpolateBsplineWire()));

	tiglInterpolateLinearWireAction = new QAction( tr("Use Linear Interpolation"), this );
	tiglInterpolateLinearWireAction->setStatusTip(tr("Use a linear interpolation between the points of a wire"));
	connect(tiglInterpolateLinearWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglInterpolateLinearWire()));

	tiglApproximateBsplineWireAction = new QAction( tr("Use BSpline Approximation"), this );
	tiglApproximateBsplineWireAction->setStatusTip(tr("Use a BSpline approximation for the points of a wire"));
	connect(tiglApproximateBsplineWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglApproximateBsplineWireAction()));

	// Export functions
	tiglExportFusedIgesAction = new QAction( tr("Export Fused Model as IGES"), this );
	tiglExportFusedIgesAction->setStatusTip(tr("Export Fused Model as IGES"));
	connect(tiglExportFusedIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFusedAsIges()));

	tiglExportIgesAction = new QAction( tr("Export Model as IGES"), this );
	tiglExportIgesAction->setStatusTip(tr("Export Model as IGES"));
	connect(tiglExportIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportAsIges()));

	tiglExportMeshedWingSTL = new QAction( tr("Export meshed Wing as STL"), this );
	tiglExportMeshedWingSTL->setStatusTip(tr("Export meshed Wing as STL"));
	connect(tiglExportMeshedWingSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingSTL()));

	tiglExportMeshedFuselageSTL = new QAction( tr("Export meshed Fuselage as STL"), this );
	tiglExportMeshedFuselageSTL->setStatusTip(tr("Export meshed Fuselage as STL"));
	connect(tiglExportMeshedFuselageSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageSTL()));

	tiglExportMeshedWingVTK = new QAction( tr("Export meshed Wing as VTK"), this );
	tiglExportMeshedWingVTK->setStatusTip(tr("Export meshed Wing as VTK"));
	connect(tiglExportMeshedWingVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTK()));

	tiglExportMeshedWingVTKsimple = new QAction( tr("Export meshed Wing as simple VTK"), this );
	tiglExportMeshedWingVTKsimple->setStatusTip(tr("Export meshed Wing as simple VTK - no cpacs realted export"));
	connect(tiglExportMeshedWingVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTKsimple()));

	tiglExportMeshedFuselageVTK = new QAction( tr("Export meshed Fuselage as VTK"), this );
	tiglExportMeshedFuselageVTK->setStatusTip(tr("Export meshed Fuselage as VTK"));
	connect(tiglExportMeshedFuselageVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTK()));

	tiglExportMeshedFuselageVTKsimple = new QAction( tr("Export meshed Fuselage as simple VTK"), this );
	tiglExportMeshedFuselageVTKsimple->setStatusTip(tr("Export meshed Fuselage as simple VTK - no cpacs realted export"));
	connect(tiglExportMeshedFuselageVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTKsimple()));



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
		cpacsMenu->addAction( showAllWingsAndFuselagesAction );
		cpacsMenu->addAction( showAllWingsAndFuselagesSurfacePointsAction );
		cpacsMenu->addAction( drawFusedAircraftAction );
		cpacsMenu->addAction( drawWingFuselageIntersectionLineAction );
		cpacsMenu->addSeparator();
		cpacsMenu->addAction( drawWingsAction );
		cpacsMenu->addAction( drawWingProfilesAction );
		cpacsMenu->addAction( drawWingOverlayCPACSProfilePointsAction );
		cpacsMenu->addAction( drawWingTriangulationAction );
		cpacsMenu->addAction( drawWingSamplePointsAction );
		cpacsMenu->addAction( drawFusedWingAction );
		cpacsMenu->addAction( drawWingComponentSegmentAction );
		cpacsMenu->addSeparator();
		cpacsMenu->addAction( drawFuselageAction );
		cpacsMenu->addAction( drawFuselageProfilesAction );
		cpacsMenu->addAction( drawFuselageTriangulationAction );
		cpacsMenu->addAction( drawFuselageSamplePointsAction );
		cpacsMenu->addAction( drawFuselageSamplePointsAngleAction );
		cpacsMenu->addAction( drawFusedFuselageAction );


	// TIGL menu
	tiglMenu = menuBar()->addMenu( tr("&TIGL Methods") );
		tiglAlgorithmMenu = tiglMenu->addMenu( tr("Algorithm") );
			tiglAlgorithmMenu->addAction( tiglInterpolateBsplineWireAction );
			tiglAlgorithmMenu->addAction( tiglInterpolateLinearWireAction );
			tiglAlgorithmMenu->addAction( tiglApproximateBsplineWireAction );

		tiglExportMenu = tiglMenu->addMenu( tr("&Export using TIGL") );
			tiglExportMenu->addAction( tiglExportIgesAction );
			tiglExportMenu->addAction( tiglExportFusedIgesAction );
			tiglExportMenu->addAction( tiglExportMeshedWingSTL );
			tiglExportMenu->addAction( tiglExportMeshedFuselageSTL );
			tiglExportMenu->addAction( tiglExportMeshedWingVTK );
			tiglExportMenu->addAction( tiglExportMeshedWingVTKsimple );
			tiglExportMenu->addAction( tiglExportMeshedFuselageVTK );
			tiglExportMenu->addAction( tiglExportMeshedFuselageVTKsimple );



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


