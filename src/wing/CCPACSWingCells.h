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

#include <vector>
#include "tixi.h"
#include "tigl_internal.h"

#include <string>

namespace tigl
{

class CCPACSWingCell;
class CCPACSWingShell;

class CCPACSWingCells
{
private:
    // Typedef for a CCPACSWing container to store the wings of a configuration.
    typedef std::vector<CCPACSWingCell*> CCPACSWingCellContainer;
    
public:
    // Constructor
    TIGL_EXPORT CCPACSWingCells(CCPACSWingShell* parent);
    
    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingCells(void);
    
    TIGL_EXPORT void Reset();

    TIGL_EXPORT void Invalidate();
    
    // Read CPACS wings elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& cellsXPath);
    
    // Writing CPACS wing cells
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& cellsXPath) const;
    
    // Returns the total count of wing cells for that wing component segment
    TIGL_EXPORT int GetCellCount(void) const;
    
    // Returns the wing cell for a given index.
    TIGL_EXPORT CCPACSWingCell& GetCell(int index) const;
    
    // Returns the wing cell for a given UID.
    TIGL_EXPORT CCPACSWingCell& GetCell(const std::string& UID) const;

    // Get parent wing shell element
    TIGL_EXPORT CCPACSWingShell* GetParentElement() const;

protected:
    // Cleanup routine
    void Cleanup(void);
    
private:
    // Copy constructor
    CCPACSWingCells(const CCPACSWingCells&);
    
    // Assignment operator
    void operator=(const CCPACSWingCells&);
    
private:
    CCPACSWingCellContainer cells;         /**< Wing Cell elements */
    
    CCPACSWingShell* parentShell;

};

} // end namespace tigl

#endif // CCPACSWINGCELLS_H
