#pragma once

#include "generated/CPACSFuselageStructure.h"

namespace tigl
{
    class CCPACSFuselageStructure : public generated::CPACSFuselageStructure {
    public:
        TIGL_EXPORT CCPACSFuselageStructure(CCPACSFuselage* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
