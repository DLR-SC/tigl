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
#include "FuselageUIDDialog.h"
#include "CCPACSFuselage.h"
#include "TIGLViewerDoc.h"
#include "CCPACSConfiguration.h"
#include ".\fuselageuiddialog.h"

IMPLEMENT_DYNAMIC(CFuselageUIDDialog, CDialog)

BEGIN_MESSAGE_MAP(CFuselageUIDDialog, CDialog)
	ON_LBN_SELCHANGE(IDC_FUSELAGE_LIST, OnLbnSelchangeFuselageList)
END_MESSAGE_MAP()

CFuselageUIDDialog::CFuselageUIDDialog(CTIGLViewerDoc& document)
	: CDialog(CFuselageUIDDialog::IDD, 0)
	, m_document(document)
{
}

CFuselageUIDDialog::~CFuselageUIDDialog()
{
}

void CFuselageUIDDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FUSELAGE_LIST, m_fuselageList);
}

int CFuselageUIDDialog::GetFuselageIndex(void) const
{
    return index;
}

std::string CFuselageUIDDialog::GetFuselageUID(void) const
{
    return fuselageUID;
}

void CFuselageUIDDialog::OnOK()
{
	CString strTemp;
	UpdateData(TRUE);
	index = m_fuselageList.GetCurSel();
	m_fuselageList.GetText(index, strTemp);
	fuselageUID = (char*)(LPCSTR)strTemp; 
    CDialog::OnOK();
}

BOOL CFuselageUIDDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Initialize wing list
	tigl::CCPACSConfiguration& config = m_document.GetConfiguration();
	int fuselageCount = config.GetFuselageCount();
	for (int i = 1; i <= fuselageCount; i++) 
	{
		tigl::CCPACSFuselage& fuselage = config.GetFuselage(i);
		std::string name = fuselage.GetUID();
		if (name == "") name = "Unknown fuselage";
		m_fuselageList.InsertString(i - 1, _T(name.c_str()));
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CFuselageUIDDialog::OnLbnSelchangeFuselageList()
{
	// TODO: Fügen Sie hier Ihren Kontrollbehandlungscode für die Benachrichtigung ein.
}
