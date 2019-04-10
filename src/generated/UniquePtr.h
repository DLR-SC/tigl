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

namespace tigl
{
    template <typename T>
    using unique_ptr [[deprecated]] = std::unique_ptr<T>;

    template<typename T, typename... Args>
    auto make_unique(Args&&... args) -> std::unique_ptr<T>
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
