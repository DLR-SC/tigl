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

#ifndef CCPACSControlSurfaceDevice_H
#define CCPACSControlSurfaceDevice_H

#include <string>
#include <vector>

#include "tigl_internal.h"
#include "CSharedPtr.h"

#include "tixi.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDevicePath.h"
#include "CTiglFusePlane.h"

namespace tigl
{

class CCPACSWingComponentSegment;
class CCPACSConfiguration;

class CCPACSControlSurfaceDevice : public CTiglAbstractPhysicalComponent
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDevice(CCPACSConfiguration* config, CCPACSWingComponentSegment* segment);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & controlSurfaceDeviceXPath, TiglControlSurfaceType type = TRAILING_EDGE_DEVICE);

    // Return the CPACS outer shape of the flap
    TIGL_EXPORT const CCPACSControlSurfaceDeviceOuterShape& getOuterShape() const;
    TIGL_EXPORT const CCPACSControlSurfaceDevicePath& getMovementPath() const;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_CONTROLSURF | TIGL_COMPONENT_PHYSICAL;}
    TIGL_EXPORT PNamedShape getCutOutShape(void);
    TIGL_EXPORT PNamedShape getFlapShape(void);
    TIGL_EXPORT PNamedShape getTransformedFlapShape();

    // Returns the flap transformation based on the current deflection
    TIGL_EXPORT gp_Trsf GetFlapTransform() const;

    // Returns the minimal deflection value (defined in CPACS file)
    TIGL_EXPORT double GetMinDeflection() const;

    // Returns the minimal deflection value (defined in CPACS file)
    TIGL_EXPORT double GetMaxDeflection() const;

    // Get and set the current deflection  value
    TIGL_EXPORT double GetDeflection() const;
    TIGL_EXPORT void SetDeflection(const double deflect);


    TIGL_EXPORT gp_Vec getNormalOfControlSurfaceDevice();
    TIGL_EXPORT CCPACSWingComponentSegment* getSegment();
    TIGL_EXPORT TiglControlSurfaceType getType();

    TIGL_EXPORT void SetUID(const std::string& uid);
    TIGL_EXPORT std::string GetShortShapeName();

protected:
    PNamedShape BuildLoft();
    void InvalidateShapes();

private:
    CCPACSControlSurfaceDevice(const CCPACSControlSurfaceDevice& segment); /* disable copy constructor */

    // CPACS elements of control surface
    CCPACSControlSurfaceDevicePath path;
    CCPACSControlSurfaceDeviceOuterShape outerShape;
    CSharedPtr<CCPACSControlSurfaceDeviceWingCutOut> wingCutOut;
    CSharedPtr<CTiglControlSurfaceHingeLine> _hingeLine;

    double currentDeflection;

    // Helper members
    CCPACSWingComponentSegment* _segment;
    CCPACSConfiguration* _config;
    TiglControlSurfaceType _type;

};

} // end namespace tigl

#endif // CCPACSControlSurfaceDevice_H
