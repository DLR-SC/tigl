#pragma once

#include "generated/CPACSCargoCrossBeamStrutsAssembly.h"

namespace tigl
{
    class CCPACSCargoCrossBeamStrutsAssembly : public generated::CPACSCargoCrossBeamStrutsAssembly {
    public:
        TIGL_EXPORT CCPACSCargoCrossBeamStrutsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
