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

#include "generated/CPACSFarField.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CCPACSTransformation.h"
#include "tixi.h"
#include "tigl_internal.h"
#include "tigl_config.h"
#include "TiglFarFieldType.h"

namespace tigl
{
class CCPACSFarField : public generated::CPACSFarField, public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CCPACSFarField();

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) OVERRIDE;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE;

protected:
    PNamedShape BuildLoft() const OVERRIDE;

private:
    void init();
};

} // namespace tigl

#endif // CCPACSFARFIELD_H
