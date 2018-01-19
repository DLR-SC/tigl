#include "CCPACSCrossBeamAssemblyPosition.h"

namespace tigl
{
    CCPACSCrossBeamAssemblyPosition::CCPACSCrossBeamAssemblyPosition(CCPACSCargoCrossBeamsAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSCrossBeamAssemblyPosition(parent, uidMgr) {}

    void CCPACSCrossBeamAssemblyPosition::Invalidate() {
        invalidated = true;
    }
}
