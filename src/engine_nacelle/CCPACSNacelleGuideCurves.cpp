/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-22 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleGuideCurves.h"
#include "CTiglError.h"

namespace tigl {


TIGL_EXPORT size_t CCPACSNacelleGuideCurves::GetGuideCurveCount() const
{
    return GetNacelleGuideCurveTypes().size();
}

TIGL_EXPORT CCPACSNacelleGuideCurve& CCPACSNacelleGuideCurves::GetGuideCurve(size_t index) const
{
    index--;
    if (index < 0 || index >= GetGuideCurveCount()) {
        throw CTiglError("Invalid index in CCPACSNacelleGuideCurves::GetGuideCurve", TIGL_INDEX_ERROR);
    }
    return *m_nacelleGuideCurveTypes[index];
}

} //namepsace tigl
