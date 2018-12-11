#pragma once

#include "generated/CPACSXsiIsoLine.h"

namespace tigl
{
class CCPACSXsiIsoLine : public generated::CPACSXsiIsoLine
{
public:
    TIGL_EXPORT CCPACSXsiIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent);

    TIGL_EXPORT virtual void SetXsi(const double& value);
    TIGL_EXPORT virtual void SetReferenceUID(const std::string& value);

    //TIGL_EXPORT double ComputeCSOrTEDXsi() const;

private:
    void InvalidateParent();
};
}
