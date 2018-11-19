/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleProfile.h"

namespace tigl {

CCPACSNacelleProfile::CCPACSNacelleProfile(CTiglUIDManager* uidMgr)
    : generated::CPACSProfileGeometry2D(uidMgr)
    , pointListAlgo(*this,&CCPACSNacelleProfile::buildPointListAlgo)
{}

void CCPACSNacelleProfile::buildPointListAlgo(unique_ptr<CTiglWingProfilePointList>& cache) const
{
    cache.reset(new CTiglWingProfilePointList(*this, *m_pointList_choice1));
}

ITiglWingProfileAlgo* CCPACSNacelleProfile::GetProfileAlgo()
{
    if (m_pointList_choice1) {
        // in case the wing profile algorithm is a point list, create the additional algorithm instance
        return &**pointListAlgo;
    } else if (m_cst2D_choice2) {
        return &*m_cst2D_choice2;
    } else {
        throw CTiglError("no profile algorithm");
    }
}

const ITiglWingProfileAlgo* CCPACSNacelleProfile::GetProfileAlgo() const
{
    return const_cast<CCPACSNacelleProfile&>(*this).GetProfileAlgo();
}

} //namepsace tigl
