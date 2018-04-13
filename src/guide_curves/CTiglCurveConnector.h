/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#ifndef CTIGLCURVECONNECTOR_H
#define CTIGLCURVECONNECTOR_H

#include <vector>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Compound.hxx>

#include <CCPACSGuideCurves.h>

namespace tigl {

class CTiglCurveConnector
{
    enum dependencyType
    {
        none = 0,
        C2_from_previous,
        C2_to_previous
    };

    struct guideCurvePart
    {
        std::vector<CCPACSGuideCurve*> localGuides;

        std::vector<double> startParameters; //TODO unused so far.
        dependencyType dependency = none;

        TopoDS_Edge localCurve;
    };

    struct guideCurveConnected
    {
    public:
        std::vector<guideCurvePart> parts;
        std::vector<int> interpolationOrder;
    };

public:
    CTiglCurveConnector (std::vector<CCPACSGuideCurve*> roots);

    /**
     * @brief GetConnectedGuideCurves
     * @return
     */
    TopoDS_Compound GetConnectedGuideCurves ();

private:

    /**
     * @brief Verifies that all connected guide curves intersect the same
     * number of segments
     */
    void VerifyNumberOfSegments (std::vector<CCPACSGuideCurve*>& roots);

    /**
     * @brief Creates the list of partial curves for every connected guide
     * curve recursively
     *
     * The list of segmentwise guidecurves for a connected guide curve
     * is broken into partial curves. The breakpoints are at the guidecurve
     * with a prescribed continuity condition.
     *
     */
    void CreatePartialCurves (guideCurveConnected& connectedCurve,
                              CCPACSGuideCurve* current);

    /**
     * @brief Creates the interpolation order for the partial curves,
     * based on their interdependencies (from/to continuities)
     */
    void CreateInterpolationOrder (guideCurveConnected& connectedCurve);

    /**
     * @brief interpolates a partial curve given its continuity conditions
     * and prescribed tangents
     */
    void InterpolateGuideCurvePart(guideCurvePart& curvePart);



    std::vector<guideCurveConnected> m_connectedCurves;
};

} // namespace tigl

#endif // CTICLCURVECONNECTOR_H
