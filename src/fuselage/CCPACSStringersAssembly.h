/*
* Copyright (c) 2018 RISC Software GmbH
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

#pragma once

#include <string>
#include <boost/optional.hpp>
#include "CTiglTransformation.h"
#include "generated/CPACSStringersAssembly.h"
#include "ITiglFuselageDuctStructure.h"

namespace tigl
{

class CTiglRelativelyPositionedComponent;

class CCPACSStringersAssembly : public generated::CPACSStringersAssembly
{
public:
    TIGL_EXPORT CCPACSStringersAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSStringersAssembly(CCPACSDuctStructure* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;

    // returns the (grand)-parent component (either a fuselage or a duct)
    TIGL_EXPORT CTiglRelativelyPositionedComponent const* GetParentComponent() const;

    TIGL_EXPORT CTiglTransformation GetTransformationMatrix() const;

    TIGL_EXPORT ITiglFuselageDuctStructure const* GetStructureInterface() const;
};

} // namespace tigl
