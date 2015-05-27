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

#include <QString>
#include <QMessageBox>

#include "TIGLViewerWindow.h"
#include "CommandLineParameters.h"

using namespace std;

int parseArguments(QStringList);
void showHelp(QString);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
#ifdef __APPLE__
    if (!getenv("CSF_GraphicShr")){
        static char env[64];
        strcpy(env,"CSF_GraphicShr=@executable_path/libTKOpenGl.dylib");
        putenv(env);
    }
#endif

#if defined __linux__
    // we need to set us locale as we use "." for decimal point
    putenv("LC_NUMERIC=C");
    setlocale(LC_NUMERIC, "C");
#elif defined __APPLE__
    setlocale(LC_NUMERIC, "C");
#endif

    int retval = parseArguments(app.arguments());
    if (retval != 0) {
        showHelp(app.arguments().at(0));
        return retval;
    }

    TIGLViewerWindow *window = new TIGLViewerWindow();
    window->show();

    if (!PARAMS.controlFile.isEmpty()){
        if (window->getViewer()) {
            window->getViewer()->repaint();
        }
        window->setInitialControlFile(PARAMS.controlFile);
    }

    // if a filename is given, open the configuration
    if (!PARAMS.initialFilename.isEmpty()) {
        if (window->getViewer()) {
            window->getViewer()->repaint();
        }
        window->openFile(PARAMS.initialFilename);
    }
    
    // if a script is given
    if (!PARAMS.initialScript.isEmpty()) {
        if (window->getViewer()) {
            window->getViewer()->repaint();
        }
        window->openScript(PARAMS.initialScript);
    }

    retval = app.exec();
    window->hide();
    delete window;
    return retval;
}

/**
 * Show a dialog with command line information.
 */
void showHelp(QString appName)
{
    QString helpText = appName + " [--help] [--filename <filename>]\n\n";
    helpText += "  --help                      This help page\n";
    helpText += "  --filename <filename>    Initial CPACS file to open and display.\n";
    helpText += "  --modelUID <uid>         Initial model uid open and display.\n";
    helpText += "  --script <filename>       Script to execute.\n";
    helpText += "  --windowtitle <title>    The titel of the TIGLViewer window.\n";
    helpText += "  --controlFile <filename>    Name of the control file.\n";
    helpText += "  --JediMode <on|off>      Makes you some kind of superhero like CPACS-Ninja.\n";

    QMessageBox::information(0, "TIGLViewer Argument Error",
                                 helpText,
                                 QMessageBox::Ok );
}

/**
 * Parsing the command line arguments. The values will be saved
 * in a global structure "PARAMS".
 */
int parseArguments(QStringList argList)
{
    for (int i = 1; i < argList.size(); i++) {
        QString arg = argList.at(i);
        if (arg.compare("--help") == 0) {
            return -1;
        }
        else if (arg.compare("--filename") == 0) {
            if (i+1 >= argList.size()) {
                cout << "missing filename" << endl;
                return -1;
            }
            else {
                PARAMS.initialFilename = argList.at(++i);
            }
        }
        else if (arg.compare("--script") == 0) {
            if (i+1 >= argList.size()) {
                cout << "missing script filename" << endl;
                return -1;
            }
            else {
                PARAMS.initialScript = argList.at(++i);
            }
        }
        else if (arg.compare("--windowtitle") == 0) {
            if (i+1 >= argList.size()) {
                cout << "missing windowtitle" << endl;
                PARAMS.windowTitle = "TIGLViewer";
            }
            else {
                PARAMS.windowTitle = argList.at(++i);
            }
        }
        else if (arg.compare("--modelUID") == 0) {
            if (i+1 >= argList.size()) {
                cout << "missing modelUID" << endl;
                PARAMS.modelUID = "";
            }
            else {
                PARAMS.modelUID = argList.at(++i);
            }
        }
        else if (arg.compare("--controlFile") == 0) {
            if (i+1 >= argList.size()) {
                cout << "missing controlFile" << endl;
                PARAMS.controlFile = "";
            }
            else {
                PARAMS.controlFile = argList.at(++i);
            }
        }
        /* when there is a string behind the executable, we assume its the filename */
        else {
            PARAMS.initialFilename = arg;
        }
    }

    return 0;
}

