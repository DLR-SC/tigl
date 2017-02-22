/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-08 Martin Siggel Martin.Siggel@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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
* @brief  Implementation of an abstract segment class
*/

#include "CTiglAbstractSegment.h"
#include "TopoDS_Shape.hxx"

namespace tigl
{

CTiglAbstractSegment::CTiglAbstractSegment(int segindex, CCPACSTransformation& trans, TiglSymmetryAxis& axis)
    : CTiglAbstractGeometricComponent(trans, axis), mySegmentIndex(segindex), invalidated(true), continuity(C0) {}

CTiglAbstractSegment::CTiglAbstractSegment(int segindex, CCPACSTransformation& trans, boost::optional<TiglSymmetryAxis>& axis)
    : CTiglAbstractGeometricComponent(trans, axis), mySegmentIndex(segindex), invalidated(true), continuity(C0) {}

void CTiglAbstractSegment::Invalidate()
{
    loft.reset();
    invalidated = true;
}

void CTiglAbstractSegment::Cleanup()
{
    CTiglAbstractGeometricComponent::Reset();
    Invalidate();
    SetUID("");
}

// Returns the segment index of this segment
int CTiglAbstractSegment::GetSegmentIndex() const
{
    return mySegmentIndex;
}

TiglContinuity CTiglAbstractSegment::GetContinuity() const 
{
    return continuity;
}

} // end namespace tigl
