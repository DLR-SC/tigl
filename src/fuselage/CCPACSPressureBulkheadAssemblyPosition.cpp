#include "CCPACSPressureBulkheadAssemblyPosition.h"

namespace tigl {
    CCPACSPressureBulkheadAssemblyPosition::CCPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSPressureBulkheadAssemblyPosition(parent, uidMgr) {}

    void CCPACSPressureBulkheadAssemblyPosition::Invalidate() {
        invalidated = true;
    }
}
