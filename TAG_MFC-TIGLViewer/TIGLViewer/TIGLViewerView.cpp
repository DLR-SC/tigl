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
#include "TIGLViewerView.h"
#include ".\tiglviewerview.h"


BEGIN_MESSAGE_MAP(CTIGLViewerView, OCC_3dView)
	ON_COMMAND(ID_FILE_PRINT,         OCC_3dView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT,  OCC_3dView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OCC_3dView::OnFilePrintPreview)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CTIGLViewerView, OCC_3dView)

CTIGLViewerView::CTIGLViewerView() 
	: OCC_3dView()
{
}

CTIGLViewerView::~CTIGLViewerView()
{
}

void CTIGLViewerView::OnDraw(CDC* pDC)
{
	OCC_3dView::OnDraw(pDC);
}

BOOL CTIGLViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CTIGLViewerView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	OCC_3dView::OnBeginPrinting(pDC, pInfo);
}

void CTIGLViewerView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	OCC_3dView::OnEndPrinting(pDC, pInfo);
}

void CTIGLViewerView::OnInitialUpdate()
{
    OCC_3dView::OnInitialUpdate();
    OCC_3dView::FitAll();
}

#ifdef _DEBUG
void CTIGLViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CTIGLViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTIGLViewerDoc* CTIGLViewerView::GetDocument() const // Nicht-Debugversion ist inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTIGLViewerDoc)));
	return (CTIGLViewerDoc*)m_pDocument;
}

#endif //_DEBUG

