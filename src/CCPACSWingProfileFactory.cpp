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
#include "CCPACSWingProfile.h"
#include "CCPACSWingProfileFactory.h"
#include "CCPACSWingProfilePointList.h"
#include "CCPACSWingProfileCST.h"


namespace tigl 
{
    // Create profile algo
    ProfileAlgoPointer CCPACSWingProfileFactory::createProfileAlgo(TixiDocumentHandle tixiHandle, CCPACSWingProfile& profile, std::string & ProfileXPath)
    {
        // Get profile algorithm
        // Check if profile is given as a point list
        if (tixiCheckElement(tixiHandle, std::string(ProfileXPath + "/pointList").c_str()) == SUCCESS)
        {
            return ProfileAlgoPointer(new CCPACSWingProfilePointList(profile, ProfileXPath + "/pointList"));
        }
        // Check if profile is given as a CST curve
        else if (tixiCheckElement(tixiHandle, std::string(ProfileXPath + "/cst2D").c_str()) == SUCCESS)
        {
            return ProfileAlgoPointer(new CCPACSWingProfileCST(profile, ProfileXPath + "/cst2D"));
        }
        else
        {
            throw CTiglError("Error: CCPACSWingProfileFactory::CreateProfileAlgo: Unable to find valid profile data", TIGL_XML_ERROR);
        }
    }
}
