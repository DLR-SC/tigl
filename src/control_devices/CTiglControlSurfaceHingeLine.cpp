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

#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"

#include "CCPACSControlSurfaceDeviceOuterShape.h"
#include "CCPACSControlSurfaceDevicePath.h"

namespace tigl
{

CTiglControlSurfaceHingeLine::CTiglControlSurfaceHingeLine(const CCPACSControlSurfaceDeviceOuterShape* outerShape, 
                                                           const CCPACSControlSurfaceDevicePath* path,
                                                           CCPACSWingComponentSegment* segment)
    : _invalidated(true)
    , _outerShape(outerShape)
    , _path(path)
    , _segment(segment)
{
}


gp_Pnt CTiglControlSurfaceHingeLine::getInnerHingePoint() const
{
    buildHingeLine();
    
    return _innerHingePoint;
}

gp_Pnt CTiglControlSurfaceHingeLine::getOuterHingePoint() const
{
    buildHingeLine();
    
    return _outerHingePoint;
}

void CTiglControlSurfaceHingeLine::invalidate()
{
    _invalidated = true;
}

void CTiglControlSurfaceHingeLine::buildHingeLine() const
{
    if (!_invalidated) {
        return;
    }
    
    if (!_segment || !_path || !_outerShape) {
        throw CTiglError("Null pointer in CTiglControlSurfaceHingeLine::buildHingeLine!", TIGL_NULL_POINTER);
    }
    
    // Calculate inner and outer HingePoint
    tigl::CCPACSControlSurfaceDeviceOuterShapeBorder borders[2];
    borders[0] = _outerShape->getOuterBorder();
    borders[1] = _outerShape->getInnerBorder();

    for ( int borderCounter = 0; borderCounter < 2; borderCounter++ ) {
        double hingeXsi;
        if (borderCounter == 0) {
            hingeXsi = _path->getOuterHingePoint().getXsi();
        }
        else {
            hingeXsi = _path->getInnerHingePoint().getXsi();
        }

        double borderEtaLE = borders[borderCounter].getEtaLE();
        double borderEtaTE = borders[borderCounter].getEtaTE();
        double hingeEta = -1;

        // only calculate etaCoordinate if it´s not the same as the other one.
        if ( fabs(borderEtaLE - borderEtaTE) < 0.0001 ) {
            hingeEta = (borderEtaTE + borderEtaLE)/2;
        }
        else {
            double m = ( borderEtaLE - borderEtaTE )/(borders[borderCounter].getXsiLE() - borders[borderCounter].getXsiTE());
            hingeEta = m * (hingeXsi - borders[borderCounter].getXsiLE()) + borderEtaLE;
        }

        double eta = 0.,xsi = 0.;
        CCPACSWingSegment* wsegment = _segment->GetSegmentEtaXsi(hingeEta,hingeXsi,eta,xsi);
        gp_Pnt hingeUpper = wsegment->GetUpperPoint(eta,xsi);
        gp_Pnt hingeLower = wsegment->GetLowerPoint(eta,xsi);

        if (borderCounter == 0) {
            double relHeight = _path->getOuterHingePoint().getRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            _outerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
        else {
            double relHeight = _path->getInnerHingePoint().getRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            _innerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
    }
    
    _invalidated = false;
}


} // end namespace tigl
