#include "CCPACSXsiIsoLine.h"

#include "CTiglUIDManager.h"
#include "tigletaxsifunctions.h"

namespace tigl
{
CCPACSXsiIsoLine::CCPACSXsiIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent)
    : generated::CPACSXsiIsoLine(parent)
{
}

void CCPACSXsiIsoLine::SetXsi(const double& value)
{
    generated::CPACSXsiIsoLine::SetXsi(value);
    InvalidateParent();
}

void CCPACSXsiIsoLine::SetReferenceUID(const std::string& value)
{
    generated::CPACSXsiIsoLine::SetReferenceUID(value);
    InvalidateParent();
}

//double CCPACSXsiIsoLine::ComputeCSOrTEDXsi() const {
//    return transformEtaXsiToCSOrTed({ 0, m_xsi}, m_referenceUID, std::declval<CTiglUIDManager>()).xsi; // TODO
//}

void CCPACSXsiIsoLine::InvalidateParent()
{
    //if (IsParent<CCPACSControlSurfaceBorderTrailingEdge>()) GetParent<CCPACSControlSurfaceBorderTrailingEdge>()->Invalidate();
}
}