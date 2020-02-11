#ifndef CCPACSCURVEPOINTLISTXYZ_H
#define CCPACSCURVEPOINTLISTXYZ_H

#include "tigl_internal.h"
#include "generated/CPACSCurvePointListXYZ.h"
#include "Cache.h"

#include "CTiglPoint.h"

namespace  tigl
{

class CCPACSCurvePointListXYZ : public generated::CPACSCurvePointListXYZ
{
public:
    TIGL_EXPORT CCPACSCurvePointListXYZ(CCPACSProfileGeometry* parent);

    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) override;

    /// Returns the parameters of the kinks. If no kinks are defined
    /// this list is empty.
    TIGL_EXPORT const std::vector<unsigned int>& GetKinksAsVector() const;

    TIGL_EXPORT const std::vector<CTiglPoint>& AsVector() const;
    TIGL_EXPORT void SetValue(int index, const CTiglPoint& point);
    TIGL_EXPORT void SetAsVector(const std::vector<CTiglPoint>& points);

private:
    void BuildKinks(std::vector<unsigned int>& kinks) const;

    Cache<std::vector<unsigned int>, CCPACSCurvePointListXYZ> m_kinksVec;
    // cache
    std::vector<CTiglPoint> m_vec;
};

} // namespace tigl

#endif // CCPACSCURVEPOINTLISTXYZ_H
