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

#ifndef __AFXWIN_H__
#error 'stdafx.h' muss vor dieser Datei in PCH eingeschlossen werden.
#endif

#include "resource.h" 
#include "OCC_3dApp.h"


class CTIGLViewerApp : public OCC_3dApp
{

public:
	CTIGLViewerApp();

	virtual BOOL InitInstance();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();

public:
	CMultiDocTemplate* m_ptrTIGLView;

	DECLARE_MESSAGE_MAP()
};

extern CTIGLViewerApp theApp;
