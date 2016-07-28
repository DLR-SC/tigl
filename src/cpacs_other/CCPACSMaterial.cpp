/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSMaterial.h"

#include "CTiglError.h"
#include "CTiglLogging.h"

namespace tigl
{

CCPACSMaterial::CCPACSMaterial()
    : isvalid(false) { }

void CCPACSMaterial::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string &materialXPath)
{
    generated::CPACSMaterialDefinition::ReadCPACS(tixiHandle, materialXPath);

    if (m_compositeUID_choice1) {
        is_composite = true;
    } else if (m_materialUID_choice2) {
        is_composite = false;
    } else {
        throw CTiglError("Neither materialUID nor compositeUID specified in " + materialXPath, TIGL_ERROR);
    }

    const std::string orthoPath = materialXPath + "/orthotropyDirection";
    if (tixiCheckElement(tixiHandle, orthoPath.c_str())) {
        orthotropyDirection.construct();
        CTiglPoint& p = *orthotropyDirection;
        tixiGetPoint(tixiHandle, orthoPath.c_str(), &p.x, &p.y, &p.z);
    }

    isvalid = true;
}

TIGL_EXPORT void CCPACSMaterial::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string &materialXPath) const {
    generated::CPACSMaterialDefinition::WriteCPACS(tixiHandle, materialXPath);

    const std::string orthoPath = materialXPath + "/orthotropyDirection";
    if (orthotropyDirection) {
        const CTiglPoint& p = *orthotropyDirection;
        tixiAddPoint(tixiHandle, orthoPath.c_str(), p.x, p.y, p.z, nullptr);
    }
}

void CCPACSMaterial::Invalidate()
{
    isvalid = false;
}

bool CCPACSMaterial::isComposite() const
{
    return is_composite;
}

void CCPACSMaterial::SetComposite(bool composite)
{
    is_composite = composite;
}

bool CCPACSMaterial::IsValid() const
{
    return isvalid;
}

const std::string& CCPACSMaterial::GetUID() const
{
    if (isComposite())
        return GetCompositeUID_choice1();
    else
        return GetMaterialUID_choice2();
}

double CCPACSMaterial::GetThickness() const
{
    return GetThickness_choice2();
}

double CCPACSMaterial::GetThicknessScaling() const
{
    return GetThicknessScaling_choice1();
}

void CCPACSMaterial::SetOrthotropyDirection(tigl::CTiglPoint direction)
{
    orthotropyDirection = direction;
}

const CTiglPoint& CCPACSMaterial::GetOrthotropyDirection() const
{
    return *orthotropyDirection;
}

void CCPACSMaterial::SetUID(const std::string& uid)
{
    if (isComposite())
        return SetCompositeUID_choice1(uid);
    else
        return SetMaterialUID_choice2(uid);
}

void CCPACSMaterial::SetThickness(double thickness)
{
    SetThickness_choice2(thickness);
}

void CCPACSMaterial::SetThicknessScaling(double thicknessScaling)
{
    SetThicknessScaling_choice1(thicknessScaling);
}

} // namespace tigl
