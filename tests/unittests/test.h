/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-10-17 Markus Litz <Markus.Litz@dlr.de>
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
#include <gtest/gtest.h>
#include "tigl_internal.h"

/*
 * Define tests here
 *      
 */

template <typename Array>
::testing::AssertionResult ArraysMatch(const Array& expected,
                                       const Array& actual){
    
    if (expected.size() != actual.size()) {
        return ::testing::AssertionFailure() << "Expected size (" << expected.size() << ") != actual size (" << actual.size() <<  ")";
    }
    
    
    auto it1 = std::begin(expected);
    auto it2 = std::begin(actual);
    int idx = 0;
    while(it1 != std::end(expected) && it2 != std::end(actual)){
        if (*it1 != *it2){
            return ::testing::AssertionFailure() << "array[" << idx
                                                 << "] (" << *it2 << ") != expected[" << idx
                                                 << "] (" << *it1 << ")";
        }
        ++idx;
        ++it1;
        ++it2;
    }
    
    return ::testing::AssertionSuccess();
}
