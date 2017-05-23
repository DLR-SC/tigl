#include "CCPACSPoint.h"

namespace tigl
{
    CCPACSPoint::CCPACSPoint(CTiglUIDManager* uidMgr)
        : generated::CPACSPoint(uidMgr) {}

    CTiglPoint CCPACSPoint::AsPoint() const
    {
        CTiglPoint point;
        point.x = m_x ? *m_x : 0;
        point.y = m_y ? *m_y : 0;
        point.z = m_z ? *m_z : 0;
        return point;
    }

    void CCPACSPoint::SetAsPoint(const CTiglPoint& point)
    {
        m_x = point.x;
        m_y = point.y;
        m_z = point.z;
    }
}
