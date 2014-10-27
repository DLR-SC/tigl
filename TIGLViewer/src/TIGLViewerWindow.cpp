/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
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
#include <QTimer>
#include <QProcessEnvironment>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopoDS_Vertex.hxx>
#include <Handle_AIS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Aspect_RectangularGrid.hxx>
#include <Standard_Version.hxx>

#include "TIGLViewerWindow.h"
#include "TIGLViewerSettingsDialog.h"
#include "TIGLViewerDocument.h"
#include "TIGLViewerInputoutput.h"
#include "TIGLDebugStream.h"
#include "TIGLScriptEngine.h"
#include "CommandLineParameters.h"
#include "TIGLViewerSettings.h"
#include "TiglViewerConsole.h"
#include "TIGLViewerControlFile.h"
#include "TIGLViewerErrorDialog.h"
#include "TIGLViewerLogHistory.h"
#include "TIGLViewerLogRedirection.h"
#include "CTiglLogSplitter.h"
#include "TIGLViewerLoggerHTMLDecorator.h"
#include "TIGLViewerScreenshotDialog.h"
#include "TIGLViewerScopedCommand.h"

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
     for (myScene->getContext()->InitCurrent(); myScene->getContext()->MoreCurrent (); myScene->getContext()->NextCurrent ()) {
         if (myScene->getContext()->IsDisplayed(myScene->getContext()->Current())) {
             OneOrMoreIsSelected=true;
         }
     }

     if (OneOrMoreIsSelected) {
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

    tiglViewerSettings = &TIGLViewerSettings::Instance();
    settingsDialog = new TIGLViewerSettingsDialog(*tiglViewerSettings, this);

    myScene  = new TIGLViewerContext();
    myOCC->setContext(myScene->getContext());

    // we create a timer to workaround QFileSystemWatcher bug,
    // which emits multiple signals in a few milliseconds. This caused
    // TIGLViewer to also open a document many times.
    // Now, when we get a signal, we start the timer and only after the
    // timer times out, we reopen the cpacs config
    openTimer = new QTimer(this);
    openTimer->setSingleShot(true);
    // the timeout of 200ms is a bit arbitrary but should be conservative enough
    // to catch all events
    openTimer->setInterval(200);

    //redirect everything to TIGL console, let error messages be printed in red
    stdoutStream = new QDebugStream(std::cout);
    errorStream  = new QDebugStream(std::cerr);
    errorStream->setMarkup("<b><font color=\"red\">","</font></b>");

    // insert two loggers, one for the log history and one for the console
    CSharedPtr<tigl::CTiglLogSplitter> splitter(new tigl::CTiglLogSplitter);
    logHistory = CSharedPtr<TIGLViewerLogHistory>(new TIGLViewerLogHistory);
    logHistory->SetVerbosity(TILOG_DEBUG4);
    splitter->AddLogger(logHistory);

    logDirect = CSharedPtr<TIGLViewerLogRedirection>(new TIGLViewerLogRedirection);
    logDirect->SetVerbosity(TILOG_WARNING);
    CSharedPtr<TIGLViewerLoggerHTMLDecorator> logHTMLDecorator(new TIGLViewerLoggerHTMLDecorator(logDirect));
    splitter->AddLogger(logHTMLDecorator);

    // register logger at tigl
    tigl::CTiglLogging::Instance().SetLogger(splitter);

    QPalette p = console->palette();
    p.setColor(QPalette::Base, Qt::black);
    console->setPalette(p);

    cpacsConfiguration = new TIGLViewerDocument(this);
    scriptEngine = new TIGLScriptEngine(this);
    
    setAcceptDrops(true);

    connectSignals();
    createMenus();
    updateMenus(-1);

    loadSettings();

    statusBar()->showMessage(tr("A context menu is available by right-clicking"));

    setWindowTitle(tr(PARAMS.windowTitle.toAscii().data()));
    setMinimumSize(160, 160);
}

TIGLViewerWindow::~TIGLViewerWindow()
{
    delete stdoutStream;
    delete errorStream;
}

void TIGLViewerWindow::dragEnterEvent(QDragEnterEvent * ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();
    foreach (QUrl url, urls) {
        if (!url.toLocalFile().isEmpty()) {
            QString suffix = QFileInfo(url.toLocalFile()).suffix();
            if (suffix == "xml" || suffix == "brep" || suffix == "stp" || suffix == "igs" || suffix == "mesh") {
                ev->accept();
            }
        }
    }
}

void TIGLViewerWindow::dropEvent(QDropEvent *ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();
    foreach (QUrl url, urls) {
        if (!url.toLocalFile().isEmpty()) {
            QString suffix = QFileInfo(url.toLocalFile()).suffix();
            if (suffix == "xml" || suffix == "brep" || suffix == "stp" || suffix == "igs" || suffix == "mesh") {
                ev->accept();
                // load file
                openFile(url.toLocalFile());
            }
        }
    }
}


void TIGLViewerWindow::setInitialControlFile(QString filename)
{
    TIGLViewerControlFile cf;
    if (cf.read(filename.toStdString().c_str()) == CF_SUCCESS) {
        if (cf.showConsole == CF_TRUE) {
            console->setVisible(true);
            showConsoleAction->setChecked(true);
        }
        else if (cf.showConsole == CF_FALSE) {
            console->setVisible(false);
            showConsoleAction->setChecked(false);
        }
        if (cf.showToolbars == CF_TRUE) {
            toolBar->setVisible(true);
            toolBarView->setVisible(true);
        }
        else if (cf.showToolbars == CF_FALSE) {
            toolBar->setVisible(false);
            toolBarView->setVisible(false);
        }
    }

}


void TIGLViewerWindow::newFile()
{
    statusBar()->showMessage(tr("Invoked File|New"));
    //myOCC->getView()->ColorScaleErase();
    myScene->deleteAllObjects();
}

void TIGLViewerWindow::open()
{
    QString        fileName;

    statusBar()->showMessage(tr("Invoked File|Open"));

    fileName = QFileDialog::getOpenFileName (    this,
                                                  tr("Open File"),
                                                myLastFolder,
                                                tr( "CPACS (*.xml);;"
                                                    "Other drawing types (*.brep *.rle *.igs *iges *.stp *.step *.mesh);;"
                                                    "BREP (*.brep *.rle);;"
                                                    "STEP (*.step *.stp);;"
                                                    "IGES (*.iges *.igs);;"
                                                    "STL  (*.stl);;"
                                                    "Hotsose Mesh (*.mesh)") );
    openFile(fileName);
}


void TIGLViewerWindow::openScript()
{
    QString     fileName;

    statusBar()->showMessage(tr("Invoked File|Open Script"));

    fileName = QFileDialog::getOpenFileName (   this,
                                                tr("Open File"),
                                                myLastFolder,
                                                tr( "Choose your script (*)" ) );
    openScript(fileName);
}

void TIGLViewerWindow::openScript(const QString& fileName)
{
    scriptEngine->openFile(fileName);
}

void TIGLViewerWindow::closeConfiguration()
{
    cpacsConfiguration->closeCpacsConfiguration();
}

void TIGLViewerWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        openFile(action->data().toString());
    }
}

void TIGLViewerWindow::openFile(const QString& fileName)
{
    QString        fileType;
    QFileInfo    fileInfo;

    TIGLViewerInputOutput::FileFormat format;
    TIGLViewerInputOutput reader;
    bool triangulation = false;

    TIGLViewerScopedCommand command(getConsole());
    Q_UNUSED(command);
    statusBar()->showMessage(tr("Invoked File|Open"));

    if (!fileName.isEmpty()) {
        fileInfo.setFile(fileName);
        fileType = fileInfo.suffix();
        
        if (fileType.toLower() == tr("xml")) {
            TiglReturnCode tiglRet = cpacsConfiguration->openCpacsConfiguration(fileInfo.absoluteFilePath());
            if (tiglRet != TIGL_SUCCESS) {
                return;
            }

            updateMenus(cpacsConfiguration->getCpacsHandle());
        }
        else {

            if (fileType.toLower() == tr("brep") || fileType.toLower() == tr("rle")) {
                format = TIGLViewerInputOutput::FormatBREP;
            }
            if (fileType.toLower() == tr("step") || fileType.toLower() == tr("stp")) {
                format = TIGLViewerInputOutput::FormatSTEP;
            }
            if (fileType.toLower() == tr("iges") || fileType.toLower() == tr("igs")) {
                format = TIGLViewerInputOutput::FormatIGES;
            }
            if (fileType.toLower() == tr("stl")) {
                format = TIGLViewerInputOutput::FormatSTL;
            }
            if (fileType.toLower() == tr("mesh")) {
                format = TIGLViewerInputOutput::FormatMESH;
                triangulation = true;
            }
            if (triangulation) {
                reader.importTriangulation( fileInfo.absoluteFilePath(), format, *getScene() );
            }
            else {
                reader.importModel ( fileInfo.absoluteFilePath(), format, *getScene() );
            }
        }
        watcher = new QFileSystemWatcher();
        watcher->addPath(fileInfo.absoluteFilePath());
        QObject::connect(watcher, SIGNAL(fileChanged(QString)), openTimer, SLOT(start()));
    }
    myLastFolder = fileInfo.absolutePath();
    setCurrentFile(fileName);

    myOCC->viewAxo();
    myOCC->fitAll();
}

void TIGLViewerWindow::reopenFile()
{
    QString      fileType;
    QFileInfo    fileInfo;

    fileInfo.setFile(currentFile);
    fileType = fileInfo.suffix();

    if (fileType.toLower() == tr("xml")){
        cpacsConfiguration->updateConfiguration();
    }
    else {
        myScene->getContext()->EraseAll(Standard_False);
        openFile(currentFile);
    }
}

void TIGLViewerWindow::setCurrentFile(const QString &fileName)
{
    setWindowFilePath(fileName);
    currentFile = fileName;

    QSettings settings("DLR SC-VK","TIGLViewer");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);
    settings.setValue("lastFolder", myLastFolder);

    updateRecentFileActions();
}

void TIGLViewerWindow::loadSettings()
{
    QSettings settings("DLR SC-VK","TIGLViewer");

    bool showConsole = settings.value("show_console",QVariant(true)).toBool();

    restoreGeometry(settings.value("MainWindowGeom").toByteArray());
    restoreState(settings.value("MainWindowState").toByteArray());
    consoleDockWidget->setVisible(showConsole);
    showConsoleAction->setChecked(showConsole);

    tiglViewerSettings->loadSettings();
    settingsDialog->updateEntries();
    applySettings();
}

void TIGLViewerWindow::saveSettings()
{
    QSettings settings("DLR SC-VK","TIGLViewer");

    bool showConsole = consoleDockWidget->isVisible();
    settings.setValue("show_console", showConsole);

    settings.setValue("MainWindowGeom", saveGeometry());
    settings.setValue("MainWindowState", saveState());

    tiglViewerSettings->storeSettings();
}

void TIGLViewerWindow::applySettings()
{
    QColor col = tiglViewerSettings->BGColor();
    myOCC->setBackgroundGradient(col.red(), col.green(), col.blue());
    getScene()->getContext()->SetIsoNumber(tiglViewerSettings->numFaceIsosForDisplay());
    getScene()->getContext()->UpdateCurrentViewer();
    if (tiglViewerSettings->debugBooleanOperations()) {
        qputenv("TIGL_DEBUG_BOP", "1");
    }
    else {
        qputenv("TIGL_DEBUG_BOP", "");
    }
}

void TIGLViewerWindow::changeSettings()
{
    settingsDialog->updateEntries();
    settingsDialog->exec();
    applySettings();
}


TIGLViewerWidget* TIGLViewerWindow::getViewer()
{
    return myOCC;
}


void TIGLViewerWindow::save()
{
    QString     fileName;
    QString        fileType;
    QFileInfo    fileInfo;

    TIGLViewerInputOutput::FileFormat format;
    TIGLViewerInputOutput writer;

    statusBar()->showMessage(tr("Invoked File|Save"));

    fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), myLastFolder,
                                            tr("IGES Geometry (*.igs);;") +
                                            tr("STEP Geometry (*.stp);;") +
                                            tr("STL Triangulation (*.stl);;") +
                                            tr("BRep Geometry (*.brep)"));

    if (!fileName.isEmpty()) {
        fileInfo.setFile(fileName);
        fileType = fileInfo.suffix();
        if (fileType.toLower() == tr("brep") || fileType.toLower() == tr("rle")) {
            format = TIGLViewerInputOutput::FormatBREP;
        }
        if (fileType.toLower() == tr("step") || fileType.toLower() == tr("stp")) {
            format = TIGLViewerInputOutput::FormatSTEP;
        }
        if (fileType.toLower() == tr("iges") || fileType.toLower() == tr("igs")) {
            format = TIGLViewerInputOutput::FormatIGES;
        }
        if (fileType.toLower() == tr("stl")) {
            format = TIGLViewerInputOutput::FormatSTL;
        }

        myLastFolder = fileInfo.absolutePath();
        writer.exportModel ( fileInfo.absoluteFilePath(), format, getScene()->getContext() );
    }
}

void TIGLViewerWindow::print()
{
    QMessageBox msgBox;
    QString text =     "Printing is not yet implemented!<br>Go to TIGLViewer project page \
                    (<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked File|Print"));

    msgBox.setWindowTitle("Undo not yet implemented!");
    msgBox.setText(text);
    msgBox.exec();
}


void TIGLViewerWindow::setBackgroundImage()
{
    QString        fileName;
    QString        fileType;
    QFileInfo    fileInfo;

    statusBar()->showMessage(tr("Invoked File|Load Background Image"));

    fileName = QFileDialog::getOpenFileName (    this,
                                                  tr("Open Background Image"),
                                                myLastFolder,
                                                tr( "Images (*.jpg *.png *.gif *.bmp);;" ) );
    if (!fileName.isEmpty()) {
        fileInfo.setFile(fileName);
        fileType = fileInfo.suffix();
        
        if (fileType.toLower() == tr("bmp") || fileType.toLower() == tr("gif") || fileType.toLower() == tr("jpg") || fileType.toLower() == tr("png")) {
            myOCC->setBGImage(fileName);
        }
        else {
            displayErrorMessage(tr("Invalid image format!"),tr("TIGL Error"));
        }

    }
}


void TIGLViewerWindow::undo()
{
    QMessageBox msgBox;
    QString text =     "Undo is not yet implemented!<br>Go to TIGLViewer project page \
                    (<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Undo"));

    msgBox.setWindowTitle("Undo not yet implemented!");
    msgBox.setText(text);
    msgBox.exec();
}

void TIGLViewerWindow::redo()
{
    QMessageBox msgBox;
    QString text =     "Redo is not yet implemented!<br>Go to TIGLViewer project page \
                    (<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Redo"));

    msgBox.setWindowTitle("Redo not yet implemented!");
    msgBox.setText(text);
    msgBox.exec();
}

void TIGLViewerWindow::cut()
{
    QMessageBox msgBox;
    QString text =     "Cut is not yet implemented!<br>Go to TIGLViewer project page \
                    (<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Cut"));

    msgBox.setWindowTitle("Cut not yet implemented!");
    msgBox.setText(text);
    msgBox.exec();
}

void TIGLViewerWindow::copy()
{
    QMessageBox msgBox;
    QString text =     "Copy is not yet implemented!<br>Go to TIGLViewer project page \
                    (<a href=\"http://code.google.com/p/tigl/\">http://code.google.com/p/tigl/</a>) and make a feature request";

    statusBar()->showMessage(tr("Invoked Edit|Copy"));

    msgBox.setWindowTitle("Copy not yet implemented!");
    msgBox.setText(text);
    msgBox.exec();
}

void TIGLViewerWindow::paste()
{
    QMessageBox msgBox;
    QString text =     "Paste is not yet implemented!<br>Go to TIGLViewer project page \
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

    text =     "The <b>TIGLViewer</b> allows you to view CPACS geometries.<br> \
                   Copyright (C) 2007-2013 German Aerospace Center (DLR/SC) <br><br>";
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
    AddVertex ( X, Y, Z, myScene->getContext() );
}

void TIGLViewerWindow::statusMessage (const QString aMessage)
{
    statusBar()->showMessage(aMessage);
}


void TIGLViewerWindow::displayErrorMessage (const QString aMessage, QString aHeader = "TIGL Error")
{
    TIGLViewerErrorDialog dialog(this);
    dialog.setMessage(QString("<b>%1</b><br /><br />%2").arg(aHeader).arg(aMessage));
    dialog.setWindowTitle("Error");
    dialog.setDetailsText(logHistory->GetAllMessages());
    dialog.exec();
}

void TIGLViewerWindow::connectSignals()
{
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(openScriptAction, SIGNAL(triggered()), this, SLOT(openScript()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(closeConfiguration()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(saveScreenshotAction, SIGNAL(triggered()), this, SLOT(makeScreenShot()));
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
    connect(selectAction, SIGNAL(triggered()), myOCC, SLOT(selecting()));

    // view->grid menu
    connect(gridOnAction, SIGNAL(toggled(bool)), myScene, SLOT(toggleGrid(bool)));
    connect(gridXYAction, SIGNAL(triggered()), myScene, SLOT(gridXY()));
    connect(gridXZAction, SIGNAL(triggered()), myScene, SLOT(gridXZ()));
    connect(gridYZAction, SIGNAL(triggered()), myScene, SLOT(gridYZ()));
    connect(gridRectAction, SIGNAL(triggered()), myScene, SLOT(gridRect()));
    connect(gridCircAction, SIGNAL(triggered()), myScene, SLOT(gridCirc()));

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
    connect(showConsoleAction, SIGNAL(toggled(bool)), consoleDockWidget, SLOT(setVisible(bool)));
    connect(consoleDockWidget, SIGNAL(visibilityChanged(bool)), showConsoleAction, SLOT(setChecked(bool)));
    connect(showWireframeAction, SIGNAL(toggled(bool)), myScene, SLOT(wireFrame(bool)));

    connect(openTimer, SIGNAL(timeout()), this, SLOT(reopenFile()));

    // CPACS Wing Actions
    connect(drawWingProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingProfiles()));
    connect(drawWingOverlayCPACSProfilePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingOverlayProfilePoints()));
    connect(drawWingGuideCurvesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingGuideCurves()));
    connect(drawWingsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWing()));
    connect(drawWingTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingTriangulation()));
    connect(drawWingSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingSamplePoints()));
    connect(drawFusedWingAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedWing()));
    connect(drawWingComponentSegmentAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingComponentSegment()));
    connect(drawWingCSPointAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingComponentSegmentPoints()));
    connect(drawWingShellAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawWingShells()));


    // CPACS Aircraft Actions
    connect(showAllWingsAndFuselagesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWings()));
    connect(showAllWingsAndFuselagesSurfacePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawAllFuselagesAndWingsSurfacePoints()));
    connect(drawFusedAircraftAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedAircraft()));
    connect(drawIntersectionAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawIntersectionLine()));
    connect(showFusedAirplaneTriangulation, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedAircraftTriangulation()));
    connect(drawFarFieldAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFarField()));

    // CPACS Fuselage Actions
    connect(drawFuselageProfilesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageProfiles()));
    connect(drawFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselage()));
    connect(drawFuselageTriangulationAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageTriangulation()));
    connect(drawFuselageSamplePointsAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePoints()));
    connect(drawFuselageSamplePointsAngleAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageSamplePointsAngle()));
    connect(drawFusedFuselageAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFusedFuselage()));
    connect(drawFuselageGuideCurvesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawFuselageGuideCurves()));
    
    // Misc drawing actions
    connect(drawPointAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawPoint()));
    connect(drawVectorAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(drawVector()));

    // Export functions
    connect(tiglExportFusedIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFusedAsIges()));
    connect(tiglExportIgesAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportAsIges()));
    connect(tiglExportFusedStepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportAsStepFused()));
    connect(tiglExportStepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportAsStep()));
    connect(tiglExportMeshedWingSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingSTL()));
    connect(tiglExportMeshedFuselageSTL, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageSTL()));
    connect(tiglExportMeshedWingVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTK()));
    connect(tiglExportMeshedWingVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedWingVTKsimple()));
    connect(tiglExportMeshedFuselageVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTK()));
    connect(tiglExportMeshedConfigVTK, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedConfigVTK()));
    connect(tiglExportMeshedConfigVTKNoFuse, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedConfigVTKNoFuse()));
    connect(tiglExportMeshedFuselageVTKsimple, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportMeshedFuselageVTKsimple()));
    connect(tiglExportWingColladaAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportWingCollada()));
    connect(tiglExportFuselageColladaAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFuselageCollada()));
    connect(tiglExportConfigurationColladaAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportConfigCollada()));
    connect(tiglExportFuselageBRepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFuselageBRep()));
    connect(tiglExportWingBRepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportWingBRep()));
    connect(tiglExportFusedConfigBRep, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFusedConfigBRep()));
    connect(tiglExportWingCurvesBRepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportWingCurvesBRep()));
    connect(tiglExportFuselageCurvesBRepAction, SIGNAL(triggered()), cpacsConfiguration, SLOT(exportFuselageCurvesBRep()));

    // The co-ordinates from the view
    connect( myOCC, SIGNAL(mouseMoved(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
             this,   SLOT(xyzPosition(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

    // Add a point from the view
    connect( myOCC, SIGNAL(pointClicked(V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)),
             this,   SLOT (addPoint    (V3d_Coordinate,V3d_Coordinate,V3d_Coordinate)) );

    connect( myOCC, SIGNAL(sendStatus(const QString)), this,  SLOT  (statusMessage(const QString)) );

    connect( cpacsConfiguration, SIGNAL(documentUpdated(TiglCPACSConfigurationHandle)), 
             this, SLOT(updateMenus(TiglCPACSConfigurationHandle)) );

    connect(stdoutStream, SIGNAL(sendString(QString)), console, SLOT(output(QString)));
    connect(errorStream , SIGNAL(sendString(QString)), console, SLOT(output(QString)));

    connect(logDirect.get(), SIGNAL(newMessage(QString)), console, SLOT(output(QString)));

    connect(scriptEngine, SIGNAL(scriptResult(QString)), console, SLOT(output(QString)));
    connect(scriptEngine, SIGNAL(scriptError(QString)), console, SLOT(outputError(QString)));
    connect(scriptEngine, SIGNAL(evalDone()), console, SLOT(endCommand()));
    connect(console, SIGNAL(onChange(QString)), scriptEngine, SLOT(textChanged(QString)));
    connect(console, SIGNAL(onCommand(QString)), scriptEngine, SLOT(eval(QString)));

    connect(settingsAction, SIGNAL(triggered()), this, SLOT(changeSettings()));
}

void TIGLViewerWindow::createMenus()
{
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileMenu->addAction(recentFileActions[i]);
    }
    updateRecentFileActions();
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
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j) {
        recentFileActions[j]->setVisible(false);
    }

    recentFileMenu->setEnabled(numRecentFiles > 0);

    myLastFolder = settings.value("lastFolder").toString();
}

void TIGLViewerWindow::updateMenus(TiglCPACSConfigurationHandle hand)
{
    int nWings = 0;
    int nFuselages = 0;
    if (hand > 0) {
        tiglGetWingCount(hand, &nWings);
        tiglGetFuselageCount(hand, &nFuselages);
    }
    menuWings->setEnabled(nWings > 0);
    menuFuselages->setEnabled(nFuselages > 0);
    menuAircraft->setEnabled(nWings > 0 || nFuselages > 0);

    closeAction->setEnabled(hand > 0);

    bool hasFarField = false;
    try {
        if (hand > 0) {
            tigl::CCPACSConfiguration& config = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(hand);
            hasFarField = config.GetFarField().GetFieldType() != tigl::NONE;
        }
    }
    catch(tigl::CTiglError& ){}
    drawFarFieldAction->setEnabled(hasFarField);
}

void TIGLViewerWindow::closeEvent(QCloseEvent*)
{
    saveSettings();
}

TIGLViewerSettings& TIGLViewerWindow::getSettings()
{
    return *tiglViewerSettings;
}

Console* TIGLViewerWindow::getConsole()
{
    return console;
}

void TIGLViewerWindow::makeScreenShot()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Screenshot"), myLastFolder,
                                                    tr("PNG-Image (*.png);;") +
                                                    tr("JPEG-Image (*.jpg);;") +
                                                    tr("Windows-BMP-Image (*.bmp)"));

    if (!fileName.isEmpty() && myOCC) {
        
        TIGLViewerScreenshotDialog dialog(fileName, this);
        dialog.setQualityValue(80);
        dialog.setImageSize(myOCC->width(), myOCC->height());
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        
        int width, height;
        dialog.getImageSize(width, height);
        try {
            myOCC->makeScreenshot(width, height, dialog.getQualityValue(), fileName);
        }
        catch(tigl::CTiglError&) {
            displayErrorMessage("Cannot save screenshot.", "Error");
        }
    }
}
