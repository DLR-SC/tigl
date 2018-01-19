#include "CCPACSPressureBulkheadAssembly.h"

#include "CCPACSPressureBulkheadAssemblyPosition.h"

namespace tigl
{
    CCPACSPressureBulkheadAssembly::CCPACSPressureBulkheadAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
     : generated::CPACSPressureBulkheadAssembly(parent, uidMgr) {}

    void CCPACSPressureBulkheadAssembly::Invalidate() {
        for (int i = 0; i < m_pressureBulkheads.size(); i++) {
            m_pressureBulkheads[i]->Invalidate();
        }
    }
}
