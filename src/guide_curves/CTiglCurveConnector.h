/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
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
#include <TopoDS_Wire.hxx>
#include <TopoDS_Compound.hxx>

#include <CCPACSGuideCurves.h>

namespace tigl {

class CTiglCurveConnector
{

public:
    CTiglCurveConnector(std::vector<CCPACSGuideCurve*> roots);

    TopoDS_Compound GetConnectedGuideCurves();
    void Invalidate();

private:
    TopoDS_Wire GetInterpolatedCurveFromRoot(CCPACSGuideCurve* curve);

    // the root guide curve (segments)
    std::vector<CCPACSGuideCurve*> m_roots;

    // the number of segments per guide curve
    int m_numSegments;

    TopoDS_Compound m_result;
    bool m_isBuilt = false;
};

} // namespace tigl

#endif // CTICLCURVECONNECTOR_H
