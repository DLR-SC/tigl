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

#ifndef CMUTEX_H
#define CMUTEX_H

#include "tigl_internal.h"

namespace tigl
{

/**
 * @brief Implements a platform indepent Mutex object
 * to protect critical code areas. 
 *
 * On unixoid systems the pthread library is used, on 
 * windows the native windows treading support is used 
 * to implement the mutex.
 */
class CMutex // TODO: replace by std::mutex
{
public:
    TIGL_EXPORT CMutex();
    TIGL_EXPORT int lock();
    TIGL_EXPORT int unlock();
    TIGL_EXPORT virtual ~CMutex();
private:
    struct CMutex_Impl;
    CMutex_Impl * pimpl_;
};


} // namespace tigl

#endif // CMUTEX_H
