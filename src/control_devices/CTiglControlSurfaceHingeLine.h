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

#ifndef CTiglControlSurfaceHingeLine_H
#define CTiglControlSurfaceHingeLine_H

#include "tigl_internal.h"

#include "gp_Pnt.hxx"

// TODO: Forward declare
#include "generated/CPACSControlSurfacePath.h"


namespace tigl
{

class CCPACSControlSurfaceOuterShapeTrailingEdge;


class CCPACSWingComponentSegment;

class CTiglControlSurfaceHingeLine
{
public:
    TIGL_EXPORT CTiglControlSurfaceHingeLine(const CCPACSControlSurfaceOuterShapeTrailingEdge* outerShape,
                                             const CCPACSControlSurfacePath* path, CCPACSWingComponentSegment* segment);

    TIGL_EXPORT gp_Pnt getInnerHingePoint() const;
    TIGL_EXPORT gp_Pnt getOuterHingePoint() const;
    
    void invalidate();

private:
    void buildHingeLine() const;
    
    const CCPACSControlSurfaceOuterShapeTrailingEdge* _outerShape;
    const CCPACSControlSurfacePath* _path;
    CCPACSWingComponentSegment* _segment;
    
    mutable bool _invalidated;
    
    mutable gp_Pnt _innerHingePoint;
    mutable gp_Pnt _outerHingePoint;
};

} // end namespace tigl

#endif // CTiglControlSurfaceHingeLine_H
