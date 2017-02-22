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

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

    TIGL_EXPORT virtual const std::string& GetUID() const override;
    TIGL_EXPORT virtual void SetUID(const std::string& uid) override;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType();

    TIGL_EXPORT TiglFarFieldType GetFieldType();
    TIGL_EXPORT void SetFieldType(const TiglFarFieldType& value);

protected:
    virtual PNamedShape BuildLoft();

private:
    void init();

    CCPACSTransformation dummyTrans; // TODO: CCPACSFarField must provide a CCPACSTransformation as it is a CTiglAbstractGeometricalComponent, is this correct? Can we remove the base class CTiglAbstractGeometricalComponent?
    TiglSymmetryAxis dummySymmetry;
};

} // namespace tigl

#endif // CCPACSFARFIELD_H
