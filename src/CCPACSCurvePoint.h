#pragma once

#include "generated/CPACSCurvePoint.h"

namespace tigl
{
class CCPACSCurvePoint : public generated::CPACSCurvePoint
{
public:
    TIGL_EXPORT CCPACSCurvePoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSCurvePoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void SetEta(const double& value) override;
    TIGL_EXPORT void SetReferenceUID(const std::string& value) override;

private:
    void InvalidateParent() const;
};
}
