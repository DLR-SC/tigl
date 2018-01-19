#pragma once

#include "generated/CPACSCargoCrossBeamsAssembly.h"

namespace tigl
{
    class CCPACSCargoCrossBeamsAssembly : public generated::CPACSCargoCrossBeamsAssembly {
    public:
        TIGL_EXPORT CCPACSCargoCrossBeamsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
