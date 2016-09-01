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

#ifndef CCPACSWINGPROFILEFACTORY_H
#define CCPACSWINGPROFILEFACTORY_H
#include <string>
#include <map>
#include "tixi.h"
#include "tigl_internal.h"
#include "PTiglWingProfileAlgo.h"

namespace tigl 
{

class CCPACSWingProfile;
class ITiglWingProfileAlgo;

// registration function
typedef PTiglWingProfileAlgo (*CreateProfileAlgoCallback)(const CCPACSWingProfile& profile, const std::string& cpacsPath);

class CCPACSWingProfileFactory
{
public:
    static CCPACSWingProfileFactory& Instance();

    bool RegisterAlgo(std::string cpacsID, CreateProfileAlgoCallback);

    TIGL_EXPORT PTiglWingProfileAlgo CreateProfileAlgo(TixiDocumentHandle tixiHandle, CCPACSWingProfile& profile, const std::string & ProfileXPath);
private:
    typedef std::map<std::string, CreateProfileAlgoCallback> ProfileMap;
    ProfileMap profileMap;

    CCPACSWingProfileFactory();
};

}
# endif // CCPACSWINGPROFILEFACTORY_H
