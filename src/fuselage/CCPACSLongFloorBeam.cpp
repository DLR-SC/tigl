#include "CCPACSLongFloorBeam.h"

namespace tigl
{
    CCPACSLongFloorBeam::CCPACSLongFloorBeam(CCPACSLongFloorBeamsAssembly* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSLongFloorBeam(parent, uidMgr) {}

    void CCPACSLongFloorBeam::Invalidate()
    {
        invalidated = true;
    }
}
