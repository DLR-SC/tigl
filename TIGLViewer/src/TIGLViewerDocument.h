/*
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewerDocument.h 4482 2011-03-04 18:50:36Z litz_ma $
*
* Version: $Revision: 4482 $
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
#ifndef TIGLVIEWERDOCUMENT_H
#define TIGLVIEWERDOCUMENT_H

#include <QtCore/QObject>
#include "TIGLViewer.h"
#include "TIGLViewerWidget.h"
#include "tigl.h"
#include "CTiglError.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglAlgorithmManager.h"
#include "tixi.h"

class QOCC_DECLSPEC TIGLViewerDocument : public QObject
{
	Q_OBJECT

	#define CPACS_XPATH_AIRCRAFT 			"/cpacs/vehicles/aircraft"
	#define CPACS_XPATH_ROTORCRAFT 			"/cpacs/vehicles/rotorcraft"
	#define CPACS_XPATH_AIRCRAFT_MODEL 		"/cpacs/vehicles/aircraft/model"
	#define CPACS_XPATH_ROTORCRAFT_MODEL 	"/cpacs/vehicles/rotorcraft/model"

public:

	TIGLViewerDocument( QWidget *parentWidget, const Handle_AIS_InteractiveContext& ic );
    ~TIGLViewerDocument( );

    void openCpacsConfiguration(const QString fileName);
    void OnShowAll( );

    // a small helper when we just want to display a shape
    Handle(AIS_Shape) displayShape(TopoDS_Shape shape);

	// Draws a point
	void DisplayPoint(gp_Pnt& aPoint,
						char* aText,
						Standard_Boolean UpdateViewer,
						Standard_Real anXoffset,
						Standard_Real anYoffset,
						Standard_Real aZoffset,
						Standard_Real TextScale);

    // Returns the CPACS configuration
	tigl::CCPACSConfiguration& GetConfiguration(void) const;

public slots:
	// Wing slots
	void drawWingProfiles();
	void drawWings();
	void drawWingOverlayProfilePoints();

	// Fuselage slots
	void drawFuselageProfiles();
	void drawFuselage();

private slots:

	// Wing selection dialogs
	QString dlgGetWingSelection();
	QString dlgGetWingProfileSelection();


	// Fuselage selection dialogs
	QString dlgGetFuselageSelection();
	QString dlgGetFuselageProfileSelection();


private: 
    TiglCPACSConfigurationHandle 			m_cpacsHandle;
    QWidget* 								parent;
    Handle_AIS_InteractiveContext			myAISContext;
    TIGLViewerWidget*     					myOCC;
	
};

#endif // TIGLVIEWERDOCUMENT_H
