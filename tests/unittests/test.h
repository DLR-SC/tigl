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
#include "tigl.h"
#include <stdexcept>
#include <cmath>

/*
 * Define tests here
 *      
 */


struct InTolerance
{
    InTolerance(double tol) : m_tol(tol) {}
    InTolerance(const InTolerance& other) : m_tol(other.m_tol) {}

    bool operator()(double v1, double v2) const
    {
        return fabs(v1 - v2) <= m_tol;
    }

    double m_tol;
};



template <typename Array, class Compare>
::testing::AssertionResult ArraysMatch(const Array& expected,
                                       const Array& actual,
                                       Compare is_equal)
{
    
    if (expected.size() != actual.size()) {
        return ::testing::AssertionFailure() << "Expected size (" << expected.size() << ") != actual size (" << actual.size() <<  ")";
    }
    
    
    auto it1 = std::begin(expected);
    auto it2 = std::begin(actual);
    int idx = 0;
    while(it1 != std::end(expected) && it2 != std::end(actual)){
        if (!is_equal(*it1, *it2)){
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

template <typename Array>
::testing::AssertionResult ArraysMatch(const Array& expected,
                                       const Array& actual)
{
    using T = typename Array::value_type;

    return ArraysMatch(expected, actual, [](const T& v1, const T& v2) {
        return v1 == v2;
    });
}


class TiGLTestError : public std::runtime_error
{
public:
    TiGLTestError(const std::string& what) : std::runtime_error(what)
    {}
};

class TixiHandleWrapper
{
public:
    TixiHandleWrapper(const std::string& filename)
        : tixiHandle(-1)
    {
        if (tixiOpenDocument(filename.c_str(), &tixiHandle) != SUCCESS) {
            throw TiGLTestError("Cannot open file: "+ filename);
        }
    }

    operator TixiDocumentHandle()
    {
        return tixiHandle;
    }

    ~TixiHandleWrapper()
    {
        if (tixiHandle >= 0) {
            tixiCloseDocument(tixiHandle);
        }
    }

private:
    TixiDocumentHandle tixiHandle;
};

class TiglHandleWrapper
{
public:

    TiglHandleWrapper(const std::string& filename, const std::string& config)
        : tixiHandle(filename)
    {
        if (tiglOpenCPACSConfiguration(tixiHandle, config.c_str(), &tiglHandle) != TIGL_SUCCESS) {
            throw TiGLTestError("Error opening CPACS config at: "+ filename);
        }
    }

    operator TiglCPACSConfigurationHandle()
    {
        return tiglHandle;
    }

    ~TiglHandleWrapper()
    {
        if (tiglHandle >= 0) {
            tiglCloseCPACSConfiguration(tiglHandle);
        }
    }

private:
    TixiHandleWrapper tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
};
