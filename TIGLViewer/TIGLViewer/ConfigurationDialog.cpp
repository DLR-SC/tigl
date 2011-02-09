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
#include "ConfigurationDialog.h"
#include "CCPACSFuselage.h"
#include "TIGLViewerDoc.h"
#include "CCPACSConfiguration.h"

IMPLEMENT_DYNAMIC(CConfigurationDialog, CDialog)

BEGIN_MESSAGE_MAP(CConfigurationDialog, CDialog)
END_MESSAGE_MAP()

CConfigurationDialog::CConfigurationDialog(std::vector<char*>* configVector)
	: CDialog(CConfigurationDialog::IDD, 0)
{
	configurations = configVector;
}

CConfigurationDialog::~CConfigurationDialog()
{
}

void CConfigurationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIGURATION_LIST, m_configurationList);
}

int CConfigurationDialog::GetConfigurationIndex(void) const
{
    return index;
}

void CConfigurationDialog::OnOK()
{
	UpdateData(TRUE);
	index = m_configurationList.GetCurSel();
    CDialog::OnOK();
}

BOOL CConfigurationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();


	for (std::vector<char*>::size_type i = 0; i < configurations->size(); i++)
    {
		m_configurationList.InsertString(i - 1, configurations->at(i));
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}
