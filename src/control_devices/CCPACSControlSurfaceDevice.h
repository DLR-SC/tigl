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

#include "tixi.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CTiglControlSurfaceTransformation.h"
#include "tigl_internal.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "gp_Lin.hxx"

namespace tigl
{

class CCPACSWingComponentSegment;

class CCPACSControlSurfaceDevice : public CTiglAbstractPhysicalComponent
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment);
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & controlSurfaceDeviceXPath, TiglControlSurfaceType type = TRAILING_EDGE_DEVICE);
    TIGL_EXPORT CCPACSControlSurfaceDeviceOuterShape getOuterShape();
    TIGL_EXPORT CCPACSControlSurfaceDevicePath getMovementPath();        // Returns the Component Type TIGL_COMPONENT_WING.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_CONTROLSURF | TIGL_COMPONENT_PHYSICAL;}
    TIGL_EXPORT PNamedShape getCutOutShape(void);
    TIGL_EXPORT void setLoft(PNamedShape loft);
    TIGL_EXPORT TopoDS_Face getFace();
    TIGL_EXPORT gp_Trsf getTransformation(double flapStatusInPercent);
    TIGL_EXPORT void getProjectedPoints(gp_Pnt point1, gp_Pnt point2, gp_Pnt point3,
                                        gp_Pnt point4, gp_Vec& projectedPoint1,
                                        gp_Vec& projectedPoint2, gp_Vec& projectedPoint3,
                                        gp_Vec& projectedPoint4 );
    TIGL_EXPORT gp_Vec getNormalOfControlSurfaceDevice();
    TIGL_EXPORT CCPACSWingComponentSegment* getSegment();
    TIGL_EXPORT TiglControlSurfaceType getType();

protected:
    PNamedShape BuildLoft();

private:
    double linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1);
    CCPACSControlSurfaceDevice(const CCPACSControlSurfaceDevice& segment); /* disable copy constructor */
    double determineCutOutPrismThickness();
    std::string GetShortShapeName();
    double determineSpoilerThickness();

    gp_Pnt getLeadingEdgeShapeLeadingEdgePoint(bool isInnerBorder);
    gp_Pnt getLeadingEdgeShapeLowerPoint(bool isInnerBorder);
    gp_Pnt getLeadingEdgeShapeUpperPoint(bool isInnerBorder);
    gp_Vec getLeadingEdgeShapeTangent(gp_Pnt leadingPoint, gp_Pnt lowerPoint, gp_Pnt upperPoint,bool isInnerBorder, bool isUpper);
    gp_Vec getLeadingEdgeShapeNormal(gp_Pnt point, gp_Vec tangent,gp_Pln etaPlane, gp_Vec checker);
    TopoDS_Wire buildLeadingEdgeShapeWire(bool isInnerBorder);

    CCPACSControlSurfaceDevicePath path;
    CCPACSControlSurfaceDeviceOuterShape outerShape;
    CCPACSControlSurfaceDeviceWingCutOut wingCutOut;

    // @todo: remove - loft belongs to abstractcomponent
    PNamedShape loft;
    CCPACSWingComponentSegment* _segment;
    // @todo: hingeline not freed
    CTiglControlSurfaceHingeLine* _hingeLine;

    TiglControlSurfaceType _type;



};

} // end namespace tigl

#endif // CCPACSControlSurfaceDevice_H
