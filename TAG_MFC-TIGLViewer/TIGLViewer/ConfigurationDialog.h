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

#include "afxwin.h"
#include <vector>

class CTIGLViewerDoc;
namespace tigl 
{
    class CCPACSConfiguration; 
}

// CWingDialog-Dialogfeld
class CConfigurationDialog : public CDialog
{
public:

	CConfigurationDialog(std::vector<char*>* configVector); 
	virtual ~CConfigurationDialog();
	virtual BOOL OnInitDialog();
    int GetConfigurationIndex(void) const;

	enum { IDD = IDD_CONFIGURATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	virtual void OnOK();

private:
	CListBox        m_configurationList;
    int             index;
	std::vector<char*>*	configurations;

	DECLARE_DYNAMIC(CConfigurationDialog)
	DECLARE_MESSAGE_MAP()
};
