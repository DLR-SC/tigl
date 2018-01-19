#include "CCPACSCrossBeamStrutAssemblyPosition.h"

namespace tigl
{
    CCPACSCrossBeamStrutAssemblyPosition::CCPACSCrossBeamStrutAssemblyPosition(CCPACSCargoCrossBeamStrutsAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSCrossBeamStrutAssemblyPosition(parent, uidMgr) {}

    void CCPACSCrossBeamStrutAssemblyPosition::Invalidate() {
        invalidated = true;
    }
}
