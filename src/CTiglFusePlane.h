/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-11-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLFUSEPLANE_H
#define CTIGLFUSEPLANE_H

#include "CNamedShape.h"

namespace tigl {

class CCPACSConfiguration;
class CTiglAbstractPhysicalComponent;

enum TiglFuseResultMode{
    HALF_PLANE = 0,
    FULL_PLANE = 1
};

class CTiglFusePlane
{
public:
    CTiglFusePlane(CCPACSConfiguration& config);

    // sets mode for plane generation, by default a half plane is created
    void SetResultMode(TiglFuseResultMode mode);

private:
    void Perform();

    CNamedShape _result;
    CCPACSConfiguration& _myconfig;
    TiglFuseResultMode _mymode;
};

} // namespace tigl

#endif // CTIGLFUSEPLANE_H
