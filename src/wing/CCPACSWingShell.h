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

#include "generated/CPACSWingShell.h"
#include "tigl_internal.h"
#include "CCPACSWingCells.h"
#include "CCPACSMaterialDefinition.h"
#include "Cache.h"

#include "tigl.h"

#include <gp_Vec.hxx>
#include <TopoDS_Shape.hxx>

#include <string>


namespace tigl
{

// forward declarations
class CCPACSWingCSStructure;

class CCPACSWingShell : public generated::CPACSWingShell
{
public:
    TIGL_EXPORT CCPACSWingShell(CCPACSWingCSStructure* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT int GetCellCount() const;

    TIGL_EXPORT const CCPACSWingCell& GetCell(int index) const;
    TIGL_EXPORT CCPACSWingCell& GetCell(int index);

    TIGL_EXPORT const CCPACSMaterialDefinition& GetMaterial() const;
    TIGL_EXPORT CCPACSMaterialDefinition& GetMaterial();

    TIGL_EXPORT const CCPACSWingCSStructure& GetStructure() const;
    TIGL_EXPORT CCPACSWingCSStructure& GetStructure();

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT TiglLoftSide GetLoftSide() const;

private:
    //@todo stringers

    struct GeometryCache
    {
    };

    void BuildGeometry(GeometryCache& cache) const;

    Cache<GeometryCache, CCPACSWingShell> m_geometryCache;
};

} // namespace tigl

#endif // CCPACSWINGSHELL_H
