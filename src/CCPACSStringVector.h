#pragma once

#include <vector>
#include "generated/CPACSStringVectorBase.h"

namespace tigl
{
    class CCPACSStringVector : private generated::CPACSStringVectorBase
    {
    public:
        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) OVERRIDE;
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const OVERRIDE;

        TIGL_EXPORT const std::vector<double>& AsVector() const;
        TIGL_EXPORT std::vector<double>& AsVector();

    private:
        // cache
        std::vector<double> m_vec;
    };
}
