/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerApplication.h 4495 2011-03-17 20:25:19Z litz_ma $
*
* Version: $Revision: 4495 $
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
#ifndef TIGLVIEWERAPPLICATION_H
#define TIGLVIEWERAPPLICATION_H

#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtGui/QPixmap>
#include <QtGui/QSplashScreen>

#include "TIGLViewer.h"

class QOCC_DECLSPEC TIGLViewerApplication : public QApplication
{

	Q_OBJECT

public:

	TIGLViewerApplication( int &argc, char **argv, int = QT_VERSION );
    ~TIGLViewerApplication( );
	
private:

};

#endif // TIGLVIEWERAPPLICATION_H
