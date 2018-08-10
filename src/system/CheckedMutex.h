// from here: https://gist.github.com/yohhoy/296152db5f7292ca178aace6c8b010cd
// Copyright(c) 2017 yohhoy
//   adapted by RISC Software GmbH in 2018
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.



#pragma once

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "CTiglError.h"

namespace tigl {
    // mutex with recursive locking detection
    class CheckedMutex {
        boost::thread::id          _owner;
        boost::mutex               _mtx;
        boost::condition_variable  _cv;

    public:
#ifdef HAVE_CPP11
        CheckedMutex() = default;
        ~CheckedMutex() = default;
        CheckedMutex(const CheckedMutex&) = delete;
        CheckedMutex& operator=(const CheckedMutex&) = delete;
#endif

        void lock()
        {
            const boost::thread::id this_tid = boost::this_thread::get_id();
            boost::unique_lock<boost::mutex> lk(_mtx);
            if (_owner == this_tid) {
                // C++14 [thread.mutex.requirements.mutex]/p6-7
                //   The expression m.lock() shall be well-formed and have the following semantics:
                //   Requires: If m is of type std::mutex, std::timed_mutex, or std::shared_timed_mutex, the calling thread does not own the mutex.
                throw CTiglError("Recursive lock detected");
            }
            while (_owner != boost::thread::id())
                _cv.wait(lk);
            _owner = this_tid;
        }

        bool try_lock()
        {
            const auto this_tid = boost::this_thread::get_id();
            boost::lock_guard<boost::mutex> lk(_mtx);
            if (_owner == this_tid) {
                // C++14 [thread.mutex.requirements.mutex]/p14-15
                //   The expression m.try_lock() shall be well-formed and have the following semantics:
                //   Requires: If m is of type std::mutex, std::timed_mutex, or std::shared_timed_mutex, the calling thread does not own the mutex.
                throw CTiglError("Recursive try_lock detected");
            }
            if (_owner != boost::thread::id())
                return false;
            _owner = this_tid;
            return true;
        }

        void unlock()
        {
            boost::lock_guard<boost::mutex> lk(_mtx);
            if (_owner != boost::this_thread::get_id()) {
                // C++14 [thread.mutex.requirements.mutex]/p21-22
                //   The expression m.unlock() shall be well-formed and have the following semantics:
                //   Requires: The calling thread shall own the mutex.
                throw CTiglError("unlock from different thread detected");
            }
            _owner = boost::thread::id();
            _cv.notify_all();
        }
    };
}
