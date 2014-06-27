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
* @brief  Definition of the interface which encapsulates different wing profile algorithms (point list, CST profile, etc.).
*/

#ifndef ITIGLWINGPROFILEALGO_H
#define ITIGLWINGPROFILEALGO_H

#include <vector>
#include <string>
#include "tixi.h"
#include "tigl.h"
#include "CTiglPoint.h"

class TopoDS_Wire;
class TopoDS_Edge;

namespace tigl 
{
class ITiglWingProfileAlgo
{
public:
    // Clean up 
    virtual void Cleanup(void)  = 0;

    virtual void Update(void)   = 0;

    // Returns the algorithm code identifier for an algorithm
    virtual void ReadCPACS(TixiDocumentHandle tixiHandle)  = 0;

    // Returns points on profile.
    virtual std::vector<CTiglPoint*> GetSamplePoints() const = 0;

    // get profiles CPACS XML path
    virtual const std::string & GetProfileDataXPath() const = 0;

    // get upper wing profile wire
    virtual const TopoDS_Edge & GetUpperEdge() const = 0;

    // get lower wing profile wire
    virtual const TopoDS_Edge & GetLowerEdge() const = 0;

    // get the upper and lower wing profile combined into one edge
    virtual const TopoDS_Edge & GetUpperLowerEdge() const = 0;

    // get trailing edge
    virtual const TopoDS_Edge & GetTrailingEdge() const = 0;

    // get leading edge point();
    virtual const gp_Pnt & GetLEPoint() const = 0;
    
    // get trailing edge point();
    virtual const gp_Pnt & GetTEPoint() const = 0;
};

} // end namespace tigl

#endif // ITIGLWINGPROFILEALGO_H
