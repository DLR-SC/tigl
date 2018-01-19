#pragma once

#include "generated/CPACSStructuralProfiles.h"

namespace tigl
{
    class CCPACSStructuralProfiles : public generated::CPACSStructuralProfiles {
    public:
        TIGL_EXPORT CCPACSStructuralProfiles(CTiglUIDManager* uidMgr);
    };
}
