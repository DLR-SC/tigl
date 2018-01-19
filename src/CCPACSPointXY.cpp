#include "CCPACSPointXY.h"

namespace tigl
{
    CCPACSPointXY::CCPACSPointXY(CTiglUIDManager * uidMgr)
        : generated::CPACSPointXY(uidMgr) {}

    gp_Pnt2d CCPACSPointXY::As_gp_Pnt() const
    {
        return gp_Pnt2d(m_x, m_y);
    }
}
