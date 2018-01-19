#pragma once

#include "generated/CPACSPressureBulkheadAssemblyPosition.h"

namespace tigl
{
    class CCPACSPressureBulkheadAssemblyPosition : public generated::CPACSPressureBulkheadAssemblyPosition {
    public:
        TIGL_EXPORT CCPACSPressureBulkheadAssemblyPosition(CCPACSPressureBulkheadAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
