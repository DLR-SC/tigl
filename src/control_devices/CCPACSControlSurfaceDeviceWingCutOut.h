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

#ifndef CCPACSControlSurfaceDeviceWingCutOut_H
#define CCPACSControlSurfaceDeviceWingCutOut_H

#include "tixi.h"
#include "CCPACSControlSurfaceDeviceWingCutOutProfiles.h"
#include "CCPACSControlSurfaceSkinCutOut.h"
#include "CCPACSCutOutControlPoints.h"
#include "CCPACSControlSurfaceSkinCutoutBorder.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"
#include "PNamedShape.h"
#include "tigl_internal.h"

#include <TopoDS_Wire.hxx>
#include <gp_Vec.hxx>

namespace tigl
{

class CCPACSControlSurfaceDeviceOuterShapeBorder;
class CCPACSControlSurfaceDeviceOuterShape;
class CCPACSWingComponentSegment;
class CCPACSControlSurfaceDevice;

class CCPACSControlSurfaceDeviceWingCutOut
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceWingCutOut(CCPACSControlSurfaceDevice*, CCPACSWingComponentSegment*);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & controlSurfaceDeviceWingCutOutXPath);

    TIGL_EXPORT const CCPACSCutOutControlPointsPtr cutOutProfileControlPoints() const;

    TIGL_EXPORT const CCPACSControlSurfaceSkinCutOut& upperSkin() const;
    TIGL_EXPORT const CCPACSControlSurfaceSkinCutOut& lowerSkin() const;

    TIGL_EXPORT const CCPACSControlSurfaceSkinCutoutBorderPtr innerBorder() const;
    TIGL_EXPORT const CCPACSControlSurfaceSkinCutoutBorderPtr outerBorder() const;

    TIGL_EXPORT PNamedShape GetLoft(PNamedShape wingCleanShape, CCPACSControlSurfaceDeviceOuterShape*, gp_Vec upDir);

protected:
    CCPACSControlSurfaceDeviceWingCutOutProfiles wingCutOutProfiles;
    CCPACSControlSurfaceSkinCutOut _upperSkin, _lowerSkin;
    CCPACSCutOutControlPointsPtr _cutOutProfileControlPoints;
    CCPACSControlSurfaceSkinCutoutBorderPtr _innerBorder, _outerBorder;

private:
    CCPACSWingComponentSegment* _segment;
    CCPACSControlSurfaceDevice* _csDevice;
    PNamedShape _loft;

    TopoDS_Wire getCutoutWire(bool isInnerBorder,
                              PNamedShape wingCleanShape,
                              const CCPACSControlSurfaceDeviceOuterShapeBorder*,
                              gp_Vec upDir);

    CTiglControlSurfaceBorderCoordinateSystem getCutoutCS(bool isInnerBorder,
                                                          const CCPACSControlSurfaceDeviceOuterShapeBorder*,
                                                          gp_Vec upDir);
};

} // end namespace tigl

#endif // CCPACSControlSurfaceDeviceWingCutOut_H
