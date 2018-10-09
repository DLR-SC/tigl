/* 
* Copyright (C) 2014 Airbus Defense and Space
*
* Created: 2014-09-19 Roland Landertshamer <roland.landertshamer@risc-software.at>
* Changed: $Id: CCPACSAircraftModel.h 1434 2016-04-15 15:41:53Z rlandert $ 
*
* Version: $Revision: 1434 $
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
/**
* @file 
* @brief  Implementation of Model for use as root component in CTiglUIDManager
*/

#ifndef CCPACSAIRCRAFTMODEL_H
#define CCPACSAIRCRAFTMODEL_H

#include "generated/CPACSAircraftModel.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{
class CCPACSConfiguration;

class CCPACSAircraftModel : public generated::CPACSAircraftModel, public CTiglRelativelyPositionedComponent
{
public:
    // Construct
    TIGL_EXPORT CCPACSAircraftModel(CCPACSConfiguration* config = NULL);
    TIGL_EXPORT CCPACSAircraftModel(CTiglUIDManager* config);

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE;

    TIGL_EXPORT CCPACSConfiguration& GetConfiguration() const;

    void Invalidate();

protected:
    PNamedShape BuildLoft() const OVERRIDE;

    CCPACSConfiguration* config;
};

} // end namespace tigl

#endif // CCPACSMODEL_H
