#pragma once

#include "generated/CPACSCrossBeamAssemblyPosition.h"

namespace tigl
{
    class CCPACSFrame;

    class CCPACSCrossBeamAssemblyPosition : public generated::CPACSCrossBeamAssemblyPosition {
    public:
        TIGL_EXPORT CCPACSCrossBeamAssemblyPosition(CCPACSCargoCrossBeamsAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
