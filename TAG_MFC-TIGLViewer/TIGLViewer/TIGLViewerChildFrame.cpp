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

#include "stdafx.h"
#include "TIGLViewer.h"
#include "TIGLViewerChildFrame.h"

IMPLEMENT_DYNCREATE(CTIGLViewerChildFrame, OCC_3dChildFrame)

BEGIN_MESSAGE_MAP(CTIGLViewerChildFrame, OCC_3dChildFrame)
	ON_WM_CREATE()
END_MESSAGE_MAP()

CTIGLViewerChildFrame::CTIGLViewerChildFrame()
{
}

CTIGLViewerChildFrame::~CTIGLViewerChildFrame()
{
}

int CTIGLViewerChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (OCC_3dChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_tiglToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | IDR_TIGLCHILDFRAME) ||
		!m_tiglToolBar.LoadToolBar(IDR_TIGLCHILDFRAME)) 
	{
		TRACE("Failed to create new toolbar\n");
		return -1;
	}

	m_tiglToolBar.SetBarStyle(m_tiglToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_tiglToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_tiglToolBar, AFX_IDW_DOCKBAR_LEFT);

	return 0;
}




