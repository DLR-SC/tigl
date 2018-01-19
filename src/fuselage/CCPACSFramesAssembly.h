#pragma once

#include "generated/CPACSFramesAssembly.h"

namespace tigl
{
    class CCPACSFramesAssembly : public generated::CPACSFramesAssembly {
    public:
        TIGL_EXPORT CCPACSFramesAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr);
    };
}
