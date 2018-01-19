#include "CCPACSFuselageStringer.h"

namespace tigl
{
    CCPACSFuselageStringer::CCPACSFuselageStringer(CCPACSStringersAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSStringer(parent, uidMgr) {}

    void CCPACSFuselageStringer::Invalidate() {
        invalidated = true;
    }
}
