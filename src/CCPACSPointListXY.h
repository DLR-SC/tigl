#pragma once

#include "generated/CPACSPointListXY.h"

namespace tigl
{
    class CCPACSPointListXY : public generated::CPACSPointListXY {
    public:
        TIGL_EXPORT CCPACSPointListXY(CTiglUIDManager* uidMgr);

        TIGL_EXPORT const CCPACSPointXY& GetPoint(const std::string& uid) const;
    };
}
