/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "ListFunctions.h"
#include <algorithm>

std::vector<std::string> tigl::ListFunctions::GetElementsInBetween(const std::vector<std::string>& list,
                                                                   const std::string& el1, const std::string& el2)
{

    std::vector<std::string> elementsBetween; // contain the start and the end

    if (!(ListFunctions::Contains(list, el1) && ListFunctions::Contains(list, el2))) {
        return elementsBetween;
    }

    bool afterStart = false;
    bool afterEnd   = false;

    for (int i = 0; i < list.size(); i++) {
        // cover the special case where the end and the start are the same
        if (list[i] == el1 && list[i] == el2) {
            afterStart = true;
            afterEnd   = true;
            elementsBetween.push_back(list[i]);
        }
        // we allow the start and end to be inverted
        else if (afterStart == false && (list[i] == el1 || list[i] == el2)) {
            afterStart = true;
            elementsBetween.push_back(list[i]);
        }
        else if (afterStart == true && (list[i] == el1 || list[i] == el2)) {
            afterEnd = true;
            elementsBetween.push_back(list[i]);
        }
        else if (afterStart == true && afterEnd == false) {
            elementsBetween.push_back(list[i]);
        }
    }
    return elementsBetween;
}

std::vector<std::string> tigl::ListFunctions::GetElementsAfter(const std::vector<std::string>& list,
                                                               const std::string& el)
{
    std::vector<std::string> elementsAfter;

    bool after = false;

    std::vector<std::string>::iterator it;
    for (int i = 0; i < list.size(); i++) {
        if (after) {
            elementsAfter.push_back(list[i]);
        }
        if (list[i] == el) {
            after = true;
        }
    }
    return elementsAfter;
}



std::vector<std::string> tigl::ListFunctions::GetElementsBefore(const std::vector<std::string> &list,
                                                                const std::string &el)
{
    std::vector<std::string> elementsBefore;

    bool before = true;

    std::vector<std::string>::iterator it;
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == el) {
            before = false;
        }
        if (before) {
            elementsBefore.push_back(list[i]);
        }
    }
    return elementsBefore;
}


bool tigl::ListFunctions::Contains(const std::vector<std::string>& graph, const std::string& el)
{

    if (std::find(graph.begin(), graph.end(), el) != graph.end()) {
        return true;
    }
    else {
        return false;
    }
}