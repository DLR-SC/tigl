#pragma once

#include "generated/CPACSEtaXsiPoint.h"

namespace tigl
{
class CCPACSEtaXsiPoint : public generated::CPACSEtaXsiPoint
{
public:
    TIGL_EXPORT CCPACSEtaXsiPoint(CCPACSWingSparPosition* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSEtaXsiPoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr);
    TIGL_EXPORT CCPACSEtaXsiPoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr);

    TIGL_EXPORT void SetEta(const double& value) override;
    TIGL_EXPORT void SetXsi(const double& value) override;
    TIGL_EXPORT void SetReferenceUID(const std::string& value) override;

private:
    void InvalidateParent() const;
};
}
