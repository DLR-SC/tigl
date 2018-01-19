#include "CCPACSStringersAssembly.h"

#include "CCPACSFuselageStringer.h"

namespace tigl {
    CCPACSStringersAssembly::CCPACSStringersAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSStringersAssembly(parent, uidMgr) {}

    void CCPACSStringersAssembly::Invalidate() {
        for (int i = 0; i < m_stringers.size(); i++) {
            m_stringers[i]->Invalidate();
        }
    }
}
