#include "CCPACSCurvePointListXYZ.h"

#include "tiglcommonfunctions.h"

#include <iterator>

namespace tigl
{

CCPACSCurvePointListXYZ::CCPACSCurvePointListXYZ(CCPACSProfileGeometry* parent)
    : generated::CPACSCurvePointListXYZ(parent)
    , m_cache(*this, &CCPACSCurvePointListXYZ::BuildCache)
{

}

void CCPACSCurvePointListXYZ::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{
    generated::CPACSCurvePointListXYZ::ReadCPACS(tixiHandle, xpath);

    m_cache.clear();

    // create cached representation from CPACS fields
    const std::vector<double>& xs = m_x.AsVector();
    const std::vector<double>& ys = m_y.AsVector();
    const std::vector<double>& zs = m_z.AsVector();
    if (xs.size() != ys.size() || ys.size() != zs.size()) {
        throw CTiglError("component vectors in CCPACSPointListXYZ must all have the same number of elements");
    }
    m_vec.clear();
    for (std::size_t i = 0; i < xs.size(); i++) {
        m_vec.push_back(CTiglPoint(xs[i], ys[i], zs[i]));
    }
}

const std::vector<unsigned int>& CCPACSCurvePointListXYZ::GetKinksAsVector() const
{
    return m_cache->kinks;
}

const ParamMap& CCPACSCurvePointListXYZ::GetParamsAsMap() const
{
    return m_cache->paramMap;
}

void CCPACSCurvePointListXYZ::BuildCache(CachedObjects& cache) const
{
    cache.kinks.clear();
    if (GetKinks()) {
        auto kinkAsDouble = GetKinks()->AsVector();

        auto& kinks = cache.kinks;
        kinks.clear();
        std::transform(std::begin(kinkAsDouble), std::end(kinkAsDouble), std::back_inserter(kinks), [](double v) {
            return static_cast<unsigned int>(v);
        });
        std::sort(std::begin(kinks), std::end(kinks));
    }

    cache.paramMap.clear();
    if (GetParameterMap()) {
        const auto& cpacsMap = GetParameterMap().value();
        auto params = cpacsMap.GetParamAsVector();
        auto idx = cpacsMap.GetPointIndexAsVector();

        if (idx.size() != params.size()) {
            throw CTiglError("Number of parameters does not match number of indices");
        }

        for (size_t i = 0; i < params.size(); ++i) {
            auto key = idx[i];
            auto value = params[i];
            cache.paramMap[key] = value;
        }
    }

}

const std::vector<CTiglPoint>& CCPACSCurvePointListXYZ::AsVector() const
{
    return m_vec;
}

void CCPACSCurvePointListXYZ::SetValue(int index, const CTiglPoint& point)
{
    m_x.SetValue(index, point.x);
    m_y.SetValue(index, point.y);
    m_z.SetValue(index, point.z);
    // no invalidation necessary, done by m_x, m_y, m_z
    m_vec.at(index) = point;
}

void CCPACSCurvePointListXYZ::SetAsVector(const std::vector<CTiglPoint>& points)
{
    m_vec = points;
    std::vector<double> x, y, z;

    for (std::vector<CTiglPoint>::const_iterator it = m_vec.begin(); it != m_vec.end(); ++it) {
        x.push_back(it->x);
        y.push_back(it->y);
        z.push_back(it->z);
    }
    m_x.SetAsVector(x);
    m_y.SetAsVector(y);
    m_z.SetAsVector(z);
    // no invalidation necessary, done by m_x, m_y, m_z
}

} // namespace tigl
