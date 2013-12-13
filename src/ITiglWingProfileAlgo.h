/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-12 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Definition of the interface which encapsulates different wing profile algorithms (constructors). 
*/

#ifndef ITIGLWINGPROFILEALGO_H
#define ITIGLWINGPROFILEALGO_H

#include <vector>
#include "tixi.h"
#include "tigl.h"
#include "CTiglPoint.h"

namespace tigl 
{

    class ITiglWingProfileAlgo
    {
        public:
        // Clean up 
        virtual void Cleanup(void)  = 0;

        // Returns the algorithm code identifier for an algorithm
        virtual void ReadCPACS(TixiDocumentHandle tixiHandle)  = 0;


        // Returns the profile points as read from TIXI.
        virtual std::vector<CTiglPoint*> GetCoordinateContainer()  = 0;

        // Builds the wing profile wires.
        virtual void BuildWires(gp_Pnt & upperPoint)  = 0;

        // Builds leading and trailing edge points of the wing profile wire.
        virtual void BuildLETEPoints(void) = 0;

        public:
        std::string               ProfileDataXPath;   /**< CPACS path to profile data (pointList or cst2D) */
        TopoDS_Wire               wireClosed;     /**< Forced closed wing profile wire */
        TopoDS_Wire               wireOriginal;   /**< Original wing profile wire */
        TopoDS_Wire               upperWire;      /**< wire of the upper wing profile */
        TopoDS_Wire               lowerWire;      /**< wire of the lower wing profile */
        gp_Pnt                    lePoint;        /**< Leading edge point */
        gp_Pnt                    tePoint;        /**< Trailing edge point */
    };

} // end namespace tigl

#endif // ITIGLWINGPROFILEALGO_H
