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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "TIGLViewerDocument.h"
#include "TIGLViewerContext.h"
#include "TIGLViewerWidget.h"
#include "TIGLViewerInputoutput.h"

class QAction;
class QLabel;
class QMenu;
class QFileSystemWatcher;

class TIGLViewerWindow : public QMainWindow
{
    Q_OBJECT

public:

    TIGLViewerWindow();
	Handle_AIS_InteractiveContext& getContext() { return myVC->getContext(); };

	void setInitialCpacsFileName(QString filename);

	// Returns the Open CASCADE Widget and Context.
	TIGLViewerWidget* getMyOCC();

	// Displays a simple dialog for error messages
	void displayErrorMessage (const QString aMessage, QString aHeader);

private slots:
    void newFile();
    void open();
    void save();
    void print();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void about();
    void aboutQt();
	void xyzPosition (V3d_Coordinate X,
					  V3d_Coordinate Y,
					  V3d_Coordinate Z);
	void addPoint (V3d_Coordinate X,
				   V3d_Coordinate Y,
				   V3d_Coordinate Z);
	void statusMessage (const QString aMessage);

private:
    void createActions();
    void createMenus();

    QMenu *fileMenu;
    QMenu *editMenu;
	QMenu *viewMenu;
	QMenu *viewActionsMenu;
	QMenu *viewDisplayMenu;
    QMenu *formatMenu;
    QMenu *helpMenu;
	QMenu *gridMenu;
	QMenu *cpacsMenu;
	QMenu *tiglMenu;
	QMenu *tiglAlgorithmMenu;
	QMenu *tiglExportMenu;

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *printAction;
    QAction *exitAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *aboutAction;

	QAction *fitAction;
	QAction *fitAllAction;
	QAction *zoomAction;
	QAction *panAction;
	QAction *rotAction;

	QAction *gridXYAction;
	QAction *gridXZAction;
	QAction *gridYZAction;
	QAction *gridOnAction;
	QAction *gridOffAction;
	QAction *gridRectAction;
	QAction *gridCircAction;

	QAction *viewFrontAction;
	QAction *viewBackAction;
	QAction *viewTopAction;
	QAction *viewBottomAction;
	QAction *viewLeftAction;
	QAction *viewRightAction;
	QAction *viewAxoAction;
	QAction *viewResetAction;
	QAction *viewGridAction; /** Action for grid view */

	QAction *backgroundAction;
    QAction *aboutQtAction;

    // Aircraft actions
    QAction *showAllWingsAndFuselagesAction;
    QAction *showAllWingsAndFuselagesSurfacePointsAction;
    QAction *drawFusedAircraftAction;
    QAction *drawWingFuselageIntersectionLineAction;

    // Wing actions
    QAction *drawWingsAction;
    QAction *drawWingProfilesAction;
    QAction *drawWingOverlayCPACSProfilePointsAction;
    QAction *drawWingTriangulationAction;
    QAction *drawWingSamplePointsAction;
    QAction *drawFusedWingAction;
    QAction *drawWingComponentSegmentAction;

    // Fuselage actions
    QAction *drawFuselageProfilesAction;
    QAction *drawFuselageAction;
    QAction *drawFuselageTriangulationAction;
    QAction *drawFuselageSamplePointsAction;
    QAction *drawFuselageSamplePointsAngleAction;
    QAction *drawFusedFuselageAction;

    // TIGL actions
    QAction *tiglInterpolateBsplineWireAction;
    QAction *tiglInterpolateLinearWireAction;
    QAction *tiglApproximateBsplineWireAction;

	// Export actions
    QAction *tiglExportIgesAction;
    QAction *tiglExportFusedIgesAction;
    QAction *tiglExportMeshedWingSTL;
    QAction *tiglExportMeshedFuselageSTL;
    QAction *tiglExportMeshedWingVTK;
    QAction *tiglExportMeshedWingVTKsimple;
    QAction *tiglExportMeshedFuselageVTK;
    QAction *tiglExportMeshedFuselageVTKsimple;

	// The OpenCASCADE widget and context;
	TIGLViewerWidget*     	myOCC;
	TIGLViewerContext*  	myVC;

	QString 				myLastFolder;

	TIGLViewerDocument*		cpacsConfiguration;
	QString cpacsFileName;
	QString controlFileName;
	QFileSystemWatcher *watcher;

};

#endif

