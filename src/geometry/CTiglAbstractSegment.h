/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Interface of CPACS wing/fuselage segment handling routines.
*/

#ifndef CTIGLABTRACTSEGMENT_H
#define CTIGLABTRACTSEGMENT_H

#include <boost/optional/optional.hpp>
#include <cassert>
#include <vector>
#include <algorithm>
#include "generated/UniquePtr.h"
#include "tigl.h"
#include "tigl_internal.h"
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
    TIGL_EXPORT CTiglAbstractSegment(const std::vector<unique_ptr<SegmentType>>& segments, const boost::optional<TiglSymmetryAxis>& parentSymmetry)
        : _segments(segments), _parentSymmetry(parentSymmetry), _continuity(C0) {}

    // Returns the segment index of this segment
    TIGL_EXPORT int GetSegmentIndex() const
    {
        for (std::size_t i = 0; i < _segments.size(); i++)
            if (_segments[i].get() == static_cast<const SegmentType*>(this))
                return static_cast<int>(i + 1);
        throw CTiglError("Invalid parent?"); // cannot happen, this is always part of the container of its parent class
    }

    TIGL_EXPORT virtual TiglSymmetryAxis GetSymmetryAxis() const OVERRIDE
    {
        if (_parentSymmetry)
            return *_parentSymmetry;
        else
            return TIGL_NO_SYMMETRY;
    }

    // Returns the continuityof the connection to the next segment
    TIGL_EXPORT TiglContinuity GetContinuity() const
    {
        return _continuity;
    }

protected:
    const std::vector<unique_ptr<SegmentType>>& _segments;       /**< References the segment collection of the parent container element in the CPACS tree */
    const boost::optional<TiglSymmetryAxis>&    _parentSymmetry; /**< References the symmetry of a parent element in the CPACS tree */
    TiglContinuity                              _continuity;     /**< Continuity of the connection to the next segment */

};  // end class CTiglAbstractSegment

} // end namespace tigl

#endif // CTIGLABTRACTSEGMENT_H
