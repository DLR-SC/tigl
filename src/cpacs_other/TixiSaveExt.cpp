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
* @file TixiSaveExt.cpp
* @brief  Routines for writing XML elements and attributes with TiXI
*/

#include "TixiSaveExt.h"

namespace tigl
{

// create if necessary the element
void TixiSaveExt::TixiSaveElement(TixiDocumentHandle tixiHandle, const char* elementPath, const char* element)
{
    // check if the element exist
    std::string subpath;
    subpath.append(elementPath);
    subpath.append("/");
    subpath.append(element);

    // if the element doesn't exist already, it create it
    if (tixiCheckElement(tixiHandle, subpath.c_str()) == ELEMENT_NOT_FOUND) {
        if (tixiCreateElement(tixiHandle, elementPath, element) != SUCCESS) {
            throw CTiglError("Error in TixiSaveExt::TixiSaveElement : tixiCreateElement error", TIGL_XML_ERROR);
        }
    }
    else if (tixiCheckElement(tixiHandle, subpath.c_str()) != SUCCESS) {
        throw CTiglError("Error in TixiSaveExt::TixiSaveElement : inexistant path", TIGL_XML_ERROR);
    }
}

// update an element with text, and create it if necessary
// INDICATION : updating an empty text doesn't work : no error, but the text is not updated
void TixiSaveExt::TixiSaveTextElement(TixiDocumentHandle tixiHandle, const char* elementPath, const char* element, const char* text)
{
    std::string subpath;
    subpath.append(elementPath);
    subpath.append("/");
    subpath.append(element);

    // check if the element exist
    if (tixiCheckElement(tixiHandle, elementPath) != SUCCESS) {
        throw CTiglError("Error in TixiSaveExt::TixiSaveTextElement : inexistant path", TIGL_XML_ERROR);
    }

    // first, delete the element
    if (tixiCheckElement(tixiHandle, subpath.c_str()) == SUCCESS) {
        if (tixiRemoveElement(tixiHandle, subpath.c_str()) != SUCCESS) {
            throw CTiglError("Error: XML error while removing text in TixiSaveExt::TixiSaveTextElement", TIGL_XML_ERROR);
        }
    }

    // then, the new element is created with the text
    if (tixiAddTextElement(tixiHandle, elementPath, element, text) != SUCCESS) {
        throw CTiglError("Error in TixiSaveExt::TixiSaveTextElement : tixiAddTextElement error", TIGL_XML_ERROR);
    }

}


// update an attribute, or create it if necessary
void TixiSaveExt::TixiSaveTextAttribute(TixiDocumentHandle tixiHandle, const char* attributePath, const char* attribute, const char* text)
{
    ReturnCode tixiRet;
    // check is it exist, and update it
    if (tixiCheckElement(tixiHandle, attributePath) == SUCCESS) {
        if ((tixiRet = tixiAddTextAttribute(tixiHandle, attributePath, attribute, text)) != SUCCESS) {
            throw CTiglError("Error in TixiSaveExt::TixiSaveTextAttribute : tixiAddTextAttribute error", TIGL_XML_ERROR);
        }
    }
    else {
        throw CTiglError("Error in TixiSaveExt::TixiSaveTextAttribute : inexistant path", TIGL_XML_ERROR);
    }
}



// update a point coordinates, or create it if necessary
void TixiSaveExt::TixiSavePoint(TixiDocumentHandle tixiHandle, const char* elementPath, double x, double y, double z, const char* format)
{
    ReturnCode tixiRet;
    std::string subpath;

    // approximate values of the point
    if (fabs(x) < 10e-7) {
        x = 0;
    }
    if (fabs(y) < 10e-7) {
        y = 0;
    }
    if (fabs(z) < 10e-7) {
        z = 0;
    }

    // check if the element exist
    if (tixiCheckElement(tixiHandle, elementPath) != SUCCESS) {
        throw CTiglError("Error in TixiSaveExt::TixiSavePoint : inexistant path", TIGL_XML_ERROR);
    }

    subpath.append(elementPath);
    subpath.append("/x");
    // check if the sublement exist, and update it if yes
    if ((tixiRet = tixiCheckElement(tixiHandle, subpath.c_str())) == SUCCESS) {
        if (tixiUpdateDoubleElement(tixiHandle, subpath.c_str(), x, format) != SUCCESS) {
            throw CTiglError("Error: XML error while updating point x in TixiSaveExt::TixiSavePoint", TIGL_XML_ERROR);
        }
        subpath = "";
        subpath.append(elementPath);
        subpath.append("/y");
        if (tixiUpdateDoubleElement(tixiHandle, subpath.c_str(), y, format) != SUCCESS) {
            throw CTiglError("Error: XML error while updating point y in TixiSaveExt::TixiSavePoint", TIGL_XML_ERROR);
        }
        subpath = "";
        subpath.append(elementPath);
        subpath.append("/z");
        if (tixiUpdateDoubleElement(tixiHandle, subpath.c_str(), z, format) != SUCCESS) {
            throw CTiglError("Error: XML error while updating point z in TixiSaveExt::TixiSavePoint", TIGL_XML_ERROR);
        }
    }
    // else, it create it
    else if (tixiCheckElement(tixiHandle, subpath.c_str()) == ELEMENT_NOT_FOUND) {
        if (tixiAddPoint(tixiHandle, elementPath, x, y, z, format) != SUCCESS) {
            throw CTiglError("Error: XML error while creating a point in TixiSaveExt::TixiSavePoint", TIGL_XML_ERROR);
        }
    }
    else {
        throw CTiglError("Error: XML error while creating a point in TixiSaveExt::TixiSavePoint", TIGL_XML_ERROR);
    }
}

// Update a element with double
void TixiSaveExt::TixiSaveDoubleElement(TixiDocumentHandle tixiHandle, const char* elementPath, const char* element, double val, const char* format)
{
    std::string subpath;
    subpath.append(elementPath);
    subpath.append("/");
    subpath.append(element);

    // approximate value
    if (fabs(val) < 10e-12) {
        val = 0.;
    }

    // check if the element exist
    if (tixiCheckElement(tixiHandle, elementPath) != SUCCESS) {
        throw CTiglError("Error in TixiSaveExt::TixiSaveDoubleElement : inexistant path", TIGL_XML_ERROR);
    }

    // if it exist, it update it
    if (tixiCheckElement(tixiHandle, subpath.c_str()) == SUCCESS) {
        if (tixiUpdateDoubleElement(tixiHandle, subpath.c_str(), val, format) != SUCCESS) {
            throw CTiglError("Error: XML error while updating text in TixiSaveExt::TixiSaveDoubleElement", TIGL_XML_ERROR);
        }
    }
    // else, it create it.
    else if (tixiCheckElement(tixiHandle, subpath.c_str()) == ELEMENT_NOT_FOUND) {
        if (tixiAddDoubleElement(tixiHandle, elementPath, element, val, format) != SUCCESS) {
            throw CTiglError("Error in TixiSaveExt::TixiSaveDoubleElement : tixiAddTextElement error", TIGL_XML_ERROR);
        }
    }
    else {
        throw CTiglError("Error in TixiSaveExt::TixiSaveDoubleElement : tixiCheckElement error", TIGL_XML_ERROR);
    }
}

// update a point coordinates, or create it if necessary
void TixiSaveExt::TixiSaveVector(TixiDocumentHandle tixiHandle, const char* elementPath, const char* element, const double* vector, int length)
{
    std::string subpath;
    subpath.append(elementPath);
    subpath.append("/");
    subpath.append(element);

    // check if element exist, if yes, it destroy it
    if (tixiCheckElement(tixiHandle, subpath.c_str()) == SUCCESS) {
        if (tixiRemoveElement(tixiHandle, subpath.c_str()) != SUCCESS) {
            throw CTiglError("Error: XML error while removing vector in TixiSaveExt::TixiSaveVector", TIGL_XML_ERROR);
        }
    }
    // set the new vector with new values
    if (tixiAddFloatVector(tixiHandle, elementPath, element, vector, length, NULL) != SUCCESS) {
        throw CTiglError("Error: XML error while saving vector in TixiSaveExt::TixiSaveVector", TIGL_XML_ERROR);
    }
}

// update a point coordinates, or create it if necessary
void TixiSaveExt::TixiSaveVector(TixiDocumentHandle tixiHandle, const std::string& elementPath, const std::string& element, const std::vector<double>& val)
{
    if (val.empty()) {
        TixiSaveVector(tixiHandle, elementPath.c_str(), element.c_str(), NULL, 0);
    }
    else {
        TixiSaveVector(tixiHandle, elementPath.c_str(), element.c_str(), &val[0], val.size());
    }
}


}
// end of file

