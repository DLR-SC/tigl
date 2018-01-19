#include "CCPACSFuselageStringerFramePosition.h"

namespace tigl{
    CCPACSFuselageStringerFramePosition::CCPACSFuselageStringerFramePosition(CCPACSFrame* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSStringerFramePosition(parent, uidMgr) {}

    CCPACSFuselageStringerFramePosition::CCPACSFuselageStringerFramePosition(CCPACSFuselageStringer* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSStringerFramePosition(parent, uidMgr) {}

    void CCPACSFuselageStringerFramePosition::Invalidate()
    {
        invalidated = true;
    }
}
