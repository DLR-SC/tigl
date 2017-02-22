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
#include "CTiglAbstractPhysicalComponent.h"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSRotorcraftModel : public generated::CPACSRotorcraftModel, public CTiglAbstractPhysicalComponent
{
public:
    // Construct
    TIGL_EXPORT CCPACSRotorcraftModel();
    TIGL_EXPORT CCPACSRotorcraftModel(CCPACSConfiguration* config);

    TIGL_EXPORT virtual const std::string& GetUID() const override;
    TIGL_EXPORT virtual void SetUID(const std::string& uid) override;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT TiglGeometricComponentType GetComponentType();

    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    void Invalidate();

protected:
    PNamedShape BuildLoft();

    CCPACSTransformation dummyTrans; // TODO: CCPACSRotorcraftModel must provide a CCPACSTransformation as it is a CTiglAbstractGeometricalComponent, is this correct? Can we remove the base class CTiglAbstractGeometricalComponent?
    TiglSymmetryAxis dummySymmetry;
    CCPACSConfiguration* config;
};

} // end namespace tigl

#endif // CCPACSMODEL_H
