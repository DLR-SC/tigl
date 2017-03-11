#pragma once

#include "generated/CPACSPoint.h"
#include "CTiglPoint.h"

namespace tigl
{
    class CCPACSPoint : public generated::CPACSPoint
    {
    public:
        TIGL_EXPORT CTiglPoint AsPoint() const;
        TIGL_EXPORT void SetAsPoint(const CTiglPoint& point);
    };
}
