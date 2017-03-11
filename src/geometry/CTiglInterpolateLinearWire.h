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
* @brief  Implementation of routines for building a wire from a std::vector
*         of points by linear interpolation.
*/

#ifndef CTIGLINTERPOLATELINEARWIRE_H
#define CTIGLINTERPOLATELINEARWIRE_H

#include "tigl_internal.h"
#include "ITiglWireAlgorithm.h"

namespace tigl 
{

class CTiglInterpolateLinearWire : public ITiglWireAlgorithm
{

public:
    // Constructor
    TIGL_EXPORT CTiglInterpolateLinearWire();

    // Destructor
    TIGL_EXPORT virtual ~CTiglInterpolateLinearWire();

    // Builds the wire from the given points
    TIGL_EXPORT virtual TopoDS_Wire BuildWire(const CPointContainer& points, bool forceClosed = false) const;

    // Returns the algorithm code identifier for an algorithm
    TIGL_EXPORT virtual TiglAlgorithmCode GetAlgorithmCode() const;

    // Returns the point on the wire with the smallest x value
    TIGL_EXPORT virtual gp_Pnt GetPointWithMinX(const CPointContainer& points) const;

    // Returns the point on the wire with the biggest x value
    TIGL_EXPORT virtual gp_Pnt GetPointWithMaxX(const CPointContainer& points) const;

    // Returns the point on the wire with the smallest y value
    TIGL_EXPORT virtual gp_Pnt GetPointWithMinY(const CPointContainer& points) const;

    // Returns the point on the wire with the biggest y value
    TIGL_EXPORT virtual gp_Pnt GetPointWithMaxY(const CPointContainer& points) const;

private:
    // Copy constructor
    CTiglInterpolateLinearWire(const CTiglInterpolateLinearWire& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CTiglInterpolateLinearWire& )             { /* Do nothing */ }
};

} // end namespace tigl

#endif // CTIGLINTERPOLATELINEARWIRE_H
