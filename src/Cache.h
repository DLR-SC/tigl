/*
* Copyright (c) 2018 RISC Software GmbH
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

#include <boost/optional.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "CheckedMutex.h"

namespace tigl
{
    template <typename T>
    class ScopeLockedReference {
    public:
        ScopeLockedReference(T& reference, boost::mutex& mutex)
            : m_reference(reference), m_mutex(mutex)
        {
        }

        ~ScopeLockedReference() {
            m_mutex.unlock();
        }

        ScopeLockedReference(ScopeLockedReference&&) = default;

        T& operator*() const { return m_reference; }
        T* operator->() const { return &m_reference; }

    private:
        T& m_reference;
        boost::mutex& m_mutex;
    };

    template <typename CacheStruct, typename CpacsClass>
    class Cache
    {
    public:
        typedef void(CpacsClass::* BuildFunc)(CacheStruct&) const;

        Cache(CpacsClass& instance, BuildFunc buildFunc)
            : m_instance(instance), m_buildFunc(buildFunc)
        {
        }

        // constructs the cache and gives direct access to it
        // the access is guarded for the lifetime of the returned ScopeLockedReference
        // prefer to rely on the build function for updating the cache
        ScopeLockedReference<CacheStruct> writeAccess() {
            boost::unique_lock<boost::mutex> lock(m_mutex);
            if (!m_cache)
                m_cache.emplace();
            return ScopeLockedReference<CacheStruct>(*m_cache, *lock.release());
        }

        const CacheStruct& value() const
        {
            //boost::lock_guard<CheckedMutex> guard(m_mutex);
            boost::lock_guard<boost::mutex> guard(m_mutex);
            if (!m_cache) {
                m_cache.emplace();
                try {
                    (m_instance.*m_buildFunc)(*m_cache);
                }
                catch (...) {
                    m_cache = boost::none;
                    throw;
                }
            }
            return m_cache.value();
        }

        const CacheStruct& operator*() const { return value(); }
        const CacheStruct* operator->() const { return &value(); }

        void clear()
        {
            //boost::lock_guard<CheckedMutex> guard(m_mutex);
            boost::lock_guard<boost::mutex> guard(m_mutex);
            m_cache = boost::none;
        }

    private:
        CpacsClass& m_instance;
        BuildFunc m_buildFunc;
        //mutable CheckedMutex m_mutex;
        mutable boost::mutex m_mutex;
        mutable boost::optional<CacheStruct> m_cache;
    };
}
