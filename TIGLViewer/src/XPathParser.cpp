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

#include "XPathParser.h"
#include <sstream>
#include "CTiglLogging.h"

std::string cpcr::XPathParser::GetFirstNode(std::string xpath)
{

    // case when the xPath start with a /
    if (xpath.size() >= 1 && xpath[0] == '/') {
        xpath.erase(0, 1);
    }

    std::istringstream f(xpath);
    std::string first;
    getline(f, first, '/');

    return first;
}

std::string cpcr::XPathParser::GetLastNode(std::string xpath)
{

    // if the temp ended with '/', we first remove it
    if (xpath[xpath.size() - 1] == '/') {
        xpath.erase(xpath.size() - 1, 1);
    }

    int found = xpath.find_last_of("/");
    if (found < 0) {
        return xpath;
    }
    std::string r = xpath.substr(found + 1);
    return r;
}

std::string cpcr::XPathParser::RemoveEndingBrackets(std::string string)
{
    if (string[string.size() - 1] == ']') {
        long int found = string.find_last_of('[');
        if (found < 0) {
            LOG(ERROR) << "XPathParser: RemoveEndingBrackets:  Invalid input: " + string;
        }
        else {
            string.erase(found, string.size() - found);
        }
    }
    return string;
}

int cpcr::XPathParser::GetIndexOfNode(std::string particle)
{

    int r = 1; // be default the index is one

    if (particle[particle.size() - 1] == ']') {
        long int found = particle.find_last_of('[');
        if (found < 0) {
            LOG(ERROR) << "XPathParser: GetIndexOfNode:  Invalid input: " + particle;
        }
        else {
            particle.erase(particle.size() - 1, 1); // erase last
            particle.erase(0, found + 1);
            r = std::stoi(particle);
        }
    }
    return r;
}

std::string cpcr::XPathParser::GetFirstNodeType(std::string xpath)
{
    std::string node = GetFirstNode(xpath);
    std::string type = RemoveEndingBrackets(node);
    return type;
}

std::string cpcr::XPathParser::GetLastNodeType(std::string xpath)
{
    std::string node = GetLastNode(xpath);
    std::string type = RemoveEndingBrackets(node);
    return type;
}

int cpcr::XPathParser::GetFirstNodeIndex(std::string xpath)
{

    std::string node = GetFirstNode(xpath);
    int idx          = GetIndexOfNode(node);
    return idx;
}

int cpcr::XPathParser::GetLastNodeIndex(std::string xpath)
{
    std::string node = GetLastNode(xpath);
    int idx          = GetIndexOfNode(node);
    return idx;
}

std::string cpcr::XPathParser::AddNodeAtEnd(std::string baseXpath, std::string nodeToAdd)
{

    // Remove the "/" at the end of the new node if there is some
    if (nodeToAdd[nodeToAdd.size() - 1] == '/') {
        nodeToAdd.erase(nodeToAdd.size() - 1, 1);
    }

    // Add the "/" between the two element if needed
    if (baseXpath[baseXpath.size() - 1] != '/' && nodeToAdd[0] != '/' && baseXpath.size() > 0) {
        baseXpath = baseXpath + '/';
    }

    return baseXpath + nodeToAdd;
    ;
}

std::string cpcr::XPathParser::RemoveFirstNode(std::string xpath)
{
    // case when the xPath start with a /
    if (xpath.size() >= 1 && xpath[0] == '/') {
        xpath.erase(0, 1);
    }

    std::string firstNode = GetFirstNode(xpath);

    // update xpath ( remove temp and /)
    if (xpath.size() == firstNode.size()) {
        return "";
    }
    else {
        xpath.erase(0, firstNode.size() + 1);
        return xpath;
    }
}

std::string cpcr::XPathParser::RemoveLastNode(std::string xpath)
{

    // if the xpath ended with '/', we first remove it
    if (xpath[xpath.size() - 1] == '/') {
        xpath.erase(xpath.size() - 1, xpath.size());
    }

    std::string r;
    int found = xpath.find_last_of("/");
    if (found < 0) {
        r     = xpath;
        xpath = "";
    }
    else {
        r = xpath.substr(found + 1);
        xpath.erase(found, r.size() + 1);
    }
    return xpath;
}
