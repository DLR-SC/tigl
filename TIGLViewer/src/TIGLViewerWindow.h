/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerWindow.h 203 2012-09-25 08:47:55Z martinsiggel $
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUndoStack>

#include "TIGLViewerContext.h"
#include "TIGLViewerDocument.h"
#include "tigl.h"
#include "CSharedPtr.h"

#include "ui_TIGLViewerWindow.h"

class QAction;
class QLabel;
class QMenu;
class QFileSystemWatcher;
class QShortcut;
class TIGLViewerLogHistory;
class TIGLViewerLogRedirection;

class TIGLViewerWindow : public QMainWindow, private Ui::TIGLViewerWindow
{
    Q_OBJECT
    Q_PROPERTY(TIGLViewerWidget*  viewer READ getViewer)
    Q_PROPERTY(TIGLViewerContext* scene  READ getScene)
    Q_CLASSINFO("Description", "TiGL Viewer Application")

public:
    enum { MaxRecentFiles = 5 };

    TIGLViewerWindow();
    ~TIGLViewerWindow() OVERRIDE;

    
    void setInitialControlFile(QString filename);

    class TIGLViewerSettings& getSettings();
    class Console*            getConsole();

    // Displays a simple dialog for error messages
    void displayErrorMessage (const QString aMessage, QString aHeader);

protected:
     void dropEvent(QDropEvent *ev) OVERRIDE;
     void dragEnterEvent(QDragEnterEvent *ev) OVERRIDE;

signals:
     void windowInitialized();

public slots:
    void openFile(const QString& fileName);
    void openScript(const QString& scriptFileName);
    bool saveFile(const QString& fileName);
    void closeConfiguration();
    void setTiglWindowTitle(const QString& title, bool forceTitle=false);
    
    TIGLViewerWidget*   getViewer();
    TIGLViewerContext*  getScene() { return myScene; }
    TIGLViewerDocument* getDocument() { return cpacsConfiguration; }

private slots:
    void updateMenus();
    void newFile();
    void open();
    void reopenFile();
    void openScript();
    void openRecentFile();
    void save();
    void setBackgroundImage();
    void about();
    void aboutQt();
    void xyzPosition (V3d_Coordinate X,
                      V3d_Coordinate Y,
                      V3d_Coordinate Z);
    void statusMessage (const QString aMessage);
    void loadSettings();
    void saveSettings();
    void applySettings();
    void changeSettings();
    void makeScreenShot();
    void drawPoint();
    void drawVector();

private:
    void connectSignals();
    void connectConfiguration();
    void createMenus();
    void updateRecentFileActions();
    void setCurrentFile(const QString &);

    void closeEvent(QCloseEvent*) OVERRIDE;
    bool deleteEnvVar(const char* varname);

    QAction *recentFileActions[MaxRecentFiles];

    // The OpenCASCADE context;
    TIGLViewerContext*      myScene;

    QString                 myLastFolder;

    TIGLViewerDocument* cpacsConfiguration;
    QString currentFile;
    QString controlFileName;
    QString preferredTitle;
    QFileSystemWatcher *watcher;
    class QDebugStream * stdoutStream, * errorStream;
    CSharedPtr<TIGLViewerLogHistory> logHistory;
    CSharedPtr<TIGLViewerLogRedirection> logDirect;
    class TIGLScriptEngine * scriptEngine;
    class TIGLViewerSettingsDialog * settingsDialog;
    class TIGLViewerSettings * tiglViewerSettings;
    class QTimer * openTimer;
    class QUndoStack* undoStack;

};

#endif

