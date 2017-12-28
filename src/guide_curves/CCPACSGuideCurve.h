/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-10 Tobias Stollenwerk <tobias.stollenwerk@dlr.de>
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
* @brief  Implementation of a CPACS guide curve profile
*/

#ifndef CCPACSGUIDECURVE_H
#define CCPACSGUIDECURVE_H

#include "tigl_internal.h"
#include "tixi.h"
#include "CSharedPtr.h"

#include "generated/CPACSGuideCurve.h"

#include <gp_Pnt.hxx>
#include <TopoDS_Edge.hxx>

#include <vector>
#include <string>

#include "generated/UniquePtr.h"

namespace tigl
{

class CTiglPoint;
typedef class CSharedPtr<CTiglPoint> PCTiglPoint;

class IGuideCurveBuilder;

class CCPACSGuideCurve : public generated::CPACSGuideCurve
{

private:
    // Typedefs for a container to store the coordinates of a guide curve element.
    typedef std::vector<PCTiglPoint> CCPACSGuideCurvePoints;

public:
    // Constructor
    TIGL_EXPORT CCPACSGuideCurve(CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSGuideCurve(void) OVERRIDE;

    TIGL_EXPORT const TopoDS_Edge& GetCurve();

    // Connects the current guide curve segment with another segment guide
    // This implies, that guide.fromGuideCurveUID == this.uid
    TIGL_EXPORT void ConnectToCurve(CCPACSGuideCurve* guide);

    TIGL_EXPORT CCPACSGuideCurve* GetConnectedCurve() const;

    TIGL_EXPORT void SetGuideCurveBuilder(IGuideCurveBuilder& b);
protected:
    // Cleanup routine
    void Cleanup(void);

private:
    // Copy constructor
    CCPACSGuideCurve(const CCPACSGuideCurve&);

    // Assignment operator
    void operator=(const CCPACSGuideCurve&);

    TopoDS_Edge guideCurveTopo;           /**< Actual topological entity of the curve */
    CCPACSGuideCurve* nextGuideSegment;   /**< Pointer to a guide curve segment that is connected to this segment */

    IGuideCurveBuilder* m_builder;
    bool isBuild;                         /**< Checks whether the guide curve is already built */

};

class IGuideCurveBuilder
{
public:
    virtual TopoDS_Edge BuildGuideCurve(CCPACSGuideCurve*) = 0;
};

} // end namespace tigl

#endif // CCPACSGUIDECURVE_H

