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

#ifndef CCPACSWINGSHELL_H
#define CCPACSWINGSHELL_H

#include "tigl_internal.h"
#include "CCPACSWingCells.h"
#include "CCPACSMaterial.h"

#include <TopoDS_Shape.hxx>

#include <string>


namespace tigl 
{

class CCPACSWingShell
{
public:
    // [[CAS_AES]] added reference to parent element
    TIGL_EXPORT CCPACSWingShell();

    // [[CAS_AES]] added destructor
    TIGL_EXPORT virtual ~CCPACSWingShell();

    TIGL_EXPORT void Reset();
    
    TIGL_EXPORT const std::string& GetUID() const;

    TIGL_EXPORT int GetCellCount() const;

    TIGL_EXPORT CCPACSWingCell& GetCell(int index);

    TIGL_EXPORT CCPACSMaterial& GetMaterial();
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& shellXPath);

    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string & shellDefinitionXPath);

    TIGL_EXPORT void Invalidate();
    TIGL_EXPORT bool IsValid() const;

    // [[CAS_AES]] added method
    TIGL_EXPORT void Update();

private:
    // Information whether the shell is located on the upper or lower side of the loft
    CCPACSWingCells cells;
    CCPACSMaterial material;
    //@todo stringers

    // [[CAS_AES]] added uid
    std::string uid;
    // [[CAS_AES]] added cutting planes
    TopoDS_Shape mCuttingPlanes;

    bool isvalid;
};

} // namespace tigl

#endif // CCPACSWINGSHELL_H
