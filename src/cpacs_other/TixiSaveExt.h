/* 
* Copyright (C) 2013 Cassidian Air Systems
*
* Created: 2013-03-11 Pierre Thomazo
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
/**
* @file TixiSaveExt.h
* @brief  Routines for writing XML elements and attributes with TiXI
*
* This file is an extension to the tixi functions, in order to simplify the xml file saving.
* These functions have all the same procedure :
* - check the existence of the element parameter
* - create it if necessary
* - update or set the value
* The functions internally use the according tixi functions.
* If an error occurs, a TIGL_XML_ERROR is thrown.
*/

#ifndef TIXISAVEEXT_H
#define TIXISAVEEXT_H

#include <vector>

#include "tixi.h"
#include "tigl.h"
#include "CTiglPoint.h"
#include "CTiglError.h"
#include "math.h"

namespace tigl
{
namespace TixiSaveExt
{
    /** Save an empty element */
    void TixiSaveElement(TixiDocumentHandle tixiHandle, const char* elementPath, const char* element);

    /** Save an element containing 'text' */
    void TixiSaveTextElement(TixiDocumentHandle tixiHandle, const char* elementXPath, const char* element, const char* text);

    /** Save an attribute at the path */
    void TixiSaveTextAttribute(TixiDocumentHandle tixiHandle, const char* attributePath, const char* attribute, const char* text);

    /** Save a point (ref,x,y,z) at the path */
    void TixiSavePoint(TixiDocumentHandle tixiHandle, const char* elementXPath, double x, double y, double z, const char* format);

    /** Save an element containing a double value */
    void TixiSaveDoubleElement(TixiDocumentHandle tixiHandle, const char* elementXPath, const char* element, double val, const char* format = "%g");

    /** Save an element containing a int value */
    void TixiSaveIntElement(TixiDocumentHandle tixiHandle, const char* elementXPath, const char* element, int val);

    /** Save an element containing a vector (list of double value) at the path */
    void TixiSaveVector(TixiDocumentHandle tixiHandle, const char* elementXPath, const char* element, const double* val, std::size_t length);

    /** Save an element containing a vector at the path */
    void TixiSaveVector(TixiDocumentHandle tixiHandle, const std::string& elementXPath, const std::string& element, const std::vector<double>& val);
};
}
#endif
