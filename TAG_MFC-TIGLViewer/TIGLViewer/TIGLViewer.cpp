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

#include "TIGLViewerDoc.h"
#include "OCC_MainFrame.h"
#include "OSD_Environment.hxx"
#include "TIGLViewerChildFrame.h"
#include "TIGLViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTIGLViewerApp

BEGIN_MESSAGE_MAP(CTIGLViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standarddruckbefehl "Seite einrichten"
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


// CTIGLViewerApp-Erstellung
CTIGLViewerApp::CTIGLViewerApp()
{
    //OSD_Environment theEnv("CALL_OPENGL_ANTIALIASING_MODE", "0");
    //theEnv.Build();
}

// Das einzige CTIGLViewerApp-Objekt
CTIGLViewerApp theApp;

// CTIGLViewerApp Initialisierung
BOOL CTIGLViewerApp::InitInstance()
{
	// InitCommonControls() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	InitCommonControls();

	CWinApp::InitInstance();

	// OLE-Bibliotheken initialisieren
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standardinitialisierung
	// Wenn Sie diese Features nicht verwenden und die Größe
	// der ausführbaren Datei verringern möchten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// Ändern Sie den Registrierungsschlüssel unter dem Ihre Einstellungen gespeichert sind.
	// TODO: Ändern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("TIGLViewer"));
	LoadStdProfileSettings(4);  // Standard INI-Dateioptionen laden (einschließlich MRU)

	// Dokumentvorlagen der Anwendung registrieren. Dokumentvorlagen
	//  dienen als Verbindung zwischen Dokumenten, Rahmenfenstern und Ansichten.
	m_ptrTIGLView = new CMultiDocTemplate(
		IDR_TIGLCHILDFRAME,
		RUNTIME_CLASS(CTIGLViewerDoc),
		RUNTIME_CLASS(CTIGLViewerChildFrame),
		RUNTIME_CLASS(CTIGLViewerView));
	if (m_ptrTIGLView == 0)
		return FALSE;
	AddDocTemplate(m_ptrTIGLView);

	// create main MDI Frame window
	OCC_MainFrame* pMainFrame = new OCC_MainFrame(with_AIS_TB);
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Befehlszeile parsen, um zu prüfen auf Standardumgebungsbefehle DDE, Datei offen
	CCommandLineInfo cmdInfo;

	// Turn off default OnFileNew() call
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	ParseCommandLine(cmdInfo);
	// Verteilung der in der Befehlszeile angegebenen Befehle. Es wird FALSE zurückgegeben, wenn
	// die Anwendung mit /RegServer, /Register, /Unregserver oder /Unregister gestartet wurde.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Das einzige Fenster ist initialisiert und kann jetzt angezeigt und aktualisiert werden.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// Rufen Sie DragAcceptFiles nur auf, wenn eine Suffix vorhanden ist.
	//  In einer SDI-Anwendung ist dies nach ProcessShellCommand erforderlich

	return TRUE;
}

// CTIGLViewerApp File Open Meldungshandler
void CTIGLViewerApp::OnFileOpen()
{
	TCHAR szFilters[] = _T("CPACS files (*.xml)|*.xml|All files (*.*)|*.*||");

	CFileDialog dlg (TRUE, _T("xml"), _T("*.xml"), OFN_FILEMUSTEXIST, szFilters);
	if (dlg.DoModal () == IDOK) {
		CString filename = dlg.GetPathName ();
		OpenDocumentFile(filename);
	}
}


//******************************************************************************************************

// CAboutDlg-Dialogfeld für Anwendungsbefehl 'Info'
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialogfelddaten
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCpacsDrawfusedall();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_COMMAND(ID_CPACS_DRAWFUSEDALL, OnCpacsDrawfusedall)
END_MESSAGE_MAP()

// Anwendungsbefehl zum Ausführen des Dialogfelds
void CTIGLViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CAboutDlg::OnCpacsDrawfusedall()
{
	// TODO: Fügen Sie hier Ihren Befehlsbehandlungscode ein.
}
