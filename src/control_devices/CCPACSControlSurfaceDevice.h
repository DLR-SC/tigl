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
/**
 * @file
 * @brief  Implementation of ..
 */

#ifndef CCPACSControlSurfaceDevice_H
#define CCPACSControlSurfaceDevice_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceOuterShape.h"
#include "CCPACSControlSurfaceDevicePath.h"
#include "CTiglControlSurfaceTransformation.h"
#include "tigl_internal.h"
#include "CTiglControlSurfaceHingeLine.h"

namespace tigl {

class CCPACSWingComponentSegment;

class CCPACSControlSurfaceDevice : public CTiglAbstractPhysicalComponent
{

private:

    // name
    // description
    // parentUID
    // wingCutOut
    // structure
    // tracks
    // actuators
    // cruiseRollers
    // interconnectionStruts
    // zCouplings

    CCPACSControlSurfaceDevicePath path;
    CCPACSControlSurfaceOuterShape outerShape;
    std::string uID;
    TopoDS_Shape loft;
    CCPACSWingComponentSegment* _segment;
    CTiglControlSurfaceHingeLine* _hingeLine;

    gp_Pnt s1;
    gp_Pnt s2;
    gp_Pnt s1s;
    gp_Pnt s2s;

    bool _isLeadingEdge;

public:
    TIGL_EXPORT CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment);
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & trailingEdgeDeviceXPath, bool isLeadingEdge = false);
    TIGL_EXPORT std::string getUID();
    TIGL_EXPORT CCPACSControlSurfaceOuterShape getOuterShape();
    TIGL_EXPORT CCPACSControlSurfaceDevicePath getMovementPath();        // Returns the Component Type TIGL_COMPONENT_WING.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_CONTROLSURF | TIGL_COMPONENT_PHYSICAL;}
    TIGL_EXPORT TopoDS_Shape getCutOutShape(void);
    TIGL_EXPORT void setLoft(TopoDS_Shape loft);
    TIGL_EXPORT TopoDS_Face getFace();
    TIGL_EXPORT gp_Trsf getTransformation(double flapStatusInPercent);
    TIGL_EXPORT void getProjectedPoints(gp_Pnt point1, gp_Pnt point2, gp_Pnt point3,
                                        gp_Pnt point4, gp_Vec& projectedPoint1,
                                        gp_Vec& projectedPoint2, gp_Vec& projectedPoint3,
                                        gp_Vec& projectedPoint4 );
    TIGL_EXPORT gp_Vec getNormalOfTrailingEdgeDevice();
    TIGL_EXPORT CCPACSWingComponentSegment* getSegment();

protected:
    TopoDS_Shape BuildLoft();

private:
    double linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1);
    CCPACSControlSurfaceDevice(const CCPACSControlSurfaceDevice& segment); /* disable copy constructor */
    double determineCutOutPrismThickness();

};

} // end namespace tigl

#endif // CCPACSControlSurfaceDevice_H
