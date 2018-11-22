/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-22 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleGuideCurve.h"
#include "BRepBuilderAPI_MakeWire.hxx"


namespace tigl {

TopoDS_Wire CCPACSNacelleGuideCurve::GetWire()
{
    // get start section for r0, phi0, startZeta
    // get end section for r1, phi1, endZeta

    // get profile and profile points. Interpolate using N points (<- user defined "fineness" with default value)
    // this results in phii, ri, xi, i=1,...,N

    // ri(phii) = (r1+2*r0)*phii^3 - 3*r0*phii^2 + r0 + ri;
    // phii = phi0 + (1-phii)*phi1

    // transform phii,ri,xi to catesian coordinates and interpolate

    return TopoDS_Wire();
}

} //namepsace tigl
