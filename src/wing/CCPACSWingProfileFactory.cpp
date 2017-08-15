/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-18 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile algorithm factory
*/

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfileFactory.h"
#include "CCPACSWingProfilePointList.h"
#include "CCPACSWingProfileCST.h"


namespace tigl 
{

CCPACSWingProfileFactory& CCPACSWingProfileFactory::Instance()
{
    static CCPACSWingProfileFactory instance;
    return instance;
}

CCPACSWingProfileFactory::CCPACSWingProfileFactory()
{
}

bool CCPACSWingProfileFactory::RegisterAlgo(std::string cpacsID, CreateProfileAlgoCallback callback)
{
    profileMap[cpacsID] = callback;
    return true;
}

// Create profile algo
PTiglWingProfileAlgo CCPACSWingProfileFactory::CreateProfileAlgo(TixiDocumentHandle tixiHandle, CCPACSWingProfile& profile, const std::string & ProfileXPath)
{
    ProfileMap::iterator it = profileMap.begin();
    for (; it != profileMap.end(); ++it) {
        std::string cpacsID = it->first;
        if (tixiCheckElement(tixiHandle, std::string(ProfileXPath + "/" + cpacsID).c_str()) == SUCCESS) {
            CreateProfileAlgoCallback createProfileAlgo = it->second;
            return createProfileAlgo(profile, ProfileXPath);
        }
    }

    // no profile created
    throw CTiglError("Error: CCPACSWingProfileFactory::CreateProfileAlgo: Unable to find valid profile data", TIGL_XML_ERROR);
}

} // namespace tigl
