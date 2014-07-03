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

#ifndef CTiglControlSurfaceHingeLine_H
#define CTiglControlSurfaceHingeLine_H

#include "tigl_internal.h"

#include "gp_Pnt.hxx"

#include "CCPACSControlSurfaceOuterShape.h"
#include "CCPACSControlSurfaceDevicePath.h"


namespace tigl
{

class CCPACSWingComponentSegment;

class CTiglControlSurfaceHingeLine
{

private:
    gp_Pnt innerHingePoint;
    gp_Pnt outerHingePoint;

public:
    TIGL_EXPORT CTiglControlSurfaceHingeLine(CCPACSControlSurfaceOuterShape outerShape,
                                             CCPACSControlSurfaceDevicePath path, CCPACSWingComponentSegment* segment);

    TIGL_EXPORT gp_Pnt getTransformedInnerHingePoint(gp_Vec translation);
    TIGL_EXPORT gp_Pnt getTransformedOuterHingePoint(gp_Vec translation);

    TIGL_EXPORT gp_Pnt getInnerHingePoint();
    TIGL_EXPORT gp_Pnt getOuterHingePoint();

};

} // end namespace tigl

#endif // CTiglControlSurfaceHingeLine_H
