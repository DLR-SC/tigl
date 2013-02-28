/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerContext.h 203 2012-09-25 08:47:55Z martinsiggel $
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
#ifndef TIGLVIEWERCONTEXT_H
#define TIGLVIEWERCONTEXT_H

#include <QtCore/QObject>
#include "TIGLViewer.h"

class QOCC_DECLSPEC TIGLViewerContext : public QObject
{

	Q_OBJECT

public:

	TIGLViewerContext();
    ~TIGLViewerContext();

	Handle_V3d_Viewer&              getViewer();
	Handle_AIS_InteractiveContext&  getContext();

	Handle_V3d_Viewer createViewer(	const Standard_CString aDisplay,
									const Standard_ExtString aName,
									const Standard_CString aDomain,
									const Standard_Real ViewSize );

	void deleteAllObjects();

	void setGridOffset (Quantity_Length offset);

public slots:

    void gridXY     ( void );
    void gridXZ     ( void );
    void gridYZ     ( void );
    void toggleGrid ( bool );
    void gridOn     ( void );
    void gridOff    ( void );
    void gridRect   ( void );
    void gridCirc   ( void );
    void wireFrame  ( bool );

signals:

	void error (int errorCode, QString& errorDescription);

private:

	Handle_V3d_Viewer				myViewer;
	Handle_AIS_InteractiveContext	myContext;
	Aspect_GridType					myGridType;
	Aspect_GridDrawMode				myGridMode;
	Quantity_NameOfColor			myGridColor;
	Quantity_NameOfColor			myGridTenthColor;

};

#endif // TIGLVIEWERCONTEXT_H
