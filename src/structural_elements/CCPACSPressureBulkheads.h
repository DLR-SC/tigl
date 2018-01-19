#pragma once

#include "generated/CPACSPressureBulkheads.h"

namespace tigl
{
    class CCPACSPressureBulkheads : public generated::CPACSPressureBulkheads {
    public:
        TIGL_EXPORT CCPACSPressureBulkheads(CTiglUIDManager* uidMgr);
    };
}
