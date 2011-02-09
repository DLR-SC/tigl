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
#include "WingDialog.h"
#include "CCPACSWing.h"
#include "TIGLViewerDoc.h"
#include "CCPACSConfiguration.h"

IMPLEMENT_DYNAMIC(CWingDialog, CDialog)

BEGIN_MESSAGE_MAP(CWingDialog, CDialog)
END_MESSAGE_MAP()

CWingDialog::CWingDialog(CTIGLViewerDoc& document)
	: CDialog(CWingDialog::IDD, 0)
	, m_document(document)
{
}

CWingDialog::~CWingDialog()
{
}

void CWingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WING_LIST, m_wingList);
}

int CWingDialog::GetWingIndex(void) const
{
    return index;
}

std::string CWingDialog::GetWingName(void) const
{
    return wingName;
}
void CWingDialog::OnOK()
{
	CString strTemp;
	UpdateData(TRUE);
	index = m_wingList.GetCurSel();
	m_wingList.GetText(index, strTemp);
	wingName = (char*)(LPCSTR)strTemp; 
    CDialog::OnOK();
}

BOOL CWingDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize wing list
	tigl::CCPACSConfiguration& config = m_document.GetConfiguration();
	int wingCount = config.GetWingCount();
	for (int i = 1; i <= wingCount; i++) 
	{
		tigl::CCPACSWing& wing = config.GetWing(i);
		std::string name = wing.GetName();
		if (name == "") name = "Unknown wing";
		m_wingList.InsertString(i - 1, _T(name.c_str()));
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}
