#pragma once

#include "generated/CPACSFrame.h"

namespace tigl {
    class CCPACSFrame : public generated::CPACSFrame {
    public:
        TIGL_EXPORT CCPACSFrame(CCPACSFramesAssembly* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
