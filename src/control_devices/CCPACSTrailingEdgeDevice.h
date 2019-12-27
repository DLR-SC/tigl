#pragma once
/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "generated/CPACSTrailingEdgeDevice.h"

#include "CTiglAbstractGeometricComponent.h"
#include "Cache.h"
#include "tigl.h"
#include <string>

namespace tigl
{

class CCPACSTrailingEdgeDevice : public generated::CPACSTrailingEdgeDevice, public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CCPACSTrailingEdgeDevice(CCPACSTrailingEdgeDevices* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath) override;

    TIGL_EXPORT std::string GetShortName() const;

    // Returns the flap transformation based on the current deflection
    TIGL_EXPORT gp_Trsf GetFlapTransform() const;

    TIGL_EXPORT double GetMinDeflection() const;
    TIGL_EXPORT double GetMaxDeflection() const;

    // Get and set the current deflection  value
    TIGL_EXPORT double GetDeflection() const;
    TIGL_EXPORT void SetDeflection(const double deflect);

    TIGL_EXPORT PNamedShape GetCutOutShape(void) const;
    TIGL_EXPORT PNamedShape GetFlapShape() const;
    TIGL_EXPORT PNamedShape GetTransformedFlapShape() const;
    TIGL_EXPORT gp_Vec GetNormalOfControlSurfaceDevice() const;

    TIGL_EXPORT TiglControlSurfaceType GetType() const;

    // Interface functions from CTiglAbstractGeometricComponent
    TIGL_EXPORT std::string GetDefaultedUID() const override;
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const override;
    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const override;

private:
    struct HingePoints {
        gp_Pnt inner;
        gp_Pnt outer;
    };

    Cache<HingePoints, CCPACSTrailingEdgeDevice> m_hingePoints;
    TiglControlSurfaceType m_type;
    double m_currentDeflection;

    PNamedShape BuildLoft() const override;
    void ComputeHingePoints(HingePoints&) const;
    void Invalidate();

    const CCPACSWing& Wing() const;
    CCPACSWing& Wing();
};

TIGL_EXPORT const CCPACSWingComponentSegment& ComponentSegment(const CCPACSTrailingEdgeDevice&);
TIGL_EXPORT CCPACSWingComponentSegment& ComponentSegment(CCPACSTrailingEdgeDevice&);

} // namespace tigl
