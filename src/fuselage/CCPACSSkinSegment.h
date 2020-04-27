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

#include "generated/CPACSSkinSegment.h"
#include "CTiglStringerFrameBorderedObject.h"

namespace tigl
{
class CCPACSSkinSegment : public generated::CPACSSkinSegment, public CTiglStringerFrameBorderedObject
{
public:
    TIGL_EXPORT CCPACSSkinSegment(CCPACSSkinSegments* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT virtual void SetStartFrameUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetEndFrameUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetStartStringerUID(const std::string& value) override;
    TIGL_EXPORT virtual void SetEndStringerUID(const boost::optional<std::string>& value) override;

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;
};
} // namespace tigl
