#include "CCPACSCargoCrossBeamsAssembly.h"

#include "CCPACSCrossBeamAssemblyPosition.h"

namespace tigl {
    CCPACSCargoCrossBeamsAssembly::CCPACSCargoCrossBeamsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSCargoCrossBeamsAssembly(parent, uidMgr) {}

    void CCPACSCargoCrossBeamsAssembly::Invalidate() {
        for (int i = 0; i < m_cargoCrossBeams.size(); i++) {
            m_cargoCrossBeams[i]->Invalidate();
        }
    }
}
