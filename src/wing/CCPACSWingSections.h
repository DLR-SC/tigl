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
* @brief  Implementation of CPACS wing sections handling routines.
*/

#ifndef CCPACSWINGSECTIONS_H
#define CCPACSWINGSECTIONS_H

#include "generated/CPACSWingSections.h"

namespace tigl
{

class CCPACSWingSections : public generated::CPACSWingSections
{
public:
    TIGL_EXPORT CCPACSWingSections(CCPACSWing* parent, CTiglUIDManager* uidMgr);

    // Get section count
    TIGL_EXPORT int GetSectionCount() const;

    // Returns the section for a given index
    TIGL_EXPORT CCPACSWingSection& GetSection(int index) const;
};

} // end namespace tigl

#endif // CCPACSWINGSECTIONS_H
