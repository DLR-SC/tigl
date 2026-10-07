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

#include "TIGLCreatorApp.h"
#include "TIGLCreatorWindow.h"
#include "CommandLineParameters.h"

#include <iostream>

#include <QMessageBox>

namespace
{

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
    helpText += "  --windowtitle <title>    The title of the TiGLCreator window.\n";
    helpText += "  --controlFile <filename>    Name of the control file.\n";
    helpText += "  --suppress-errors        Suppress all error message dialogs\n";

    QMessageBox::information(0, "TiGLCreator Argument Error",
                                 helpText,
                                 QMessageBox::Ok );
}

}

TIGLCreatorApp::TIGLCreatorApp(int& argc, char* argv[])
    : QApplication(argc, argv)
{

#if defined __linux__
    // we need to set us locale as we use "." for decimal point
    qputenv("LC_NUMERIC", "C");
    setlocale(LC_NUMERIC, "C");
#elif defined __APPLE__
    setlocale(LC_NUMERIC, "C");
#endif

    QLocale::setDefault(QLocale(QLocale::English));

    mainwindow.reset(new TIGLCreatorWindow);

    for (int iarg = 0; iarg < argc; ++iarg) {
        args.append(argv[iarg]);
    }
}

void TIGLCreatorApp::loadStyle()
{
    QFile styleFile(":/qdarkstyle/style.qss");
    styleFile.open(QFile::ReadOnly);
    if (styleFile.isOpen()) {
        QString StyleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(StyleSheet);
    }
}

/**
 * Parsing the command line arguments. The values will be saved
 * in a global structure "PARAMS".
 */
int TIGLCreatorApp::parseArguments(QStringList argList)
{
    for (int i = 1; i < argList.size(); i++) {
        QString arg = argList.at(i);
        if (arg.compare("--help") == 0) {
            return -1;
        }
        else if (arg.compare("--filename") == 0) {
            if (i+1 >= argList.size()) {
                std::cout << "missing filename" << std::endl;
                return -1;
            }
            else {
                config.initialFilename = argList.at(++i);
            }
        }
        else if (arg.compare("--script") == 0) {
            if (i+1 >= argList.size()) {
                std::cout << "missing script filename" << std::endl;
                return -1;
            }
            else {
                config.initialScript = argList.at(++i);
            }
        }
        else if (arg.compare("--suppress-errors") == 0) {
            config.suppressErrors = true;
        }
        else if (arg.compare("--windowtitle") == 0) {
            if (i+1 >= argList.size()) {
                std::cout << "missing windowtitle" << std::endl;
                config.windowTitle = "TIGLCreator";
            }
            else {
                config.windowTitle = argList.at(++i);
            }
        }
        else if (arg.compare("--modelUID") == 0) {
            if (i+1 >= argList.size()) {
                std::cout << "missing modelUID" << std::endl;
                config.modelUID = "";
            }
            else {
                config.modelUID = argList.at(++i);
            }
        }
        else if (arg.compare("--controlFile") == 0) {
            if (i+1 >= argList.size()) {
                std::cout << "missing controlFile" << std::endl;
                config.controlFile = "";
            }
            else {
                config.controlFile = argList.at(++i);
            }
        }
        /* when there is a string behind the executable, we assume its the filename */
        else {
            config.initialFilename = arg;
        }
    }

    return 0;
}

void TIGLCreatorApp::onWindowInitialized()
{
    // suppress errors
    mainwindow->setSuppressErrorsEnabled(config.suppressErrors);

    if (!config.controlFile.isEmpty()){
        mainwindow->setInitialControlFile(config.controlFile);
    }
    
    // if a filename is given, open the configuration
    if (!config.initialFilename.isEmpty()) {
        mainwindow->openFile(config.initialFilename);
    }
    
    // if a script is given
    if (!config.initialScript.isEmpty()) {
        mainwindow->openScript(config.initialScript);
    }
}

int TIGLCreatorApp::run()
{
    int retval = parseArguments(args);
    if (retval != 0) {
        showHelp(args.at(0));
        return retval;
    }

    connect(mainwindow.get(), SIGNAL(windowInitialized()), this, SLOT(onWindowInitialized()));
    if (!config.windowTitle.isEmpty()) {
        mainwindow->setTiglWindowTitle(config.windowTitle, true);
    }
    mainwindow->show();
    
    retval = exec();
    return retval;
}
