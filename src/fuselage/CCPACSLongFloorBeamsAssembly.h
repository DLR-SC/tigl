#pragma once

#include "generated/CPACSLongFloorBeamsAssembly.h"

namespace tigl
{
    class CCPACSLongFloorBeamsAssembly : public generated::CPACSLongFloorBeamsAssembly {
    public:
        TIGL_EXPORT CCPACSLongFloorBeamsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
