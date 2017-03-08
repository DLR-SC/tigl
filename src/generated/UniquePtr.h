// Copyright (c) 2016 RISC Software GmbH
//
// This file is part of the CPACSGen runtime.
// Do not edit, all changes are lost when files are re-deployed.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>

#ifndef CPACS_GEN
#include "tigl_config.h"
#endif

namespace tigl
{
#ifdef HAVE_CPP11
    template <typename T>
    using unique_ptr = std::unique_ptr<T>;

    template<typename T, typename... Args>
    auto make_unique(Args&&... args) -> unique_ptr<T>
    {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#else
    template <typename T>
    class unique_ptr : public std::auto_ptr<T>
    {
    public:
        explicit unique_ptr(T* p = NULL) throw() : std::auto_ptr<T>(p) {}

        // NOTE: const is a hack to allow std::vector<unique_ptr<T>>
        unique_ptr(const unique_ptr& other) throw() : std::auto_ptr<T>(const_cast<unique_ptr&>(other)) {}

        template <typename U>
        unique_ptr(unique_ptr<U>& other) throw() : std::auto_ptr<T>(other) {}

        // NOTE: const is a hack to allow std::vector<unique_ptr<T>>
        unique_ptr& operator=(const unique_ptr& other) throw()
        {
            std::auto_ptr<T>::operator=(const_cast<unique_ptr&>(other));
            return *this;
        }

        template <typename U>
        unique_ptr& operator=(unique_ptr<U>& other) throw()
        {
            std::auto_ptr<T>::operator=(other);
            return *this;
        }
    };

    template <typename T>
    unique_ptr<T> make_unique()
    {
        return unique_ptr<T>(new T());
    }

    template <typename T, typename Arg1>
    unique_ptr<T> make_unique(Arg1& arg1)
    {
        return unique_ptr<T>(new T(arg1));
    }

    template <typename T, typename Arg1, typename Arg2>
    unique_ptr<T> make_unique(Arg1& arg1, Arg2& arg2)
    {
        return unique_ptr<T>(new T(arg1, arg2));
    }

    template <typename T, typename Arg1, typename Arg2, typename Arg3>
    unique_ptr<T> make_unique(Arg1& arg1, Arg2& arg2, Arg3& arg3)
    {
        return unique_ptr<T>(new T(arg1, arg2, arg3));
    }
#endif
}
