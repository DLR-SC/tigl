#pragma once

#include "generated/CPACSStringerFramePosition.h"

namespace tigl
{
    class CCPACSFuselageStringerFramePosition : public generated::CPACSStringerFramePosition {
    public:
        TIGL_EXPORT CCPACSFuselageStringerFramePosition(CCPACSFrame* parent, CTiglUIDManager* uidMgr);
        TIGL_EXPORT CCPACSFuselageStringerFramePosition(CCPACSFuselageStringer* parent, CTiglUIDManager* uidMgr);

        TIGL_EXPORT void Invalidate();

    private:
        bool invalidated = true;
    };
}
