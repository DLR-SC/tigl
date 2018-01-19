#pragma once

#include "generated/CPACSLongFloorBeam.h"

namespace tigl
{
    class CCPACSLongFloorBeam : public generated::CPACSLongFloorBeam {
    public:
        TIGL_EXPORT CCPACSLongFloorBeam(CCPACSLongFloorBeamsAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
