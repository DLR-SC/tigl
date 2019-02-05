/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef TIGL_XPATHPARSER_H
#define TIGL_XPATHPARSER_H

#include <string>

namespace cpcr
{

/**
* @brief Helper class to parse simple xpath.
*/
class XPathParser
{
public:
    // functions to retrieve information from a xpath
    static std::string GetFirstNodeType(std::string xpath);
    static std::string GetLastNodeType(std::string xpath);
    static int GetFirstNodeIndex(std::string xpath);
    static int GetLastNodeIndex(std::string xpath);

    // function to transform a xpath
    static std::string AddNodeAtEnd(std::string baseXpath, std::string nodeToAdd);

    static std::string RemoveFirstNode(std::string xpath);
    static std::string RemoveLastNode(std::string xpath);

protected:
    static std::string GetFirstNode(std::string xpath);
    static std::string GetLastNode(std::string xpath);
    // return the same string without the brackets and the content
    static std::string RemoveEndingBrackets(std::string string);
    static int GetIndexOfNode(std::string particle);
};
}

#endif //TIGL_XPATHPARSER_H
