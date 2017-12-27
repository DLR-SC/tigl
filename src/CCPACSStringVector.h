#pragma once

#include <vector>
#include "generated/CPACSStringVectorBase.h"

namespace tigl
{
    TIGL_EXPORT std::vector<double> stringToDoubleVec(const std::string& s);
    TIGL_EXPORT std::string doubleVecToString(const std::vector<double>& v);

    class CCPACSStringVector : private generated::CPACSStringVectorBase
    {
    public:
        TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string &xpath) OVERRIDE;
        TIGL_EXPORT void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const OVERRIDE;

        using generated::CPACSStringVectorBase::GetMapType;
        using generated::CPACSStringVectorBase::SetMapType;

        TIGL_EXPORT const std::vector<double>& AsVector() const;
        TIGL_EXPORT std::vector<double>& AsVector();

    private:
        // cache
        std::vector<double> m_vec;
    };
}
