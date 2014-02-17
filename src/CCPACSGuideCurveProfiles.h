/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS guide curve profiles handling routines
*/

#ifndef CCPACSGUIDECURVEPROFILES_H
#define CCPACSGUIDECURVEPROFILES_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CSharedPtr.h"
#include "CCPACSGuideCurveProfile.h"
#include <string>
#include <map>

namespace tigl
{

typedef CSharedPtr<CCPACSGuideCurveProfile> PCCPACSGuideCurveProfile;

class CCPACSGuideCurveProfiles
{

private:
    // Typedef for a container to store the guide curves.
    typedef std::map<std::string, PCCPACSGuideCurveProfile> CCPACSGuideCurveProfileContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSGuideCurveProfiles(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSGuideCurveProfiles(void);

    // Read CPACS guide curves
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the total count of guide curves in this configuration
    TIGL_EXPORT int GetGuideCurveProfileCount(void) const;

    // Returns the guide curve for a given uid.
    TIGL_EXPORT CCPACSGuideCurveProfile& GetGuideCurveProfile(std::string uid) const;

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSGuideCurveProfiles(const CCPACSGuideCurveProfiles&)
    {
        /* Do nothing */
    }

    // Assignment operator
    void operator=(const CCPACSGuideCurveProfiles&)
    {
        /* Do nothing */
    }

private:
    CCPACSGuideCurveProfileContainer guideCurves;     // All guide curves

};

} // end namespace tigl

#endif // CCPACSGUIDECURVEPROFILES_H

