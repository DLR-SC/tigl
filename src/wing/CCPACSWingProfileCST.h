/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile as a CST profile
*
* The wing profile is defined by the (C)lass function / (S)hape function (T)ransformation
* geometry representation method.
*/

#include "tigl_internal.h"
#include "ITiglWingProfileAlgo.h"

#include <vector>
#include <TopoDS_Edge.hxx>

#ifndef CCPACSWINGPROFILECST_H
#define CCPACSWINGPROFILECST_H

namespace tigl
{
class CCPACSWingProfile;

class CCPACSWingProfileCST : public ITiglWingProfileAlgo
{

private:
    // Typedef for a container to store the coordinates of a wing profile element.
    typedef std::vector<CTiglPoint*> CCPACSCoordinateContainer;

public:
    // Constructor
    TIGL_EXPORT CCPACSWingProfileCST(const CCPACSWingProfile&, const std::string& aFilename);

    // Destructor
    TIGL_EXPORT ~CCPACSWingProfileCST(void);

    TIGL_EXPORT static std::string CPACSID();

    // Cleanup routine
    TIGL_EXPORT void Cleanup(void);

    // Update of wire points ...
    TIGL_EXPORT void Update(void);

    // Read CPACS wing profile file
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the profile points as read from TIXI.
    TIGL_EXPORT std::vector<CTiglPoint*> GetSamplePoints() const;

    // get profiles CPACS XML path
    TIGL_EXPORT const std::string & GetProfileDataXPath() const;

    // get upper wing profile wire
    TIGL_EXPORT const TopoDS_Edge & GetUpperEdge() const;

    // get lower wing profile wire
    TIGL_EXPORT const TopoDS_Edge & GetLowerEdge() const;

    // get trailing edge
    TIGL_EXPORT const TopoDS_Edge & GetTrailingEdge() const;
    
    // gets the upper and lower wing profile into on edge
    TIGL_EXPORT const TopoDS_Edge & GetUpperLowerEdge() const;

    // get leading edge point();
    TIGL_EXPORT const gp_Pnt & GetLEPoint() const;

    // get trailing edge point();
    TIGL_EXPORT const gp_Pnt & GetTEPoint() const;

protected:
    // Builds the wing profile wires.
    void BuildWires();

private:
    // Copy constructor
    CCPACSWingProfileCST(const CCPACSWingProfileCST& );

    // Assignment operator
    void operator=(const CCPACSWingProfileCST& );

private:
    std::string               ProfileDataXPath;   /**< CPACS path to profile data (pointList or cst2D) */
    TopoDS_Edge               upperEdge;          /**< edge of the upper wing profile */
    TopoDS_Edge               lowerEdge;          /**< edge of the lower wing profile */
    TopoDS_Edge               upperLowerEdge;     /**< edge consisting of upper and lower wing profile */
    TopoDS_Edge               trailingEdge;       /**< edge of the trailing edge */
    gp_Pnt                    lePoint;            /**< Leading edge point */
    gp_Pnt                    tePoint;            /**< Trailing edge point */
    double                    upperN1;            /**< CST parameter N1 */
    double                    upperN2;            /**< CST parameter N2 */
    std::vector<double>       upperB;             /**< CST parameter B */
    double                    lowerN1;            /**< CST parameter N1 */
    double                    lowerN2;            /**< CST parameter N2 */
    std::vector<double>       lowerB;             /**< CST parameter B */
};

} // end namespace tigl

#endif // CCPACSWINGPROFILECST_H
