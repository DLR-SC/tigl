#pragma once

#include "generated/CPACSEtaXsiRelHeightPoint.h"
#include "EtaXsi.h"

namespace tigl
{
class CCPACSEtaXsiRelHeightPoint : public generated::CPACSEtaXsiRelHeightPoint
{
public:
    TIGL_EXPORT CCPACSEtaXsiRelHeightPoint(CCPACSWingSparPosition* parent);

    TIGL_EXPORT void SetEta(const double& value) OVERRIDE;
    TIGL_EXPORT void SetXsi(const double& value) OVERRIDE;
    TIGL_EXPORT void SetRelHeight(const boost::optional<double>& value) OVERRIDE;
    TIGL_EXPORT void SetReferenceUID(const std::string& value) OVERRIDE;

    //TIGL_EXPORT EtaXsi ComputeCSOrTEDEtaXsi() const;

private:
    void InvalidateParent();
};
}
