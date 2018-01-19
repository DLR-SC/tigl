#pragma once

#include "generated/CPACSProfileBasedStructuralElements.h"

namespace tigl
{
    class CCPACSProfileBasedStructuralElements : public generated::CPACSProfileBasedStructuralElements {
    public:
        TIGL_EXPORT CCPACSProfileBasedStructuralElements(CTiglUIDManager* uidMgr);
    };
}