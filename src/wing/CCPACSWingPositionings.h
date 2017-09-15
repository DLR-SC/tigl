/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
* @brief  Implementation of CPACS wing positionings handling routines.
*/

#ifndef CCPACSWINGPOSITIONINGS_H
#define CCPACSWINGPOSITIONINGS_H

#include "tixi.h"
#include "tigl_internal.h"
#include "CCPACSWingPositioning.h"
#include "CTiglError.h"
#include <string>
#include <vector>
#include <map>

namespace tigl
{

class CCPACSWingPositionings
{

public:
    // Typedef for a CCPACSWingPositioning container to store the positionings.
    typedef std::map<std::string, CCPACSWingPositioning*> CCPACSWingPositioningContainer;
    typedef CCPACSWingPositioningContainer::iterator CCPACSWingPositioningIterator;

public:
    // Constructor
    TIGL_EXPORT CCPACSWingPositionings(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingPositionings(void);

    // Read CPACS positionings element
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath);

    // Write CPACS positionings element
    TIGL_EXPORT void WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath) const;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(void);

    // Returns the positioning matrix for a given section-uid
    TIGL_EXPORT CTiglTransformation GetPositioningTransformation(std::string sectionIndex);

    // Cleanup routine
    TIGL_EXPORT void Cleanup(void);

    TIGL_EXPORT CCPACSWingPositioningContainer& GetPositionings();

protected:
    // Update internal positioning structure
    void Update(void);

    // Update connected positionings recursive
    void UpdateNextPositioning(CCPACSWingPositioning*, int depth);

private:
    // Copy constructor
    CCPACSWingPositionings(const CCPACSWingPositionings& );

    // Assignment operator
    void operator=(const CCPACSWingPositionings& );

private:
    CCPACSWingPositioningContainer positionings;      /**< Positioning elements */
    bool                           invalidated;       /**< Internal state flag  */

};

} // end namespace tigl

#endif // CCPACSWINGPOSITIONINGS_H
