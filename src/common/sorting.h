/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-23 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef SORTING_H
#define SORTING_H

#include <algorithm>

namespace tigl
{

template<class ForwardIterator>
void rotate_right(ForwardIterator begin, ForwardIterator last)
{
    for (; begin != last; ++begin) {
        std::swap(*begin, *last);
    }
}

/**
 * Sorts an array that contains a continous number of entries, possible unordered
 */
template<class ForwardIterator, class UnaryPredicate>
void follow_sort(ForwardIterator begin, ForwardIterator end, UnaryPredicate follows)
{

    ForwardIterator sorted = begin;

    bool swapped = true;
    while (swapped) {
        swapped = false;
        for (ForwardIterator it = sorted + 1; it != end; ++it) {
            if (follows(*it, *sorted)) {
                sorted++;
                std::swap(*it, *sorted);
                swapped = true;
            }
            else if (follows(*begin, *it)) {
                sorted++;
                std::swap(*it, *sorted);
                rotate_right(begin, sorted);
                swapped = true;
            }
        }
    }

    if (sorted+1 != end) {
        throw std::invalid_argument("Vector not continous");
    }
}

}

#endif // SORTING_H
