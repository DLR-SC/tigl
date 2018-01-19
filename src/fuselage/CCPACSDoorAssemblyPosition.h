#pragma once

#include "generated/CPACSDoorAssemblyPosition.h"

namespace tigl
{
    class CCPACSDoorAssemblyPosition : public generated::CPACSDoorAssemblyPosition {
    public:
        CCPACSDoorAssemblyPosition(CCPACSCargoDoorsAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated;
    };
}
