#include "CCPACSEtaXsiRelHeightPoint.h"

#include "CTiglUIDManager.h"
#include "tigletaxsifunctions.h"
#include "CCPACSWingSparPosition.h"

namespace tigl
{

CCPACSEtaXsiRelHeightPoint::CCPACSEtaXsiRelHeightPoint(CCPACSWingSparPosition* parent)
    : generated::CPACSEtaXsiRelHeightPoint(parent)
{
}

void CCPACSEtaXsiRelHeightPoint::SetEta(const double& value)
{
    generated::CPACSEtaXsiRelHeightPoint::SetEta(value);
    InvalidateParent();
}

void CCPACSEtaXsiRelHeightPoint::SetXsi(const double& value)
{
    generated::CPACSEtaXsiRelHeightPoint::SetXsi(value);
    InvalidateParent();
}

void CCPACSEtaXsiRelHeightPoint::SetRelHeight(const boost::optional<double>& value)
{
    generated::CPACSEtaXsiRelHeightPoint::SetRelHeight(value);
    InvalidateParent();
}

void CCPACSEtaXsiRelHeightPoint::SetReferenceUID(const std::string& value)
{
    generated::CPACSEtaXsiRelHeightPoint::SetReferenceUID(value);
    InvalidateParent();
}

//EtaXsi CCPACSEtaXsiRelHeightPoint::ComputeCSOrTEDEtaXsi() const {
//    return transformEtaXsiToCSOrTed({ m_eta, 0 }, m_referenceUID, std::declval<CTiglUIDManager>()); // TODO
//}

void CCPACSEtaXsiRelHeightPoint::InvalidateParent()
{
    //if (IsParent<CCPACSComponentSegmentPath>()) GetParent<CCPACSComponentSegmentPath>()->Invalidate();
    //if (IsParent<CCPACSWingSparPosition>()) GetParent<CCPACSWingSparPosition>()->Invalidate();
    //if (IsParent<CCPACSWingStringer>()) GetParent<CCPACSWingStringer>()->Invalidate();
}
}
