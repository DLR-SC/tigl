/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-18 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @file CCPACSGuideCurveAlgo.h
* @brief  Implementation of CPACS guide curve construction routine
*
* Template class where profileGetPointAlgo gives the point and the tangent of a
* wing or fuselage profile at a given length parameter
*/

#ifndef CCPACSGUIDECURVEALGO_H
#define CCPACSGUIDECURVEALGO_H

#include "tigl_internal.h"
#include "CCPACSGuideCurveProfile.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"

namespace tigl
{

class CTiglPoint;
typedef class CSharedPtr<CTiglPoint> PCTiglPoint;

template<class profileGetPointAlgo>
class CCPACSGuideCurveAlgo
{
public:

    // Typedefs for a container to store the coordinates of a guide curve element.
    typedef std::vector<PCTiglPoint> CCPACSGuideCurveProfilePoints;

    /**
     * \brief Algorithm which constructs a guide curve for a wing or fuselage profile
     *
     * \param profileContainer1 Contains the 1st profile in world coordinates (inner in the case of wing profiles)
     * \param profileContainer2 Contains the 2nd profile in world coordinates (outer in the case of wing profiles)
     * \param alpha1 Parameter which defines the starting point of the guide curve on the 1st profile
     * \param alpha2 Parameter which defines the starting point of the guide curve on the 2nd profile
     * \param scale1 Scaling factor from 1st profile (inner profile chord line in the case of wing profiles)
     * \param scale2 Scaling factor from 2nd profile (outer profile chord line in the case of wing profiles)
     * \param gcp Guide curve profile coordinates
     *
     * @return Guide curve wire in world coordinates
     */
    TIGL_EXPORT CCPACSGuideCurveAlgo(const TopTools_SequenceOfShape& profileContainer1,
                                     const TopTools_SequenceOfShape& profileContainer2,
                                     const Standard_Real& alpha1,
                                     const Standard_Real& alpha2,
                                     const Standard_Real& scale1,
                                     const Standard_Real& scale2,
                                     CCPACSGuideCurveProfile& gcp) :
        _getPointAlgo1(profileContainer1),
        _getPointAlgo2(profileContainer2),
        _alpha1(alpha1),
        _alpha2(alpha2),
        _scale1(scale1),
        _scale2(scale2),
        _guideCurveProfile(gcp)
    {
    }

    TIGL_EXPORT operator TopoDS_Edge()
    {
        // get guide Curve points in local coordinates
        std::vector<CTiglPoint> guideCurveProfilePoints = _guideCurveProfile.GetGuideCurveProfilePoints();
        // container for guide Curve points in world coordinates
        std::vector<gp_Pnt> guideCurvePoints(guideCurveProfilePoints.size()+2);
        // get anchor points in world coordinates
        gp_Vec tangent;
        _getPointAlgo1.GetPointTangent(_alpha1, guideCurvePoints[0], tangent);
        _getPointAlgo2.GetPointTangent(_alpha2, guideCurvePoints[guideCurvePoints.size()-1], tangent);
        // loop over guide Curve profile points
        for (int i=0; i!=guideCurveProfilePoints.size(); i++) {

            Standard_Real alpha = guideCurveProfilePoints[i].x;
            Standard_Real beta  = guideCurveProfilePoints[i].y;
            Standard_Real gamma = guideCurveProfilePoints[i].z;

            // ******************************************************************
            // construct line between anchor points at the start and end profile
            // ******************************************************************
            // get starting point
            gp_Pnt startPoint;
            gp_Vec startTangent;
            _getPointAlgo1.GetPointTangent(alpha, startPoint, startTangent);

            // get end point
            gp_Pnt endPoint;
            gp_Vec endTangent;
            _getPointAlgo2.GetPointTangent(alpha, endPoint, endTangent);

            // construct vector in beta direction
            gp_Vec vectorBeta(startPoint, endPoint);

            // ******************************************************************
            // construct start and end normal vectors in gamma direction
            // ******************************************************************
            // interpolate tangent vectors at start and end profile
            gp_Vec tangentInterpolated = startTangent + (endTangent - startTangent) * beta;

            gp_Vec vectorGamma = tangentInterpolated.Crossed(vectorBeta);
            gp_Dir normalGamma(vectorGamma);

            // ******************************************************************
            // calculate linear interpolated scaling factor in gamma direction
            // ******************************************************************
            Standard_Real scale = _scale1 + (_scale2-_scale1) * beta;

            // ******************************************************************
            // construct world coordinates for guide curve point
            // ******************************************************************
            // start with the anchor point at the 1st profile
            gp_Vec guideCurvePoint (gp_Pnt(), startPoint);
            // add contribution from beta along the connection between the two anchor points
            guideCurvePoint += beta * vectorBeta;
            // add contribution from gamma
            // direction: linear interpolation of normal vectors in gamma direction
            // magnitude: gamma times linear interpolated scaling factor
            guideCurvePoint += gamma * scale * normalGamma;
            // save to container
            guideCurvePoints[i+1]=gp_Pnt(guideCurvePoint.XYZ()) ;
        }

        // interpolate B-Spline curve through guide curve points
        TopoDS_Edge guideCurveEdge = EdgeSplineFromPoints(guideCurvePoints);

        return guideCurveEdge;
    }

private:
    profileGetPointAlgo _getPointAlgo1;            /**< Get point and tangent of 1st profile */
    profileGetPointAlgo _getPointAlgo2;            /**< Get point and tangent of 2nd profile */
    Standard_Real           _alpha1;               /**< Starting point parameter */
    Standard_Real           _alpha2;               /**< End point parameter */
    Standard_Real           _scale1;               /**< 1st scale factor */
    Standard_Real           _scale2;               /**< 2nd scale factor */
    CCPACSGuideCurveProfile& _guideCurveProfile;   /**< Guide curve profile */
};

} // end namespace tigl

#endif // CCPACSGUIDECURVEALGO_H



