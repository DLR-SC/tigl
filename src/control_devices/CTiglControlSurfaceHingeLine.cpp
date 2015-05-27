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
 * @brief  Implementation of CPACS ...  handling routines.
 */


#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"

namespace tigl
{

// @todo: All work is done in constructor
// this is not correct. in particalar the wing transformation is not completely known at
// this point. we should not call any getPoint functions at this stage
CTiglControlSurfaceHingeLine::CTiglControlSurfaceHingeLine(CCPACSControlSurfaceDeviceOuterShape outerShape, CCPACSControlSurfaceDevicePath path,
                                                           CCPACSWingComponentSegment* segment)
{
    // Calculate inner and outer HingePoint
    tigl::CCPACSControlSurfaceDeviceOuterShapeBorder borders[2];
    borders[0] = outerShape.getOuterBorder();
    borders[1] = outerShape.getInnerBorder();

    for ( int borderCounter = 0; borderCounter < 2; borderCounter++ )
    {
        double hingeXsi;
        if (borderCounter == 0) {
            hingeXsi = path.getOuterHingePoint().getXsi();
        }
        else {
            hingeXsi = path.getInnerHingePoint().getXsi();
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
        CCPACSWingSegment* wsegment = segment->GetSegmentEtaXsi(hingeEta,hingeXsi,eta,xsi);
        gp_Pnt hingeUpper = wsegment->GetUpperPoint(eta,xsi);
        gp_Pnt hingeLower = wsegment->GetLowerPoint(eta,xsi);

        if (borderCounter == 0) {
            double relHeight = path.getOuterHingePoint().getRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            outerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
        else {
            double relHeight = path.getInnerHingePoint().getRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            innerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
    }
}

gp_Pnt CTiglControlSurfaceHingeLine::getTransformedInnerHingePoint(gp_Vec translation)
{
    return gp_Pnt((gp_Vec(innerHingePoint.XYZ()) + translation).XYZ());
}

gp_Pnt CTiglControlSurfaceHingeLine::getTransformedOuterHingePoint(gp_Vec translation)
{
    return gp_Pnt((gp_Vec(outerHingePoint.XYZ()) + translation).XYZ());
}

gp_Pnt CTiglControlSurfaceHingeLine::getInnerHingePoint()
{
    return innerHingePoint;
}

gp_Pnt CTiglControlSurfaceHingeLine::getOuterHingePoint()
{
    return outerHingePoint;
}


} // end namespace tigl
