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
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma once

#include <OCC_3dBaseDoc.h>
#include "tigl.h"
#include "CTiglError.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglAlgorithmManager.h"
#include <iostream>
#include <exception>

class CTIGLViewerDoc : public OCC_3dBaseDoc
{

public:
	// Called on opening of a document
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	// Destructor
	virtual ~CTIGLViewerDoc();

	// Called by the MFC framework before destroying or reusing a document.
	virtual void DeleteContents();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Popups wing profile selection dialog
	afx_msg void OnShowWingProfiles();

	// Popups wing selection dialog and draws a wing
	afx_msg void OnShowWings();

	// Popups wings selection dialog and draws a wing with its triangulation overlayed
    afx_msg void OnShowWingTriangulation();

    // Popups wings selection dialog and draws a wing by drawing some surface points
    afx_msg void OnShowWingPoints();

	// Popups wings selection dialog and draws the profile point from the CPACS file
    afx_msg void OnDrawOverlayCPACSProfilePoints();

	// Popups fuselage profile selection dialog
    afx_msg void OnShowFuselageProfiles();

	// Popups fuselage selection dialog and draws a fuselage
    afx_msg void OnShowFuselages();

	// Popups fuselage selection dialog and draws a fuselage with its triangulation overlayed
    afx_msg void OnShowFuselageTriangulation();

    // Popups fuselage selection dialog and draws a fuselage by drawing some surface points
    afx_msg void OnShowFuselagePoints();

	// Draws a fuselage intersection point at 45° degrees
    afx_msg void OnShowFuselagePointsAngle();

    // Draws whole CPACS configuration by drawing some surface points
    afx_msg void OnShowAllPoints();

    // Draws whole CPACS configuration
	afx_msg void OnShowAll();

	// Draws wing/fuselage intersection line
	afx_msg void OnShowIntersectionLine();

    // Export CPACS configuration as IGES/STL file
    afx_msg void OnExportAsIges();
	afx_msg void OnExportAsFusedIges();
	afx_msg void OnExportMeshedWingSTL();
	afx_msg void OnExportMeshedFuselageSTL();
	afx_msg void OnExportMeshedWingVTKIndex();
	afx_msg void OnExportMeshedWingVTKSimple();
	afx_msg void OnExportMeshedFuselageVTKIndex();
	afx_msg void OnExportMeshedFuselageVTKSimple();

	// Change wire building algorithm
	afx_msg void OnInterpolateBsplineWire();
	afx_msg void OnInterpolateLinearWire();
	afx_msg void OnApproximateBsplineWire();
	afx_msg void OnUpdateInterpolateBsplineWire(CCmdUI *pCmdUI);
	afx_msg void OnUpdateInterpolateLinearWire(CCmdUI *pCmdUI);
	afx_msg void OnUpdateApproximateBsplineWire(CCmdUI *pCmdUI);

	// fuse operations
	afx_msg void OnShowFusedFuselages();
	afx_msg void OnShowFusedWings();
	afx_msg void OnShowFusedAll();

	// Returns the CPACS configuration
	tigl::CCPACSConfiguration& GetConfiguration(void) const;

	// Draws a point
	void DisplayPoint(gp_Pnt& aPoint,
		char* aText,
		Standard_Boolean UpdateViewer,
		Standard_Real anXoffset,
		Standard_Real anYoffset,
		Standard_Real aZoffset,
		Standard_Real TextScale);

	// Draw xyz axis
	void DrawXYZAxis(void);

	//debug functon to count sub-shapes
	void PrintContents (TopoDS_Shape geom);

protected: 
	// Constructor
	CTIGLViewerDoc();

private:
    TiglCPACSConfigurationHandle m_cpacsHandle;

	DECLARE_DYNCREATE(CTIGLViewerDoc)
	DECLARE_MESSAGE_MAP()

};


