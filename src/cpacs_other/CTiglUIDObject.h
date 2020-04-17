/*
* Copyright (c) 2020 RISC Software GmbH
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

#pragma once

#include <atomic>
#include <string>

#include <boost/optional.hpp>

#include "tigl_internal.h"


namespace tigl
{

class CTiglUIDManager;

/**
 * @brief Abstract base class of all classes providing an UID
 *
 * This class is used by the cpacs code generator as base class for all classes
 * which provide a UID. It allows to get the UID of an object without explicitly
 * knowing the current type, to navigate upwards in the CPACS tree (e.g. for
 * checking if an object is a child of another object), as well as to invalidate
 * the derived classes in a generic way.
 * The class is mainly used by the CTiglUIDManager for managing the hierarchy of
 * objects with a uid as well as for providing a list of all objects referencing
 * a specific uid.
 */
class CTiglUIDObject
{
public:
    // NVI using ReentryGuard
    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;

    virtual boost::optional<std::string> GetObjectUID() const = 0;
    virtual CTiglUIDObject* GetNextUIDParent() = 0;
    virtual const CTiglUIDObject* GetNextUIDParent() const = 0;

    virtual CTiglUIDManager& GetUIDManager() = 0;
    virtual const CTiglUIDManager& GetUIDManager() const = 0;

protected:
    ~CTiglUIDObject() = default;

private:
    virtual void InvalidateImpl(const boost::optional<std::string>& source) const {};
    TIGL_EXPORT virtual void InvalidateReferences(const boost::optional<std::string>& source) const;

    mutable std::atomic<bool> m_isInvalidating{false};
};

/**
 * @brief Specialization of CTiglUIDObject for classes with required UID
 */
class CTiglReqUIDObject : public virtual CTiglUIDObject
{
public:
    virtual const std::string& GetUID() const = 0;

    boost::optional<std::string> GetObjectUID() const {
        return GetUID();
    }
};

/**
 * @brief Specialization of CTiglUIDObject for classes with optional UID
 */
class CTiglOptUIDObject : public virtual CTiglUIDObject
{
public:
    virtual const boost::optional<std::string>& GetUID() const = 0;

    boost::optional<std::string> GetObjectUID() const {
        return GetUID();
    }
};

}
