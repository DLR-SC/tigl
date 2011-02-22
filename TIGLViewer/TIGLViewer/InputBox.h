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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define INPUTBOX_WIDTH 400
#define INPUTBOX_HEIGHT 125


class InputBoxHelper  
{
    static HFONT m_hFont;
    static HWND  m_hWndInputBox;
    static HWND  m_hWndParent;
    static HWND  m_hWndEdit;
    static HWND  m_hWndOK;
    static HWND  m_hWndCancel;
    static HWND  m_hWndPrompt;

    static HINSTANCE m_hInst;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
    // text from InputBox
	LPTSTR Text;
    BOOL DoModal(LPCTSTR szCaption, LPCTSTR szPrompt);

	InputBoxHelper(HWND hWndParent);
	virtual ~InputBoxHelper();

};

