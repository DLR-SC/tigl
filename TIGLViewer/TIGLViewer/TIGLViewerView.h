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

#include "OCC_3dView.h"
#include "TIGLViewerDoc.h"

class CTIGLViewerView : public OCC_3dView
{

public:
	CTIGLViewerDoc* GetDocument() const;
	virtual void OnDraw(CDC* pDC);     
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    virtual void OnInitialUpdate();

protected:
	CTIGLViewerView();           
	virtual ~CTIGLViewerView();

	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	DECLARE_DYNCREATE(CTIGLViewerView)
	DECLARE_MESSAGE_MAP()

};

#ifndef _DEBUG  // Debugversion in TIGLViewerView.cpp
inline CTIGLViewerDoc* CTIGLViewerView::GetDocument() const
   { return reinterpret_cast<CTIGLViewerDoc*>(m_pDocument); }
#endif

