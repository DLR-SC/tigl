/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include <boost/optional.hpp>
#include <gp_Ax1.hxx>
#include <TopoDS_Shape.hxx>

#include "generated/CPACSSkinSegment.h"

class TopoDS_Face;
class TopoDS_Edge;

namespace tigl
{
class CCPACSSkinSegment : public generated::CPACSSkinSegment
{
public:
    TIGL_EXPORT CCPACSSkinSegment(CCPACSSkinSegments* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void Invalidate();

    TIGL_EXPORT bool Contains(const TopoDS_Face& face);
    TIGL_EXPORT bool Contains(const TopoDS_Edge& edge);
    TIGL_EXPORT bool Contains(const gp_Pnt& point);

private:
    void UpdateBorders();
    void UpdateBorder(gp_Ax1& b, TopoDS_Shape s1, TopoDS_Shape s2);

private:
    struct BorderCache {
        gp_Ax1 sFrame_sStringer;
        gp_Ax1 sFrame_eStringer;
        gp_Ax1 eFrame_sStringer;
        gp_Ax1 eFrame_eStringer;
    };

    boost::optional<BorderCache> m_borderCache;
};
}
