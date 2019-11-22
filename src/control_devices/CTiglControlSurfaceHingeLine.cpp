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

#include "generated/CPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "CPACSControlSurfacePath.h"
#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "CCPACSControlSurfaceOuterShapeTrailingEdge.h"


namespace tigl
{

CTiglControlSurfaceHingeLine::CTiglControlSurfaceHingeLine(const CCPACSControlSurfaceOuterShapeTrailingEdge* outerShape,
                                                           const CCPACSControlSurfacePath* path,
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
    for ( int borderCounter = 0; borderCounter < 2; borderCounter++ ) {
        const CCPACSControlSurfaceBorderTrailingEdge* border = NULL;
        double hingeXsi;
        if (borderCounter == 0) {
            border = &_outerShape->GetOuterBorder();
            hingeXsi = _path->GetOuterHingePoint().GetHingeXsi();
        }
        else {
            border = &_outerShape->GetInnerBorder();
            hingeXsi = _path->GetInnerHingePoint().GetHingeXsi();
        }

        // TODO: this is still CPACS 2 and must be adapted
        double borderEtaLE = border->GetEtaLE().GetEta();
        double borderEtaTE = border->GetEtaTE()->GetEta();
        double hingeEta = -1;

        // only calculate etaCoordinate if it´s not the same as the other one.
        if ( fabs(borderEtaLE - borderEtaTE) < 0.0001 ) {
            hingeEta = (borderEtaTE + borderEtaLE)/2;
        }
        else {
            // TODO: hackish implementation. XSI TE is dependent, what kind of control surface device we have
            double xsiTE = 1.;
            double m = ( borderEtaLE - borderEtaTE )/(border->GetXsiLE().GetXsi() - xsiTE);
            hingeEta = m * (hingeXsi - border->GetXsiLE().GetXsi()) + borderEtaLE;
        }

        double eta = 0.,xsi = 0.;
        std::string segmentUID;
        _segment->GetSegmentEtaXsi(hingeEta,hingeXsi,segmentUID, eta,xsi);
        CCPACSWingSegment* wsegment = &(_segment->GetUIDManager().ResolveObject<CCPACSWingSegment>(segmentUID));

        gp_Pnt hingeUpper = wsegment->GetUpperPoint(eta,xsi);
        gp_Pnt hingeLower = wsegment->GetLowerPoint(eta,xsi);

        if (borderCounter == 0) {
            double relHeight = _path->GetOuterHingePoint().GetHingeRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            _outerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
        else {
            double relHeight = _path->GetInnerHingePoint().GetHingeRelHeight();
            gp_Vec upperToLower = (gp_Vec(hingeLower.XYZ()) - gp_Vec(hingeUpper.XYZ())).Multiplied(relHeight);
            _innerHingePoint = gp_Pnt(( gp_Vec(hingeUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
        }
    }
    
    _invalidated = false;
}


} // end namespace tigl
