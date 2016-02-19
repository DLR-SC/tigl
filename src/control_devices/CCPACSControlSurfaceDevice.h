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

namespace tigl
{

class CCPACSWingComponentSegment;

class CCPACSControlSurfaceDevice : public CTiglAbstractPhysicalComponent
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & controlSurfaceDeviceXPath, TiglControlSurfaceType type = TRAILING_EDGE_DEVICE);

    // Return the CPACS outer shape of the flap
    TIGL_EXPORT const CCPACSControlSurfaceDeviceOuterShape& getOuterShape() const;
    TIGL_EXPORT const CCPACSControlSurfaceDevicePath& getMovementPath() const;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_CONTROLSURF | TIGL_COMPONENT_PHYSICAL;}
    TIGL_EXPORT PNamedShape getCutOutShape(void);
    
    // @TODO: the loft buildup should be done inside this class. Currently it
    // is done inside the wing class.
    TIGL_EXPORT void setLoft(PNamedShape loft);

    // Returns the flap transformation based on the deflection
    TIGL_EXPORT gp_Trsf GetFlapTransform(double deflection) const;

    // Returns the minimal deflaction value (defined in CPACS file)
    TIGL_EXPORT double GetMinDeflection() const;

    // Returns the minimal deflaction value (defined in CPACS file)
    TIGL_EXPORT double GetMaxDeflection() const;


    TIGL_EXPORT gp_Vec getNormalOfControlSurfaceDevice();
    TIGL_EXPORT CCPACSWingComponentSegment* getSegment();
    TIGL_EXPORT TiglControlSurfaceType getType();

protected:
    PNamedShape BuildLoft();

private:
    CCPACSControlSurfaceDevice(const CCPACSControlSurfaceDevice& segment); /* disable copy constructor */
    double determineCutOutPrismThickness();
    std::string GetShortShapeName();

    class CSCoordSystem getBorderPlane(bool isInnerBorder);
    TopoDS_Wire getCutoutWire(bool isInnerBorder);

    CCPACSControlSurfaceDevicePath path;
    CCPACSControlSurfaceDeviceOuterShape outerShape;
    CCPACSControlSurfaceDeviceWingCutOut wingCutOut;

    CCPACSWingComponentSegment* _segment;
    CSharedPtr<CTiglControlSurfaceHingeLine> _hingeLine;

    TiglControlSurfaceType _type;



};

} // end namespace tigl

#endif // CCPACSControlSurfaceDevice_H
