/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#ifndef CCPACSCONTROLSURFACEDEVICEWINGCUTOUTSTRUCTURALPROFILE_H
#define CCPACSCONTROLSURFACEDEVICEWINGCUTOUTSTRUCTURALPROFILE_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CSharedPtr.h"

#include <gp_Pnt.hxx>

#include <vector>
#include <string>

#include <gp_Pnt.hxx>


namespace tigl
{
// @todo: replace pointer on points to simple points
class CTiglPoint;
typedef class CSharedPtr<CTiglPoint> PCTiglPoint;


class CCPACSControlSurfaceDeviceWingCutOutStructuralProfile
{
public:
    // Constructor
    TIGL_EXPORT CCPACSControlSurfaceDeviceWingCutOutStructuralProfile(const std::string& path);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSControlSurfaceDeviceWingCutOutStructuralProfile(void);

    // Read CPACS wingCutOut profile file
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the filename of the wingCutOut file
    TIGL_EXPORT const std::string& GetFileName(void) const;

    // Returns the name of the wingCutOut
    TIGL_EXPORT const std::string& GetName(void) const;

    // Returns the UID of the wingCutOut
    TIGL_EXPORT const std::string& GetUID(void) const;

    // Returns the wingCutOut points as read from TIXI.
    // @todo: return const CCPACSCoordinateContainer& instead
    TIGL_EXPORT std::vector<PCTiglPoint> GetWingCutOutProfilePoints();

private:
    // Copy constructor
    CCPACSControlSurfaceDeviceWingCutOutStructuralProfile(const CCPACSControlSurfaceDeviceWingCutOutStructuralProfile&);

    typedef std::vector<PCTiglPoint> CCPACSCoordinateContainer;
    CCPACSCoordinateContainer        coordinates;    /**< Coordinates of a wingCutOut profile element */
    std::string                      wingCutOutProfileXPath;   /**< The XPath to this wingCutOut in cpacs */
    std::string                      name;              /**< The Name of the wingCutOut*/
    std::string                      description;       /**< The description of the wingCutOut */
    std::string                      uid;               /**< The UID of the wingCutOut */
};

} // end namespace tigl

#endif // endif CCPACSCONTROLSURFACEDEVICEWINGCUTOUTSTRUCTURALPROFILE_H

