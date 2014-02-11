/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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
* @brief  Implementation of a CPACS guide curve
*/

#ifndef CCPACSGUIDECURVE_H
#define CCPACSGUIDECURVE_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CSharedPtr.h"

#include <gp_Pnt.hxx>

#include <vector>
#include <string>

namespace tigl
{

class CTiglPoint;
typedef class CSharedPtr<CTiglPoint> PCTiglPoint;

class CCPACSGuideCurve
{

private:
    // Typedefs for a container to store the coordinates of a guide curve element.
    typedef std::vector<PCTiglPoint> CCPACSGuideCurvePoints;

public:
    // Constructor
    TIGL_EXPORT CCPACSGuideCurve(const std::string& path);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSGuideCurve(void);

    // Read CPACS guide curve file
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the filename of the guide curve file
    TIGL_EXPORT const std::string& GetFileName(void) const;

    // Returns the name of the guide curve
    TIGL_EXPORT const std::string& GetName(void) const;

    // Returns the UID of the guide curve
    TIGL_EXPORT const std::string& GetUID(void) const;

    // Returns the guide curve points as read from TIXI.
    TIGL_EXPORT std::vector<PCTiglPoint> GetGuideCurvePoints();

protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSGuideCurve(const CCPACSGuideCurve& ) { /* Do nothing */ };

    // Assignment operator
    void operator=(const CCPACSGuideCurve& ) { /* Do nothing */ };

    std::string               GuideCurveXPath;   /**< The XPath to this guide curve in cpacs */
    std::string               name;              /**< The Name of the guide curve */
    std::string               description;       /**< The description of the guide curve */
    std::string               uid;               /**< The UID of the guide curve */
    CCPACSGuideCurvePoints    coordinates;       /**< Coordinates of a guide curve points */

};

} // end namespace tigl

#endif // CCPACSGUIDECURVE_H
