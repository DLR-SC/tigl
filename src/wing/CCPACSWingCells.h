/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSWINGCELLS_H
#define CCPACSWINGCELLS_H

#include "generated/CPACSWingCells.h"

namespace tigl
{
class CCPACSWingShell;

class CCPACSWingCells : public generated::CPACSWingCells
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingCells(CCPACSWingShell* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

    // Returns the total count of wing cells for that wing component segment
    TIGL_EXPORT int GetCellCount() const;

    // Returns the wing cell for a given index.
    TIGL_EXPORT const CCPACSWingCell& GetCell(int index) const;

    TIGL_EXPORT const CCPACSWingCell& GetCell(const std::string& UID) const;
};

} // end namespace tigl

#endif // CCPACSWINGCELLS_H
