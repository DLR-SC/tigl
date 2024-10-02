/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-03-15 Martin Siggel <Martin Siggel@dlr.de>
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

#ifndef TIGLVIEWERAPP_H
#define TIGLVIEWERAPP_H

#include <QObject>
#include <QApplication>
#include "CommandLineParameters.h"
#include "TIGLViewerWindow.h"
#include "tigl_internal.h"
#include <memory.h>

class TIGLViewerApp : public QApplication
{
    Q_OBJECT
public:
    explicit TIGLViewerApp(int& argc, char* argv[]);
    
    int run();
    void loadStyle();
private:
    int parseArguments(QStringList argList);

    CommandLineParameters config;
    QStringList args;
    std::unique_ptr<TIGLViewerWindow> mainwindow;

signals:
    
public slots:
    void onWindowInitialized();
};

#endif // TIGLVIEWERAPP_H
