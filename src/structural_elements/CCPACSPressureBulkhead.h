#pragma once

#include "generated/CPACSPressureBulkhead.h"

namespace tigl
{
    class CCPACSPressureBulkhead : public generated::CPACSPressureBulkhead {
    public:
        TIGL_EXPORT CCPACSPressureBulkhead(CCPACSPressureBulkheads* parent, CTiglUIDManager* uidMgr);
    };
}
