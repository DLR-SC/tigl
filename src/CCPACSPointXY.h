#pragma once

#include <gp_Pnt2d.hxx>

#include "generated/CPACSPointXY.h"

namespace tigl
{
    class CCPACSPointXY : public generated::CPACSPointXY
    {
    public:
        TIGL_EXPORT CCPACSPointXY(CTiglUIDManager* uidMgr);

        TIGL_EXPORT gp_Pnt2d As_gp_Pnt() const;
    };
}
