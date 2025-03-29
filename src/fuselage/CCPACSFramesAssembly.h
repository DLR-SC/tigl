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
#include "generated/CPACSFramesAssembly.h"

namespace tigl
{

class CTiglRelativelyPositionedComponent;
class ITiglFuselageDuctStructure;

class CCPACSFramesAssembly : public generated::CPACSFramesAssembly
{
public:
    TIGL_EXPORT CCPACSFramesAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSFramesAssembly(CCPACSDuctStructure* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSFramesAssembly(CCPACSVesselStructure* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;

    // returns the (grand)-parent component (either a fuselage or a duct)
    TIGL_EXPORT CTiglRelativelyPositionedComponent const* GetParentComponent() const;

    // returns the transformation matrix of the (grand-)parent component
    TIGL_EXPORT CTiglTransformation GetTransformationMatrix() const;

    TIGL_EXPORT ITiglFuselageDuctStructure const* GetStructureInterface() const;
};

} // namespace tigl
