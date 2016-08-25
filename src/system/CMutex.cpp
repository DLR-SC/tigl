/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CMutex.h"

#if defined (_MSC_VER)
    #include <windows.h>
#else
    #include <pthread.h>
#endif

namespace tigl
{

struct CMutex::CMutex_Impl
{
#if defined (_MSC_VER)
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};

CMutex::CMutex() : pimpl_(new CMutex_Impl)
{
#if defined (_MSC_VER)
    InitializeCriticalSection(&pimpl_->mutex);
#else
    pthread_mutex_init(&pimpl_->mutex,NULL);
#endif
}

int CMutex::lock()
{
#if defined (_MSC_VER)
    EnterCriticalSection(&pimpl_->mutex);
    return 0;
#else
    return pthread_mutex_lock(&pimpl_->mutex);
#endif
}

int CMutex::unlock()
{
#if defined (_MSC_VER)
    LeaveCriticalSection(&pimpl_->mutex);
    return 0;
#else
    return pthread_mutex_unlock(&pimpl_->mutex);
#endif
}

CMutex::~CMutex()
{
#if defined (_MSC_VER)
    DeleteCriticalSection(&pimpl_->mutex);
#else
    pthread_mutex_destroy(&pimpl_->mutex);
#endif
    delete pimpl_;
}

} // namespace tigl
