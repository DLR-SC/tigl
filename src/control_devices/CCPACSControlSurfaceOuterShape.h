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
 * @brief  Implementation of ..
 */

#ifndef CCPACSControlSurfaceOuterShape_H
#define CCPACSControlSurfaceOuterShape_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSControlSurfaceBorder.h"
#include "tigl_internal.h"


#include "TopoDS_Face.hxx"

namespace tigl
{

class CCPACSControlSurfaceOuterShape
{

private:
    // intermediateAirfoils
    CCPACSControlSurfaceBorder outerBorder;
    CCPACSControlSurfaceBorder innerBorder;

public:
    TIGL_EXPORT CCPACSControlSurfaceOuterShape();

    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle,
            const std::string & outerShapeXPath);

    TIGL_EXPORT const CCPACSControlSurfaceBorder& getInnerBorder();
    TIGL_EXPORT const CCPACSControlSurfaceBorder& getOuterBorder();

};

} // end namespace tigl

#endif // CCPACSControlSurfaceOuterShape_H
