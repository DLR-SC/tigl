#pragma once

#include "generated/CPACSSheetList.h"

namespace tigl
{
    class CCPACSStructuralProfileSheets : public generated::CPACSSheetList {
    public:
        TIGL_EXPORT CCPACSStructuralProfileSheets(CCPACSStructuralProfile* parent, CTiglUIDManager* uidMgr);
    };
}
