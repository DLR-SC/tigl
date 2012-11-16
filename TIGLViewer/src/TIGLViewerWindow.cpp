/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerWindow.cpp 214 2012-10-16 09:20:45Z martinsiggel $
*
* Version: $Revision: 214 $
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

#include <QtGui/QtGui>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QShortcut>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopoDS_Vertex.hxx>
#include <Handle_AIS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Aspect_RectangularGrid.hxx>
#include <V3d_View.hxx>
#include <Standard_Version.hxx>

#include "TIGLViewerWindow.h"
#include "TIGLDebugStream.h"
#include "TIGLScriptEngine.h"
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

void TIGLViewerWindow::contextMenuEvent(QContextMenuEvent *event)
 {
     QMenu menu(this);

     bool OneOrMoreIsSelected = false;
     for (myVC->getContext()->InitCurrent(); myVC->getContext()->MoreCurrent (); myVC->getContext()->NextCurrent ())
         if (myVC->getContext()->IsDisplayed(myVC->getContext()->Current())) OneOrMoreIsSelected=true;

     if(OneOrMoreIsSelected) {
        QAction *eraseAct;
        eraseAct = new QAction(tr("&Erase"), this);
        eraseAct->setStatusTip(tr("Erase selected components"));
        menu.addAction(eraseAct);
        connect(eraseAct, SIGNAL(triggered()), myOCC, SLOT(eraseSelected()));

        QAction *transparencyAct;
        transparencyAct = new QAction(tr("&Transparency"), this);
        transparencyAct->setStatusTip(tr("Component Transparency"));
        menu.addAction(transparencyAct);
        connect(transparencyAct, SIGNAL(triggered()), myOCC, SLOT(setTransparency()));

        QAction *wireframeAct;
        wireframeAct = new QAction(tr("&Wireframe"), this);
        wireframeAct->setStatusTip(tr("Component Wireframe"));
        menu.addAction(wireframeAct);
        connect(wireframeAct, SIGNAL(triggered()), myOCC, SLOT(setObjectsWireframe()));

        QAction *shadingAct;
        shadingAct = new QAction(tr("&Shading"), this);
        shadingAct->setStatusTip(tr("Component Shading"));
        menu.addAction(shadingAct);
        connect(shadingAct, SIGNAL(triggered()), myOCC, SLOT(setObjectsShading()));

        QAction *colorAct;
        colorAct = new QAction(tr("&Color"), this);
        colorAct->setStatusTip(tr("Component Color"));
        menu.addAction(colorAct);
        connect(colorAct, SIGNAL(triggered()), myOCC, SLOT(setObjectsColor()));

        QAction *materialAct;
        materialAct = new QAction(tr("&Material"), this);
        materialAct->setStatusTip(tr("Component Material"));
        menu.addAction(materialAct);
        connect(materialAct, SIGNAL(triggered()), myOCC, SLOT(setObjectsMaterial()));
     }

     menu.exec(event->globalPos());
 }

TIGLViewerWindow::TIGLViewerWindow()
	: myLastFolder(tr(""))
{
    setupUi(this);
    myVC  = new TIGLViewerContext();
    myOCC->setContext(myVC->getContext());
    Handle(AIS_InteractiveContext) context = myVC->getContext();


    //redirect everything to TIGL console, let error messages be printed in red
    stdoutStream = new QDebugStream(std::cout);
    errorStream  = new QDebugStream(std::cerr);
    errorStream->setMarkup("<b><font color=\"red\">Error:","</font></b>");
    connect(stdoutStream, SIGNAL(sendString(QString)), console, SLOT(append(QString)));
    connect(errorStream , SIGNAL(sendString(QString)), console, SLOT(append(QString)));

    QPalette p = console->palette();
    p.setColor(QPalette::Base, Qt::black);
    console->setPalette(p);
    console->setTextColor(Qt::green);
    console->append("TIGLViewer console output\n\n");

    cpacsConfiguration = new TIGLViewerDocument(this, myOCC->getContext());

    createActions();
    createMenus();
    updateMenus(-1);

    statusBar()->showMessage(tr("A context menu is available by right-clicking"));

    setWindowTitle(tr(PARAMS.windowTitle.toAscii().data()));
    setMinimumSize(160, 160);
    showMaximized();
    
    scriptEngine = new TIGLScriptEngine(scriptInput);
    QObject::connect(scriptInput, SIGNAL(textChanged(QString)), scriptEngine, SLOT(textChanged(QString)));
    QObject::connect(scriptInput, SIGNAL(returnPressed()), scriptEngine, SLOT(eval()));
    QObject::connect(scriptEngine, SIGNAL(printResults(QString)), console, SLOT(append(QString)));
}

TIGLViewerWindow::~TIGLViewerWindow(){
    delete stdoutStream;
    delete errorStream;
    delete scriptEngine;
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

    statusBar()->showMessage(tr("Invoked File|Open"));

	fileName = QFileDialog::getOpenFileName (	this,
				  								tr("Open File"),
												myLastFolder,
												tr( "CPACS (*.xml);;"
													"Other drawing types (*.brep *.rle *.igs *iges *.stp *.step);;"
													"BREP (*.brep *.rle);;"
													"STEP (*.step *.stp);;"
													"IGES (*.iges *.igs);;"
													"STL  (*.stl)" ) );
    openFile(fileName);
}

void TIGLViewerWindow::closeConfiguration(){
    cpacsConfiguration->closeCpacsConfiguration();
}

void TIGLViewerWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}

void TIGLViewerWindow::openFile(const QString& fileName)
{
    QString		fileType;
    QFileInfo	fileInfo;

    TIGLViewerInputOutput::FileFormat format;
    TIGLViewerInputOutput reader;

    statusBar()->showMessage(tr("Invoked File|Open"));

    if (!fileName.isEmpty())
    {
        fileInfo.setFile(fileName);
        fileType = fileInfo.suffix();
        
        if (fileType.toLower() == tr("xml"))
        {
            cpacsConfiguration->openCpacsConfiguration(fileInfo.absoluteFilePath());
            updateMenus(cpacsConfiguration->getCpacsHandle());
            watcher = new QFileSystemWatcher();
            watcher->addPath(fileInfo.absoluteFilePath());
            QObject::connect(watcher, SIGNAL(fileChanged(QString)), cpacsConfiguration, SLOT(updateConfiguration()));
        }
        else {

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

            reader.importModel ( fileInfo.absoluteFilePath(), format, myOCC->getContext() );
        }
    }
    myLastFolder = fileInfo.absolutePath();
    setCurrentFile(fileName);

    myOCC->viewAxo();
    myOCC->fitAll();
}

void TIGLViewerWindow::setCurrentFile(const QString &fileName)
{
    setWindowFilePath(fileName);

    QSettings settings("DLR SC-VK","TIGLViewer");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
    settings.setValue("lastFolder", myLastFolder);

    updateRecentFileActions();
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


void TIGLViewerWindow::setBackgroundImage()
{
	QString		fileName;
	QString		fileType;
	QFileInfo	fileInfo;

	TIGLViewerInputOutput::FileFormat format;
	TIGLViewerInputOutput reader;

    statusBar()->showMessage(tr("Invoked File|Load Background Image"));

	QMessageBox msgBox;
	QString text = 	"Undo is not yet implemented!<br>Go to TIGLViewer project page \
					(<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

	msgBox.setWindowTitle("Background pictures not yet implemented!");
	msgBox.setText(text);
	msgBox.exec();


	//fileName = QFileDialog::getOpenFileName (	this,
	//			  								tr("Open Background Image"),
	//											myLastFolder,
	//											tr( "IMAGES (*.jpg *.png *.bmp);;" ) );
	//if (!fileName.isEmpty())
	//{
	//	fileInfo.setFile(fileName);
	//	fileType = fileInfo.suffix();
	//	
	//	if (fileType.toLower() == tr("bmp"))
	//	{
			//Standard_CString FileName("C:\\DLR\\TIVA\\TIGL\\TIGLViewer\\x.bmp");
			//Aspect_FillMethod FillStyle(Aspect_FM_CENTERED);//Aspect_FM_STRETCH //Aspect_FM_CENTERED
			//Standard_Boolean update(Standard_True);
			//myOCC->getView()->SetBackgroundImage(FileName, FillStyle, update);
			//myOCC->redraw();
	//	}

	//	myLastFolder = fileInfo.absolutePath();
	//}
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
	QString occtVersion = QString("%1.%2.%3").arg(OCC_VERSION_MAJOR).arg(OCC_VERSION_MINOR).arg(OCC_VERSION_MAINTENANCE);

	text = 	"The <b>TIGLViewer</b> allows you to view CPACS geometries.<br> \
				   Copyright (C) 2007-2011 German Aerospace Center (DLR/SC) <br><br>";
	text += "TIXI version: " + tixiVersion + "<br>";
	text += "TIGL version: " + tiglVersion + "<br>";
	text += "OpenCascade version: " + occtVersion + "<br><br>";
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
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(closeConfiguration()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(printAction, SIGNAL(triggered()), this, SLOT(print()));
    connect(setBackgroundAction, SIGNAL(triggered()), this, SLOT(setBackgroundImage()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));

    // view->actions menu
    connect(fitAction, SIGNAL(triggered()), myOCC, SLOT(fitExtents()));
    connect(fitAllAction, SIGNAL(triggered()), myOCC, SLOT(fitAll()));
    //connect(zoomAction, SIGNAL(triggered()), myOCC, SLOT(fitArea()));
    connect(zoomAction, SIGNAL(triggered()),myOCC, SLOT(zoom()));
    connect(panAction, SIGNAL(triggered()), myOCC, SLOT(pan()));
    connect(rotAction, SIGNAL(triggered()), myOCC, SLOT(rotation()));

    // view->grid menu
    connect(gridOnAction, SIGNAL(toggled(bool)), myVC, SLOT(toggleGrid(bool)));
    connect(gridXYAction, SIGNAL(triggered()), myVC, SLOT(gridXY()));
    connect(gridXZAction, SIGNAL(triggered()), myVC, SLOT(gridXZ()));
    connect(gridYZAction, SIGNAL(triggered()), myVC, SLOT(gridYZ()));
    connect(gridRectAction, SIGNAL(triggered()), myVC, SLOT(gridRect()));
    connect(gridCircAction, SIGNAL(triggered()), myVC, SLOT(gridCirc()));

    // Standard View
    connect(viewFrontAction, SIGNAL(triggered()), myOCC, SLOT(viewFront()));
    connect(viewBackAction, SIGNAL(triggered()), myOCC, SLOT(viewBack()));
    connect(viewTopAction, SIGNAL(triggered()), myOCC, SLOT(viewTop()));
    connect(viewBottomAction, SIGNAL(triggered()), myOCC, SLOT(viewBottom()));
    connect(viewLeftAction, SIGNAL(triggered()), myOCC, SLOT(viewLeft()));
    connect(viewRightAction, SIGNAL(triggered()), myOCC, SLOT(viewRight()));
    connect(viewAxoAction, SIGNAL(triggered()), myOCC, SLOT(viewAxo()));
    connect(viewGridAction, SIGNAL(triggered()), myOCC, SLOT(viewGrid()));
    connect(viewResetAction, SIGNAL(triggered()), myOCC, SLOT(viewReset()));
    connect(viewZoomInAction, SIGNAL(triggered()), myOCC, SLOT(zoomIn()));
    connect(viewZoomOutAction, SIGNAL(triggered()), myOCC, SLOT(zoomOut()));
	connect(backgroundAction, SIGNAL(triggered()), myOCC, SLOT(background()));
    connect(showConsoleAction, SIGNAL(toggled(bool)), console, SLOT(setVisible(bool)));
    connect(showScriptAction, SIGNAL(toggled(bool)), scriptInput, SLOT(setVisible(bool)));
    connect(showWireframeAction, SIGNAL(toggled(bool)), myVC, SLOT(wireFrame(bool)));


	// CPACS Wing Actions
	connect(drawWingProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingProfiles()));
	connect(drawWingOverlayCPACSProfilePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingOverlayProfilePoints()));
	connect(drawWingsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWing()));
	connect(drawWingTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingTriangulation()));
	connect(drawWingSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingSamplePoints()));
	connect(drawFusedWingAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedWing()));
	connect(drawWingComponentSegmentAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingComponentSegment()));


	// CPACS Aircraft Actions
	connect(showAllWingsAndFuselagesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWings()));
	connect(showAllWingsAndFuselagesSurfacePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWingsSurfacePoints()));
	connect(drawFusedAircraftAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedAircraft()));
	connect(drawWingFuselageLineAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingFuselageIntersectionLine()));
	connect(showFusedAirplaneTriangulation, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedAircraftTriangulation()));

    /*drawWingFuselageIntersectionLineAction->setStatusTip(tr("Show Intersection Line Between Wing and Fuselage"));
    connect(drawWingFuselageIntersectionLineAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingFuselageIntersectionLine()));*/


	// CPACS Fuselage Actions
	connect(drawFuselageProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageProfiles()));
	connect(drawFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselage()));
	connect(drawFuselageTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageTriangulation()));
	connect(drawFuselageSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePoints()));
	connect(drawFuselageSamplePointsAngleAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePointsAngle()));
	connect(drawFusedFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedFuselage()));

	// TIGL Actions
    /*tiglInterpolateBsplineWireAction->setStatusTip(tr("Use a BSpline interpolation between the points of a wire"));
	connect(tiglInterpolateBsplineWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglInterpolateBsplineWire()));

	tiglInterpolateLinearWireAction = new QAction( tr("Use Linear Interpolation"), this );
	tiglInterpolateLinearWireAction->setStatusTip(tr("Use a linear interpolation between the points of a wire"));
	connect(tiglInterpolateLinearWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglInterpolateLinearWire()));

	tiglApproximateBsplineWireAction = new QAction( tr("Use BSpline Approximation"), this );
	tiglApproximateBsplineWireAction->setStatusTip(tr("Use a BSpline approximation for the points of a wire"));
    connect(tiglApproximateBsplineWireAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(tiglApproximateBsplineWireAction()));*/

	// Export functions
	connect(tiglExportFusedIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFusedAsIges()));
	connect(tiglExportIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportAsIges()));
	connect(tiglExportMeshedWingSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingSTL()));
	connect(tiglExportMeshedFuselageSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageSTL()));
	connect(tiglExportMeshedWingVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTK()));
	connect(tiglExportMeshedWingVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTKsimple()));
	connect(tiglExportMeshedFuselageVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTK()));
	connect(tiglExportMeshedFuselageVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTKsimple()));

	// The co-ordinates from the view
	connect( myOCC, SIGNAL(mouseMoved(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
		     this,   SLOT(xyzPosition(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

	// Add a point from the view
	connect( myOCC, SIGNAL(pointClicked(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
		     this,   SLOT (addPoint    (V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

	connect( myOCC, SIGNAL(sendStatus(const QString)),
		     this,  SLOT  (statusMessage(const QString)) );

	connect( cpacsConfiguration, SIGNAL(documentUpdated(TiglCPACSConfigurationHandle)), 
		     this, SLOT(updateMenus(TiglCPACSConfigurationHandle)) );
}

void TIGLViewerWindow::createMenus()
{
    for (int i = 0; i < MaxRecentFiles; ++i)
        recentFileMenu->addAction(recentFileActions[i]);
    updateRecentFileActions();

    // TIGL menu
    //tiglMenu = menuBar()->addMenu( tr("&TIGL Methods") );
    //tiglAlgorithmMenu = tiglMenu->addMenu( tr("Algorithm") );
    //	tiglAlgorithmMenu->addAction( tiglInterpolateBsplineWireAction );
    //	tiglAlgorithmMenu->addAction( tiglInterpolateLinearWireAction );
    //	tiglAlgorithmMenu->addAction( tiglApproximateBsplineWireAction );
}

void TIGLViewerWindow::updateRecentFileActions()
{
    QSettings settings("DLR SC-VK","TIGLViewer");
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(files[i]);
        recentFileActions[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActions[j]->setVisible(false);

    recentFileMenu->setEnabled(numRecentFiles > 0);

    myLastFolder = settings.value("lastFolder").toString();
}

void TIGLViewerWindow::updateMenus(TiglCPACSConfigurationHandle hand){
    int nWings = 0;
    int nFuselages = 0;
    if(hand > 0){
        tiglGetWingCount(hand, &nWings);
        tiglGetFuselageCount(hand, &nFuselages);
    }
    menuWings->setEnabled(nWings > 0);
    menuFuselages->setEnabled(nFuselages > 0);
    menuAircraft->setEnabled(nWings > 0 || nFuselages > 0);

    closeAction->setEnabled(hand > 0);
}
