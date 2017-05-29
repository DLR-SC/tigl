#pragma once

#include "generated/CPACSPointAbsRel.h"
#include "CTiglPoint.h"

namespace tigl
{
    class CCPACSPointAbsRel : public generated::CPACSPointAbsRel
    {
    public:
        TIGL_EXPORT CCPACSPointAbsRel(CTiglUIDManager* uidMgr);

        TIGL_EXPORT CTiglPoint AsPoint() const; // missing coordinates default to zero
        TIGL_EXPORT void SetAsPoint(const CTiglPoint& point);
        TIGL_EXPORT ECPACSTranslationType GetRefDefaultedType() const;
    };
}
