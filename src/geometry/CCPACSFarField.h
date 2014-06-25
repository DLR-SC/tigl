/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-09-03 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSFARFIELD_H
#define CCPACSFARFIELD_H

#include "CTiglAbstractGeometricComponent.h"
#include "tixi.h"
#include "tigl_internal.h"
#include "tigl_config.h"

namespace tigl
{

enum TiglFarFieldType
{
    NONE,
    HALF_SPHERE,
    FULL_SPHERE,
    HALF_CUBE,
    FULL_CUBE
};

class CCPACSFarField : public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CCPACSFarField();
    TIGL_EXPORT ~CCPACSFarField();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType(void);

    TIGL_EXPORT TiglFarFieldType GetFieldType();

protected:
    virtual PNamedShape BuildLoft(void);

private:
    void init();

    TiglFarFieldType fieldType;

    /// depending on fieldtype either cross section of sphere or sidelength of cube (reference length in cpacs)
    double fieldSize;
};

} // namespace tigl

#endif // CCPACSFARFIELD_H
