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
#include "CCPACSTransformation.h"
#include "tixi.h"
#include "tigl_internal.h"
#include "tigl_config.h"
#include "TiglFarFieldType.h"

namespace tigl
{
class CCPACSFarField : public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CCPACSFarField();

    TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
    TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

    TIGL_EXPORT virtual const TiglFarFieldType& GetType() const;
    TIGL_EXPORT virtual void SetType(const TiglFarFieldType& value);

    TIGL_EXPORT virtual const double& GetReferenceLength() const;
    TIGL_EXPORT virtual void SetReferenceLength(const double& value);

    TIGL_EXPORT virtual const double& GetMultiplier() const;
    TIGL_EXPORT virtual void SetMultiplier(const double& value);

    TIGL_EXPORT std::string GetDefaultedUID() const override;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

protected:
    TiglFarFieldType m_type;
    double           m_referenceLength;
    double           m_multiplier;

    PNamedShape BuildLoft() const override;

private:
    CCPACSFarField(const CCPACSFarField&) = delete;
    CCPACSFarField& operator=(const CCPACSFarField&) = delete;

    CCPACSFarField(CCPACSFarField&&) = delete;
    CCPACSFarField& operator=(CCPACSFarField&&) = delete;

private:
    void init();
};

} // namespace tigl

#endif // CCPACSFARFIELD_H
