#pragma once

#include "generated/CPACSStringersAssembly.h"

namespace tigl
{
    class CCPACSStringersAssembly : public generated::CPACSStringersAssembly {
    public:
        TIGL_EXPORT CCPACSStringersAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
