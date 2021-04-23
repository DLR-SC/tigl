#include "CCPACSCurvePoint.h"

#include "CTiglUIDManager.h"

namespace tigl
{

CCPACSCurvePoint::CCPACSCurvePoint(CCPACSWingRibExplicitPositioning* parent)
: generated::CPACSCurvePoint(parent) {}

CCPACSCurvePoint::CCPACSCurvePoint(CCPACSWingRibsPositioning* parent)
: generated::CPACSCurvePoint(parent) {}

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
