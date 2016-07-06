/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Implementation of CPACS configuration header handling routines.
*/

#ifndef CCPACSHEADER_H
#define CCPACSHEADER_H

#include "generated/CPACSHeader.h"
#include <string>
#include "tigl_internal.h"
#include "tixi.h"
#include <ctime>

namespace tigl
{

class CCPACSHeader : public generated::CPACSHeader
{
public:
    // Constructor
    TIGL_EXPORT CCPACSHeader(const std::string& aName = "", const std::string& aCreator = "", const std::time_t& aTimestamp = 0);
};

} // end namespace tigl

#endif // CCPACSHEADER_H
