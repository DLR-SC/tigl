#include "CCPACSDoorAssemblyPosition.h"

namespace tigl
{
    CCPACSDoorAssemblyPosition::CCPACSDoorAssemblyPosition(CCPACSCargoDoorsAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSDoorAssemblyPosition(parent, uidMgr), invalidated(true) {}

    void CCPACSDoorAssemblyPosition::Invalidate() {
        invalidated = true;
    }
}
