#pragma once

#include "generated/CPACSLongFloorBeamPosition.h"

namespace tigl
{
    class CCPACSLongFloorBeamPosition : public generated::CPACSLongFloorBeamPosition {
    public:
        TIGL_EXPORT CCPACSLongFloorBeamPosition(CCPACSLongFloorBeam* parent, CTiglUIDManager* uidMgr);

    };
}
