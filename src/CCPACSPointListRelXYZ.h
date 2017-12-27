#pragma once

#include <vector>

#include "generated/CPACSPointListRelXYZVector.h"
#include "CTiglPoint.h"

namespace tigl
{
    class CCPACSPointListRelXYZ : public generated::CPACSPointListRelXYZVector
    {
    public:
        TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) OVERRIDE;
        TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const OVERRIDE;

        TIGL_EXPORT const std::vector<CTiglPoint>& AsVector() const;
        TIGL_EXPORT std::vector<CTiglPoint>& AsVector();

    private:
        // cache
        std::vector<CTiglPoint> m_vec;
    };
}
