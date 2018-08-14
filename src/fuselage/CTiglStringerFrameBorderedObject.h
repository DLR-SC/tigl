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
#include <boost/variant.hpp>
#include <TopoDS_Shape.hxx>
#include <gp_Ax1.hxx>

#include <string>

#include "tigl.h"
#include "tigl_internal.h"
#include "Cache.h"

class TopoDS_Face;
class TopoDS_Edge;

namespace tigl
{
class CTiglUIDManager;
class CCPACSFuselage;

class CTiglStringerFrameBorderedObject
{
public:
    TIGL_EXPORT CTiglStringerFrameBorderedObject(const CTiglUIDManager& uidMgr, const CCPACSFuselage& fuselage,
                                                 std::string& startFrameUID, std::string& endFrameUID,
                                                 std::string& startStringerUID,
                                                 boost::variant<std::string&, boost::optional<std::string>&> endStringerUID);

    TIGL_EXPORT void Invalidate();
    TIGL_EXPORT TopoDS_Shape GetGeometry(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    TIGL_EXPORT bool Contains(const TopoDS_Face& face) const; // in global coords
    TIGL_EXPORT bool Contains(const TopoDS_Edge& edge) const; // in global coords
    TIGL_EXPORT bool Contains(const gp_Pnt& point) const; // in global coords

private:
    struct BorderCache {
        gp_Ax1 sFrame_sStringer;
        gp_Ax1 sFrame_eStringer;
        gp_Ax1 eFrame_sStringer;
        gp_Ax1 eFrame_eStringer;
    };

    void BuildGeometry(TopoDS_Shape& cache) const;
    void UpdateBorders(BorderCache& cache) const;
    void UpdateBorder(gp_Ax1& b, TopoDS_Shape s1, TopoDS_Shape s2) const;
    std::string GetEndStringerUid() const;

private:
    const CTiglUIDManager& m_uidMgr;
    const CCPACSFuselage& m_fuselage;
    std::string& m_startFrameUID;
    std::string& m_endFrameUID;
    std::string& m_startStringerUID;
    boost::variant<std::string&, boost::optional<std::string>&> m_endStringerUID;

    Cache<BorderCache, CTiglStringerFrameBorderedObject> m_borderCache;
    Cache<TopoDS_Shape, CTiglStringerFrameBorderedObject> m_geometry;
};

} // namespace tigl
