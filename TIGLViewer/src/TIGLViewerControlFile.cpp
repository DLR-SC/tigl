/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-08 Martin Siggel Martin.Siggel@dlr.de>
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

#include "TIGLViewerControlFile.h"

#include <tixi.h>
#include <iostream>

TIGLViewerControlFile::TIGLViewerControlFile() {
    showConsole  = UNDEFINED;
    showToolbars = UNDEFINED;
}

CF_ReturnCode TIGLViewerControlFile::read(const char * filename){
    TixiDocumentHandle handle;
    if(tixiOpenDocument((char*)filename, &handle)!=SUCCESS){
        std::cerr << "Error: control file \"" << filename << "\" could not be openend" << std::endl;
        return CF_FILE_NOT_FOUND;
    }

    // read in console stuff
    const char * prefix = "/TIGLViewer/console";
    int display = 1;
    if(tixiGetBooleanAttribute(handle, prefix, "display", &display)==SUCCESS){
        if(display == 1)
            showConsole = CF_TRUE;
        else if (display == 0)
            showConsole = CF_FALSE;
        else
            showConsole = UNDEFINED;
    }

    const char * prefix2 = "/TIGLViewer/toolbars";
    display = 1;
    if(tixiGetBooleanAttribute(handle, prefix2, "display", &display)==SUCCESS){
        if(display == 1)
            showToolbars = CF_TRUE;
        else if (display == 0)
            showToolbars = CF_FALSE;
        else
            showToolbars = UNDEFINED;
    }

    return CF_SUCCESS;
}

TIGLViewerControlFile::~TIGLViewerControlFile() {
}
