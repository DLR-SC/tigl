#pragma once

#include "generated/CPACSPressureBulkheadAssembly.h"

namespace tigl
{
    class CCPACSPressureBulkheadAssembly : public generated::CPACSPressureBulkheadAssembly {
    public:
        TIGL_EXPORT CCPACSPressureBulkheadAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();
    };
}
