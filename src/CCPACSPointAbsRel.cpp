#include "CCPACSPointAbsRel.h"

namespace tigl
{
    CTiglPoint CCPACSPointAbsRel::AsPoint() const
    {
        CTiglPoint point;
        // TODO: in case m_x, m_y ot m_z is not valid, we can set the corresponding coord to zero, thus AsPoint() never throws
        point.x = *m_x;
        point.y = *m_y;
        point.z = *m_z;
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
