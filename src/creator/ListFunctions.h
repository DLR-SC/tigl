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

#ifndef TIGL_PATHGRAPH_H
#define TIGL_PATHGRAPH_H

#include <vector>
#include <string>

namespace tigl
{

class ListFunctions
{
public:

    // Return the elements in between the "el1" and "el2". Elements "el1" and "el2" are contained in the result.
    // Remark, the input order of "el1" and "el2" as no effect.
    // If one of the given elements is not present a empty vector is returned.
    static std::vector<std::string> GetElementsInBetween(const std::vector<std::string>& list, const std::string& el1,
                                                         const std::string& el2);


    //Return the elements after the element "el" without the element "el" itself.
    static std::vector<std::string> GetElementsAfter(const std::vector<std::string>& list, const std::string& el);


    //Return the elements before the element "el" without the element "el" itself.
    static std::vector<std::string> GetElementsBefore(const std::vector<std::string>& list, const std::string& el);

    // Return true if the element is contains in the list and false otherwise.
    static bool Contains(const std::vector<std::string>& list, const std::string& el);
};
}

#endif //TIGL_PATHGRAPH_H
