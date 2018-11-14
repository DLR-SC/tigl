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
* @brief  Interface of CPACS wing/fuselage segment handling routines.
*/

#ifndef CTIGLABTRACTSEGMENT_H
#define CTIGLABTRACTSEGMENT_H

#include <boost/optional.hpp>
#include <cassert>
#include <vector>
#include <algorithm>
#include "generated/UniquePtr.h"
#include "tigl.h"
#include "tigl_internal.h"
#include "CTiglError.h"
#include "CTiglAbstractGeometricComponent.h"

namespace tigl
{

// Enumeration for segment types
enum SegmentType
{
    INNER_SEGMENT,
    MID_SEGMENT,
    OUTER_SEGMENT,
    INNER_OUTER_SEGMENT
};

template <typename SegmentType>
class CTiglAbstractSegment : public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CTiglAbstractSegment(const std::vector<unique_ptr<SegmentType>>& segments, const CTiglAbstractGeometricComponent* parentComponent)
        : _segments(segments), _parentComponent(parentComponent), _continuity(::C0) {}

    // Returns the segment index of this segment
    int GetSegmentIndex() const
    {
        for (std::size_t i = 0; i < _segments.size(); i++) {
            if (_segments[i].get() == static_cast<const SegmentType*>(this)) {
                return static_cast<int>(i + 1);
            }
        }
        throw CTiglError("Invalid parent?"); // cannot happen, this is always part of the container of its parent class
    }

    TiglSymmetryAxis GetSymmetryAxis() const OVERRIDE
    {
        if (_parentComponent) {
            return _parentComponent->GetSymmetryAxis();
        }
        else {
            return TIGL_NO_SYMMETRY;
        }
    }

    // Returns the continuityof the connection to the next segment
    TiglContinuity GetContinuity() const
    {
        return _continuity;
    }

protected:
    const std::vector<unique_ptr<SegmentType>>& _segments;       /**< References the segment collection of the parent container element in the CPACS tree */
    const CTiglAbstractGeometricComponent*      _parentComponent; /**< References the symmetry of a parent element in the CPACS tree */
    TiglContinuity                              _continuity;     /**< Continuity of the connection to the next segment */

};  // end class CTiglAbstractSegment

} // end namespace tigl

#endif // CTIGLABTRACTSEGMENT_H
