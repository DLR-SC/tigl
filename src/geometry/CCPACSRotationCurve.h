/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-12-04 Jan Kleinert <jan.kleinert@dlr.de>
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

#pragma once

#include "generated/CPACSRotationCurve.h"
#include "CCPACSTransformation.h"
#include "TopoDS_Face.hxx"
#include "TopoDS_Wire.hxx"

namespace tigl {
/*
 * This class is used for the rotationally symmetric inner surface of an engine nacelle
 */
class CCPACSRotationCurve : public generated::CPACSRotationCurve
{
public:
    CCPACSRotationCurve(CTiglUIDManager* uidMgr)
        : generated::CPACSRotationCurve(uidMgr)
    {}

    enum axis {
      x=0,
      y,
      z
    };

    TIGL_EXPORT TopoDS_Wire GetCurve() const;
    TIGL_EXPORT TopoDS_Face GetRotationSurface(gp_Pnt origin = {0., 0., 0.},axis dir=x) const;

private:
    void CutCurveAtZetas(TopoDS_Edge& edge) const;
};


}
