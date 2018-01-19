#include "CCPACSLongFloorBeamsAssembly.h"

#include "CCPACSLongFloorBeam.h"

namespace tigl
{
    CCPACSLongFloorBeamsAssembly::CCPACSLongFloorBeamsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSLongFloorBeamsAssembly(parent, uidMgr) {}

    void CCPACSLongFloorBeamsAssembly::Invalidate() {
        for (int i = 0; i < m_longFloorBeams.size(); i++) {
            m_longFloorBeams[i]->Invalidate();
        }
    }
}
