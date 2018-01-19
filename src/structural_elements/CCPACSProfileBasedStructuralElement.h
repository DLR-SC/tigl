#pragma once

#include "generated/CPACSProfileBasedStructuralElement.h"

namespace tigl
{
    class CCPACSProfileBasedStructuralElement : public generated::CPACSProfileBasedStructuralElement {
    public:
        TIGL_EXPORT CCPACSProfileBasedStructuralElement(CTiglUIDManager* uidMgr);
    };
}