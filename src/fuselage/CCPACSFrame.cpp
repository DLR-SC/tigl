#include "CCPACSFrame.h"

namespace tigl {
    CCPACSFrame::CCPACSFrame(CCPACSFramesAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSFrame(parent, uidMgr) {}

    void CCPACSFrame::Invalidate()
    {
        invalidated = true;
    }
}
