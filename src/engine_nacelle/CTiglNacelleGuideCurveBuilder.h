/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-30 Jan Kleinert <jan.kleinert@dlr.de>
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


#include <vector>
#include <boost/optional.hpp>

#include "tigl_internal.h"
#include "CTiglPoint.h"
#include <generated/CPACSNacelleGuideCurve.h>
#include "CCPACSNacelleSection.h"

#include <TopoDS_Wire.hxx>


#pragma once

namespace tigl
{

    struct NacelleGuideCurveParameters {

        double fromZeta = 0.0;
        double toZeta   = 0.0;

        CCPACSNacelleSection const * fromSection = nullptr;
        CCPACSNacelleSection const * toSection   = nullptr;

        std::vector<CTiglPoint> profilePoints;
    };

    class CTiglNacelleGuideCurveBuilder
    {
    public:
        explicit CTiglNacelleGuideCurveBuilder(const CCPACSNacelleGuideCurve& guide);
        explicit CTiglNacelleGuideCurveBuilder(const NacelleGuideCurveParameters& params);

        void SetResolutionDegree(double dp) { dPhi = dp; }
        double GetResolutionDegree() const  { return dPhi; }

        TIGL_EXPORT operator TopoDS_Wire();
        TIGL_EXPORT TopoDS_Wire GetWire();

    private:
        NacelleGuideCurveParameters parameters;

        // A guide curve will be sampled at dPhi steps for the angle
        double dPhi = 5.;
    };

} //namespace tigl
