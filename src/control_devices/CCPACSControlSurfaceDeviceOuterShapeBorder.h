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

#ifndef CCPACSCONTROLSURFACEDEVICEOUTERSHAPEBORDER_H
#define CCPACSCONTROLSURFACEDEVICEOUTERSHAPEBORDER_H

#include <string>

#include "tixi.h"
#include "CTiglError.h"
#include "tigl_internal.h"
#include "CCPACSControlSurfaceDeviceBorderLeadingEdgeShape.h"
#include "CTiglControlSurfaceBorderCoordinateSystem.h"
#include "PNamedShape.h"

#include <TopoDS_Wire.hxx>

namespace tigl
{

class CCPACSWingComponentSegment;

class CCPACSControlSurfaceDeviceOuterShapeBorder
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceOuterShapeBorder(CCPACSWingComponentSegment*);

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & BorderXPath,
                               TiglControlSurfaceType type = TRAILING_EDGE_DEVICE);

    TIGL_EXPORT double getEtaLE() const;
    TIGL_EXPORT double getEtaTE() const;
    TIGL_EXPORT double getXsiLE() const;
    TIGL_EXPORT double getXsiTE() const;

    // computes the wire of the outershape
    TIGL_EXPORT TopoDS_Wire getWire(PNamedShape wingShape, gp_Vec upDir) const;

    TIGL_EXPORT CTiglControlSurfaceBorderCoordinateSystem getCoordinateSystem(gp_Vec upDir) const;

    TIGL_EXPORT CCPACSControlSurfaceDeviceBorderLeadingEdgeShape getLeadingEdgeShape() const;
    TIGL_EXPORT bool isLeadingEdgeShapeAvailable() const;

    TIGL_EXPORT void setUID(const std::string& uid);

private:
    std::string xsiType;
    std::string _uid;

    /* Simple Border */
    double etaLE;
    double etaTE;
    double xsiLE;
    double xsiTE;

    CCPACSControlSurfaceDeviceBorderLeadingEdgeShape leadingEdgeShape;
    bool leadingEdgeShapeAvailible;

    // helpers
    CCPACSWingComponentSegment* _segment;
};

} // end namespace tigl

#endif // CCPACSCONTROLSURFACEDEVICEOUTERSHAPEBORDER_H
