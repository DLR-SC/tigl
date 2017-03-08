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

#include "CCPACSWingSparPosition.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingSpars.h"
#include "CTiglError.h"


namespace tigl
{

CCPACSWingSparPosition::CCPACSWingSparPosition(CCPACSWingSparPositions* sparPositions)
: generated::CPACSSparPosition(sparPositions)
{
}

CCPACSWingSparPosition::InputType CCPACSWingSparPosition::GetInputType() const
{
    if (m_eta_choice1)
        return ENUM_VALUE(InputType, Eta);
    if (m_elementUID_choice2)
        return ENUM_VALUE(InputType, ElementUID);
    throw std::logic_error("Invalid input type");
}

const std::string& CCPACSWingSparPosition::GetElementUID() const
{
    if (!m_elementUID_choice2) {
        throw CTiglError("SparPosition is not defined via elementUID. Please check InputType first before calling CCPACSWingSparPosition::GetElementUID()");
    }
    return *m_elementUID_choice2;
}

void CCPACSWingSparPosition::SetElementUID(const std::string& uid)
{
    m_elementUID_choice2 = uid;

    m_eta_choice1 = boost::none;

    // invalidate whole component segment structure, since ribs or cells could reference the spar
    GetParent()->GetParent()->GetParent()->Invalidate();
}

double CCPACSWingSparPosition::GetEta() const
{
    if (!m_eta_choice1) {
        throw CTiglError("SparPosition is not defined via eta. Please check InputType first before calling CCPACSWingSparPosition::GetEta()");
    }
    return *m_eta_choice1;
}

void CCPACSWingSparPosition::SetEta(double value)
{
    m_eta_choice1 = value;
   
    m_elementUID_choice2 = boost::none;

    // invalidate whole component segment structure, since ribs or cells could reference the spar
    GetParent()->GetParent()->GetParent()->Invalidate();
}

} // end namespace tigl
