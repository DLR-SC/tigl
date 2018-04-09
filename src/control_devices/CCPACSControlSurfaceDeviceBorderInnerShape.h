/*
 * Copyright (C) 2007-2017 German Aerospace Center (DLR/SC)
 *
 * Created: 2017-02-28 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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

#ifndef CCPACSCONTROLSURFACEDEVICEBORDERINNERSHAPE_H
#define CCPACSCONTROLSURFACEDEVICEBORDERINNERSHAPE_H

#include <string>

#include "tixi.h"
#include "CTiglError.h"
#include "tigl_internal.h"
#include "CSharedPtr.h"

namespace tigl
{

class CCPACSControlSurfaceDeviceBorderInnerShape
{
public:
    TIGL_EXPORT CCPACSControlSurfaceDeviceBorderInnerShape();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
                               const std::string & innerShapeXPath,
                               TiglControlSurfaceType type = LEADING_EDGE_DEVICE);

    TIGL_EXPORT double getRelHeightTE() const;
    TIGL_EXPORT double getXsiTE() const;

private:
    /* LeadingEdgeShape */
    double relHeightTE;
    double xsiTE;
};

typedef CSharedPtr<CCPACSControlSurfaceDeviceBorderInnerShape> CCPACSControlSurfaceDeviceBorderInnerShapePtr;

} // end namespace tigl

#endif // CCPACSCONTROLSURFACEDEVICEBORDERINNERSHAPE_H
