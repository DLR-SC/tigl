#pragma once

#include <vector>

#include "generated/CPACSPointListXYZVector.h"
#include "CTiglPoint.h"

namespace tigl
{
    class CCPACSPointListXYZ : public generated::CPACSPointListXYZVector
    {
    public:
        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const override;

        TIGL_EXPORT const std::vector<CTiglPoint>& AsVector() const;
        TIGL_EXPORT std::vector<CTiglPoint>& AsVector();

    private:
        // cache
        std::vector<CTiglPoint> m_vec;
    };
}
