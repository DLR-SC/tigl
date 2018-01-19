#pragma once

#include "generated/CPACSSheetBasedStructuralElements.h"

namespace tigl
{
    class CCPACSSheetBasedStructuralElements : public generated::CPACSSheetBasedStructuralElements {
    public:
        TIGL_EXPORT CCPACSSheetBasedStructuralElements(CTiglUIDManager* uidMgr);
    };
}
