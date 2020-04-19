#include "CCPACSEtaXsiPoint.h"

#include "CTiglUIDManager.h"

namespace tigl
{
CCPACSEtaXsiPoint::CCPACSEtaXsiPoint()
: generated::CPACSEtaXsiPoint((CCPACSWingSparPosition*)nullptr, nullptr) {}
CCPACSEtaXsiPoint::CCPACSEtaXsiPoint(CCPACSWingSparPosition* parent, CTiglUIDManager* uidMgr)
: generated::CPACSEtaXsiPoint(parent, uidMgr) {}
CCPACSEtaXsiPoint::CCPACSEtaXsiPoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr)
: generated::CPACSEtaXsiPoint(parent, uidMgr) {}
CCPACSEtaXsiPoint::CCPACSEtaXsiPoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr)
: generated::CPACSEtaXsiPoint(parent, uidMgr) {}


void CCPACSEtaXsiPoint::SetEta(const double& value)
{
    generated::CPACSEtaXsiPoint::SetEta(value);
    InvalidateParent();
}

void CCPACSEtaXsiPoint::SetXsi(const double& value)
{
    generated::CPACSEtaXsiPoint::SetXsi(value);
    InvalidateParent();
}

void CCPACSEtaXsiPoint::SetReferenceUID(const std::string& value)
{
    generated::CPACSEtaXsiPoint::SetReferenceUID(value);
    InvalidateParent();
}


void CCPACSEtaXsiPoint::InvalidateParent() const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate();
    }
}

}
