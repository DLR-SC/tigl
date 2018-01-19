#pragma once

#include "generated/CPACSCrossBeamStrutAssemblyPosition.h"

namespace tigl
{
    class CCPACSFrame;

    class CCPACSCrossBeamStrutAssemblyPosition : public generated::CPACSCrossBeamStrutAssemblyPosition {
    public:
        TIGL_EXPORT CCPACSCrossBeamStrutAssemblyPosition(CCPACSCargoCrossBeamStrutsAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
