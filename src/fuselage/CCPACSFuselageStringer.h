#pragma once

#include "generated/CPACSStringer.h"

namespace tigl
{
    class CCPACSFuselageStringer : public generated::CPACSStringer {
    public:
        TIGL_EXPORT CCPACSFuselageStringer(CCPACSStringersAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
