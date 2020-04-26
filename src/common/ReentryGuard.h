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

namespace tigl
{

struct ReentryGuard final
{
    explicit ReentryGuard(std::atomic<bool>& flag) noexcept
    : m_flag(flag) {
        bool expected{ false };
        m_locked = flag.compare_exchange_strong(expected, true);
    }

    ~ReentryGuard() noexcept {
        if (m_locked) {
            m_flag = false;
        }
    }

    ReentryGuard(const ReentryGuard&) = delete;
    ReentryGuard& operator=(const ReentryGuard&) = delete;
    ReentryGuard(ReentryGuard&&) noexcept = delete;
    ReentryGuard& operator=(ReentryGuard&&) noexcept = delete;

    explicit operator bool() const noexcept {
        return locked();
    }

    bool operator!() const noexcept {
        return !locked();
    }

    bool locked() const noexcept {
        return m_locked;
    }

private:
    std::atomic<bool>& m_flag;
    bool m_locked{false};
};

}
