#pragma once

#include "generated/CPACSPointAbsRel.h"
#include "CTiglPoint.h"

namespace tigl
{
    class CCPACSPointAbsRel : public generated::CPACSPointAbsRel
    {
    public:
        TIGL_EXPORT CTiglPoint AsPoint() const;
        TIGL_EXPORT void SetAsPoint(const CTiglPoint& point);
        TIGL_EXPORT ECPACSTranslationType GetRefDefaultedType() const;
    };
}
