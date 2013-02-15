/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerApplication.cpp 64 2011-08-02 13:54:09Z markus.litz $
*
* Version: $Revision: 64 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* � � http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <QtCore/QEvent>
#include <QtCore/QDataStream>
#include <QtGui/QCursor>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QThread>

#include "TIGLViewerInternal.h"
#include "TIGLViewerApplication.h"

TIGLViewerApplication::TIGLViewerApplication(int &argc, char **argv, int _internal ) :
	QApplication (argc, argv, _internal)
{
}

TIGLViewerApplication::~TIGLViewerApplication()
{
}

