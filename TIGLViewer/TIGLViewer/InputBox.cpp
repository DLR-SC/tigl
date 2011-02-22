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
#include "InputBox.h"

HFONT InputBoxHelper::m_hFont = NULL;
HWND  InputBoxHelper::m_hWndInputBox = NULL;
HWND  InputBoxHelper::m_hWndParent = NULL;
HWND  InputBoxHelper::m_hWndEdit = NULL;
HWND  InputBoxHelper::m_hWndOK = NULL;
HWND  InputBoxHelper::m_hWndCancel = NULL;
HWND  InputBoxHelper::m_hWndPrompt = NULL;

HINSTANCE InputBoxHelper::m_hInst = NULL;


//////////////////////////////////////////////////////////////////////
// InputBoxHelper Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputBoxHelper::InputBoxHelper(HWND hWndParent)
{
	HINSTANCE hInst = GetModuleHandle(NULL);

	WNDCLASSEX wcex;

	if (!GetClassInfoEx(hInst, "InputBox", &wcex))
	{
		wcex.cbSize = sizeof(WNDCLASSEX); 

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInst;
		wcex.hIcon			= NULL;//LoadIcon(hInst, (LPCTSTR)IDI_MYINPUTBOX);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= "InputBox";
		wcex.hIconSm		= NULL;

		if (RegisterClassEx(&wcex) == 0)
			MessageBox(NULL, "Can't create InputBoxHelper!", "Error", MB_OK);
	}

    m_hWndParent = hWndParent;

    Text = NULL;

}

InputBoxHelper::~InputBoxHelper()
{
    if (Text) delete[] Text;
}


LRESULT CALLBACK InputBoxHelper::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LOGFONT lfont;

	switch (message) 
	{
		case WM_CREATE:
            // font
            memset(&lfont, 0, sizeof(lfont));
            lstrcpy(lfont.lfFaceName, _T("Arial"));
            lfont.lfHeight = 16;
            lfont.lfWeight = FW_NORMAL;//FW_BOLD;
            lfont.lfItalic = FALSE;
            lfont.lfCharSet = DEFAULT_CHARSET;
            lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
            lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lfont.lfQuality = DEFAULT_QUALITY;
            lfont.lfPitchAndFamily = DEFAULT_PITCH;
	        m_hFont = CreateFontIndirect(&lfont);

	        m_hInst = GetModuleHandle(NULL);

			// creating Edit
			m_hWndEdit = CreateWindowEx(WS_EX_STATICEDGE,
				"edit","",
				WS_VISIBLE | WS_CHILD  | WS_TABSTOP | ES_AUTOHSCROLL, 
				5, INPUTBOX_HEIGHT - 50, INPUTBOX_WIDTH - 16, 20, 
				hWnd, 
				NULL, 
				m_hInst, 
				NULL); 
		
            // setting font
			SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)m_hFont, 0);
			
            // button OK
			m_hWndOK = CreateWindowEx(WS_EX_STATICEDGE,
				"button","OK",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
				INPUTBOX_WIDTH - 100, 10, 90, 25, 
				hWnd, 
				NULL, 
				m_hInst, 
				NULL); 

            // setting font
            SendMessage(m_hWndOK, WM_SETFONT, (WPARAM)m_hFont, 0);

            // button Cancel
			m_hWndCancel = CreateWindowEx(WS_EX_STATICEDGE,
				"button","Cancel",
				WS_VISIBLE | WS_CHILD | WS_TABSTOP, 
				INPUTBOX_WIDTH - 100, 40, 90, 25, 
				hWnd, 
				NULL, 
				m_hInst, 
				NULL); 

            // setting font
            SendMessage(m_hWndCancel, WM_SETFONT, (WPARAM)m_hFont, 0);
            
            // static Propmpt
			m_hWndPrompt = CreateWindowEx(WS_EX_STATICEDGE,
				"static","",
				WS_VISIBLE | WS_CHILD, 
				5, 10, INPUTBOX_WIDTH - 110, INPUTBOX_HEIGHT - 70, 
				hWnd, 
				NULL, 
				m_hInst, 
				NULL); 

            // setting font
            SendMessage(m_hWndPrompt, WM_SETFONT, (WPARAM)m_hFont, 0);

            SetFocus(m_hWndEdit);
			break;
		case WM_DESTROY:

			DeleteObject(m_hFont);


			EnableWindow(m_hWndParent, TRUE);
			SetForegroundWindow(m_hWndParent);
			DestroyWindow(hWnd);
			PostQuitMessage(0);

			break;
        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:
                    if ((HWND)lParam == m_hWndOK)
                        PostMessage(m_hWndInputBox, WM_KEYDOWN, VK_RETURN, 0);
                    if ((HWND)lParam == m_hWndCancel)
                        PostMessage(m_hWndInputBox, WM_KEYDOWN, VK_ESCAPE, 0);
                    break;
            }
            break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


BOOL InputBoxHelper::DoModal(LPCTSTR szCaption, LPCTSTR szPrompt)
{
	RECT r;
	GetWindowRect(GetDesktopWindow(), &r);

	m_hWndInputBox = CreateWindowEx(WS_EX_TOOLWINDOW, 
                "InputBox",
                szCaption,
                WS_POPUPWINDOW | WS_CAPTION | WS_TABSTOP, 
                (r.right - INPUTBOX_WIDTH) / 2, (r.bottom - INPUTBOX_HEIGHT) / 2,
                INPUTBOX_WIDTH, INPUTBOX_HEIGHT,
                m_hWndParent,
                NULL,
                m_hInst,
                NULL);
    if(m_hWndInputBox == NULL)
        return FALSE;


    SetWindowText(m_hWndPrompt, szPrompt);

    SetForegroundWindow(m_hWndInputBox);

	EnableWindow(m_hWndParent, FALSE);

    ShowWindow(m_hWndInputBox, SW_SHOW); 
    UpdateWindow(m_hWndInputBox);

    BOOL ret = 0;

	MSG msg;

    HWND hWndFocused;

    while (GetMessage(&msg, NULL, 0, 0)) 
    {       
		if (msg.message == WM_KEYDOWN) 
		{
			if (msg.wParam == VK_ESCAPE)
            {
				SendMessage(m_hWndInputBox, WM_DESTROY, 0, 0);
                ret = 0;
            }
			if (msg.wParam == VK_RETURN)
            {
                int nCount = GetWindowTextLength(m_hWndEdit);
                nCount++;
                if (Text) 
                {
                    delete[] Text;
                    Text = NULL;
                }
                Text = new TCHAR[nCount];
                GetWindowText(m_hWndEdit, Text, nCount);
			    SendMessage(m_hWndInputBox, WM_DESTROY, 0, 0);
                ret = 1;
            }
			if (msg.wParam == VK_TAB)
            {
                hWndFocused = GetFocus();
                if (hWndFocused == m_hWndEdit) SetFocus(m_hWndOK);
                if (hWndFocused == m_hWndOK) SetFocus(m_hWndCancel);
                if (hWndFocused == m_hWndCancel) SetFocus(m_hWndEdit);
            }
            
		}
        TranslateMessage(&msg);
		DispatchMessage(&msg);      
    }

	return ret;
}
