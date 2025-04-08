/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Implementation of CPACS fuselage segments handling routines.
*/

#ifndef CCPACSFUSELAGESEGMENTS_H
#define CCPACSFUSELAGESEGMENTS_H

#include <string>
#include <boost/optional.hpp>

#include "generated/CPACSFuselageSegments.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "TopoDS_Compound.hxx"

namespace tigl
{
class CCPACSFuselageSegment;
class CCPACSConfiguration;

class CCPACSFuselageSegments : public generated::CPACSFuselageSegments
{
public:
    TIGL_EXPORT CCPACSFuselageSegments(CCPACSDuct* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSFuselageSegments(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT CCPACSConfiguration const& GetConfiguration() const;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate(const boost::optional<std::string>& source = boost::none) const;

    // Gets the parent component
    TIGL_EXPORT CTiglRelativelyPositionedComponent const* GetParentComponent() const;


    // CPACSFuselageSegments interface
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath) override;

    TIGL_EXPORT const TopoDS_Compound& GetGuideCurveWires() const;

private:
    void ReorderSegments();

    void BuildGuideCurves(TopoDS_Compound& cache) const;

    Cache<TopoDS_Compound, CCPACSFuselageSegments> guideCurves;
};

} // end namespace tigl

#endif // CCPACSFUSELAGESEGMENTS_H
