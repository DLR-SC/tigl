/* 
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
* @brief  Implementation of Model for use as root component in CTiglUIDManager
*/

#ifndef CCPACSROTORCRAFTMODEL_H
#define CCPACSROTORCRAFTMODEL_H

#include "generated/CPACSRotorcraftModel.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSRotorcraftModel : public generated::CPACSRotorcraftModel, public CTiglRelativelyPositionedComponent
{
public:
    // Construct
    TIGL_EXPORT CCPACSRotorcraftModel(CCPACSConfiguration* config = NULL);
    TIGL_EXPORT CCPACSRotorcraftModel(CTiglUIDManager* uidMgr);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE;

    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    void Invalidate();

protected:
    PNamedShape BuildLoft() const OVERRIDE;

    CCPACSConfiguration* config;
};

} // end namespace tigl

#endif // CCPACSMODEL_H
