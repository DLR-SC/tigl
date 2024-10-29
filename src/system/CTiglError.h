/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file 
* @brief  Exception class used to throw tigl exceptions.
*/

#ifndef CTIGLERROR_H
#define CTIGLERROR_H

#include "tigl_internal.h"
#include "tigl.h"
#include <string>
#include <exception>

namespace tigl 
{

class CTiglError : public std::exception
{

public:
    // Constructor
    TIGL_EXPORT CTiglError(std::string error = "", TiglReturnCode errorCode = TIGL_ERROR) noexcept;

    // Destructor
    TIGL_EXPORT ~CTiglError() noexcept override;

    // Default copy constructor and assignment operator are correct since
    // memberwise copy is enough for this class.

    TIGL_EXPORT const char* what() const noexcept override;

    // Returns the error code
    TIGL_EXPORT virtual TiglReturnCode getCode() const noexcept;

private:
    std::string    err;
    TiglReturnCode code;
    
};

} // end namespace tigl

#endif // CTIGLERROR_H
