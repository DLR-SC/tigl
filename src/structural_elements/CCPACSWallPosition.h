/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-10-16 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "generated/CPACSWallPosition.h"
#include "gp_Pnt.hxx"
#include "TopoDS_Shape.hxx"

namespace tigl
{

class CCPACSWalls;
class CCPACSFuselage;

class CCPACSWallPosition : public generated::CPACSWallPosition
{
public:
    TIGL_EXPORT CCPACSWallPosition(CCPACSWallPositions* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT gp_Pnt& GetBasePoint();
    TIGL_EXPORT gp_Pnt const& GetBasePoint() const;

    TIGL_EXPORT boost::optional<TopoDS_Shape>& GetShape();
    TIGL_EXPORT boost::optional<TopoDS_Shape> const& GetShape() const;

    // overriding setters for invalidation
    TIGL_EXPORT void SetBulkheadUID_choice1(const boost::optional<std::string>& value) override;
    TIGL_EXPORT void SetWallSegmentUID_choice2(const boost::optional<std::string>& value) override;
    TIGL_EXPORT void SetFuselageSectionUID_choice3(const boost::optional<std::string>& value) override;
    TIGL_EXPORT void SetX_choice4(const boost::optional<double>& value) override;
    TIGL_EXPORT void SetY(const double& value) override;
    TIGL_EXPORT void SetZ(const double& value) override;

private:
    void InvalidateImpl(const boost::optional<std::string>& source) const override;

    void  CalcBasePointAndShape() const;
    const CCPACSWalls& GetWalls() const;
    const CCPACSFuselage& GetFuselage() const;

    mutable bool isBuilt {false};
    mutable gp_Pnt base_point;
    mutable boost::optional<TopoDS_Shape> shape;
};

}
