/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: main.cpp 203 2012-09-25 08:47:55Z martinsiggel $
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

#include <iostream>
#include <fstream>
#include <clocale>
#include <sstream>

#include <QString>
#include <QFile>
#include <QMessageBox>

#include <Standard_Version.hxx>
#include "TIGLViewerApp.h"

using namespace std;

int parseArguments(QStringList);
void showHelp(QString);

void loadStyle();

int main(int argc, char *argv[])
{
    // make sure, that the proper scaling is automatically used on highdpi displays
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    TIGLViewerApp app(argc, argv);

#ifdef __APPLE__
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
#endif

    app.loadStyle();

    // set shader file location
    QString shaderDir = QCoreApplication::applicationDirPath();
#ifdef __APPLE__
    shaderDir += "/../Resources";
#else
    shaderDir += "/../share/tigl3/shaders";
#endif
    
    QByteArray envVar = qgetenv("CSF_ShadersDirectory");
    if (envVar.isNull()) {
        qputenv("CSF_ShadersDirectory", shaderDir.toUtf8());
    }
    else {
        shaderDir = envVar;
    }
    
#if OCC_VERSION_HEX >= 0x060700
    // check existence of shader dir
    // This is only required for OpenCASCADE 6.7.0 and newer
    if (!QFile(shaderDir+"/PhongShading.fs").exists()) {
        std::stringstream str;
        str << "Illegal or non existing shader directory "
            << "<p><b>" << shaderDir.toStdString() << "</b></p>"
            << "Set the environment variable <b>CSF_ShadersDirectory</b> to provide a path for the OpenCASCADE shaders.";
        QMessageBox::critical(0, "Startup error...",
                                  str.str().c_str(),
                                  QMessageBox::Ok );
        return 1;
    }
#endif

    int retval = app.run();
    return retval;
}
