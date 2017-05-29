#include "CCPACSPointAbsRel.h"

namespace tigl
{
    CCPACSPointAbsRel::CCPACSPointAbsRel(CTiglUIDManager* uidMgr)
        : generated::CPACSPointAbsRel(uidMgr) {}

    CTiglPoint CCPACSPointAbsRel::AsPoint() const
    {
        CTiglPoint point;
        point.x = m_x ? *m_x : 0;
        point.y = m_y ? *m_y : 0;
        point.z = m_z ? *m_z : 0;

        return point;
    }

    void CCPACSPointAbsRel::SetAsPoint(const CTiglPoint& point)
    {
        m_x = point.x;
        m_y = point.y;
        m_z = point.z;
    }

    ECPACSTranslationType CCPACSPointAbsRel::GetRefDefaultedType() const
    {
        // TODO: CPACSGen does not yet support optional attributes with default values, remove this code and return m_ref when defaults become available
        if (m_refType) {
            return *m_refType;
        } else {
            return ABS_LOCAL;
        }
    }
}
