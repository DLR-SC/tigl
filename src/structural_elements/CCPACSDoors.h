#pragma once

#include "generated/CPACSDoors.h"

namespace tigl
{
    class CCPACSDoors : public generated::CPACSDoors {
    public:
        TIGL_EXPORT CCPACSDoors(CTiglUIDManager* uidMgr);
    };
}
