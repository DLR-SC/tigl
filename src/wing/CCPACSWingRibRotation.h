/* 
* Copyright (C) 2016 Airbus Defence and Space
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
#ifndef CCPACSWINGRIBROTATION_H
#define CCPACSWINGRIBROTATION_H

#include "generated/CPACSRibRotation.h"

namespace tigl
{
class CCPACSWingRibRotation : public generated::CPACSRibRotation
{
public:
    TIGL_EXPORT CCPACSWingRibRotation(CCPACSWingRibsPositioning* parent);

    TIGL_EXPORT virtual void SetRibRotationReference(const ECPACSRibRotation_ribRotationReference& value) OVERRIDE;
    TIGL_EXPORT virtual void SetRibRotationReference(const boost::optional<ECPACSRibRotation_ribRotationReference>& value) OVERRIDE;

    TIGL_EXPORT virtual void SetZ(const double& value) OVERRIDE;
};

} // end namespace tigl

#endif // CCPACSWINGRIBROTATION_H
