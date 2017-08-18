/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-19 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of a CPACS guide curve
*/

#include "tigl.h"
#include "CCPACSGuideCurve.h"
#include "CTiglError.h"
#include "CTiglPoint.h"
#include "CTiglLogging.h"

#include <TopoDS_Edge.hxx>

namespace tigl
{

// Constructor
CCPACSGuideCurve::CCPACSGuideCurve(CTiglUIDManager* uidMgr)
    : generated::CPACSGuideCurve(uidMgr)
{
    Cleanup();
}

// Destructor
CCPACSGuideCurve::~CCPACSGuideCurve(void)
{
    Cleanup();
}

// Cleanup routine
void CCPACSGuideCurve::Cleanup(void)
{
    nextGuideSegment = NULL;
    guideCurveTopo.Nullify();
    builder = NULL;
    isBuild = false;
}

// This will be called be the guide curve builder
void CCPACSGuideCurve::SetCurve(const TopoDS_Edge& edge)
{
    isBuild = true;
    guideCurveTopo = edge;
}

const TopoDS_Edge& CCPACSGuideCurve::GetCurve()
{
    if (builder && !isBuild) {
        builder->BuildGuideCurve(this);
    }
    return guideCurveTopo;
}

void CCPACSGuideCurve::ConnectToCurve(CCPACSGuideCurve *guide)
{
    if (!guide) {
        throw CTiglError("Null pointer guide curve in CCPACSGuideCurve::ConnectToCurve", TIGL_ERROR);
    }
    
    if (!guide->GetFromGuideCurveUID_choice1() || *(guide->GetFromGuideCurveUID_choice1()) != m_uID) {
        throw CTiglError("Guide curves cannot be connected. Mismatching uids.", TIGL_ERROR);
    }
    
    nextGuideSegment = guide;
}

CCPACSGuideCurve* CCPACSGuideCurve::GetConnectedCurve() const
{
    return nextGuideSegment;
}

void CCPACSGuideCurve::SetGuideCurveBuilder(IGuideCurveBuilder* b)
{
    builder = b;
}

} // end namespace tigl
