#include "CCPACSCurvePoint.h"

#include "CTiglUIDManager.h"

namespace tigl
{

CCPACSCurvePoint::CCPACSCurvePoint(CCPACSWingRibExplicitPositioning* parent, CTiglUIDManager* uidMgr)
: generated::CPACSCurvePoint(parent, uidMgr) {}

CCPACSCurvePoint::CCPACSCurvePoint(CCPACSWingRibsPositioning* parent, CTiglUIDManager* uidMgr)
: generated::CPACSCurvePoint(parent, uidMgr) {}

void CCPACSCurvePoint::SetEta(const double& value)
{
    generated::CPACSCurvePoint::SetEta(value);
    InvalidateParent();
}

void CCPACSCurvePoint::SetReferenceUID(const std::string& value)
{
    generated::CPACSCurvePoint::SetReferenceUID(value);
    InvalidateParent();

}

void CCPACSCurvePoint::InvalidateParent() const
{
    const CTiglUIDObject* parent = GetNextUIDParent();
    if (parent) {
        parent->Invalidate();
    }
}

}
