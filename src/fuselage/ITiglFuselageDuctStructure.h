/*
* Copyright (C) 2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-16 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#pragma once

#include "tigl.h"
#include "Cache.h"
#include "CTiglRelativelyPositionedComponent.h"

class TopoDS_Shape;
class TopoDS_Wire;
class gp_Lin;
class gp_Pnt;

namespace tigl {

class CCPACSFuselageStringerFramePosition;

/**
 * @brief The ITiglFuselageDuctStructure class provides some common
 * geometric functionality needed by structural elements of both
 * ducts and fuselages
 */
class ITiglFuselageDuctStructure
{
public:
    TIGL_EXPORT ITiglFuselageDuctStructure(CTiglRelativelyPositionedComponent const* parent);

    TIGL_EXPORT void Invalidate() const;

    TIGL_EXPORT TopoDS_Shape const& GetLoft() const;

    // calculates the intersection of the stringer frame position with the parent loft (duct/fuselage)
    TIGL_EXPORT gp_Lin Intersection(CCPACSFuselageStringerFramePosition const& pos) const;
    TIGL_EXPORT gp_Lin Intersection(gp_Pnt pRef, double angleRef) const;

    // project the edge/wire onto the fuselage/duct loft
    TIGL_EXPORT TopoDS_Wire projectConic(TopoDS_Shape wireOrEdge, gp_Pnt origin) const;
    TIGL_EXPORT TopoDS_Wire projectParallel(TopoDS_Shape wireOrEdge, gp_Dir direction) const;

private:

    void StoreLoft(TopoDS_Shape& cache) const;

    CTiglRelativelyPositionedComponent const* m_parent;
    Cache<TopoDS_Shape, ITiglFuselageDuctStructure> m_loft;

};

} //namespace tigl
