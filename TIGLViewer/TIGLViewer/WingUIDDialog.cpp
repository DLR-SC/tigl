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
#include "WingUIDDialog.h"
#include "CCPACSWing.h"
#include "TIGLViewerDoc.h"
#include "CCPACSConfiguration.h"

IMPLEMENT_DYNAMIC(CWingUIDDialog, CDialog)

BEGIN_MESSAGE_MAP(CWingUIDDialog, CDialog)
END_MESSAGE_MAP()

CWingUIDDialog::CWingUIDDialog(CTIGLViewerDoc& document)
	: CDialog(CWingUIDDialog::IDD, 0)
	, m_document(document)
{
}

CWingUIDDialog::~CWingUIDDialog()
{
}

void CWingUIDDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WING_LIST, m_wingList);
}

int CWingUIDDialog::GetWingIndex(void) const
{
    return index;
}

std::string CWingUIDDialog::GetWingUID(void) const
{
    return wingUID;
}
void CWingUIDDialog::OnOK()
{
	CString strTemp;
	UpdateData(TRUE);
	index = m_wingList.GetCurSel();
	m_wingList.GetText(index, strTemp);
	wingUID = (char*)(LPCSTR)strTemp; 
    CDialog::OnOK();
}

BOOL CWingUIDDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize wing list
	tigl::CCPACSConfiguration& config = m_document.GetConfiguration();
	int wingCount = config.GetWingCount();
	for (int i = 1; i <= wingCount; i++) 
	{
		tigl::CCPACSWing& wing = config.GetWing(i);
		std::string name = wing.GetUID();
		if (name == "") name = "Unknown wing";
		m_wingList.InsertString(i - 1, _T(name.c_str()));
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}
