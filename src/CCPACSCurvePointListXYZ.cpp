#include "CCPACSCurvePointListXYZ.h"

namespace tigl
{

CCPACSCurvePointListXYZ::CCPACSCurvePointListXYZ(CCPACSProfileGeometry* parent)
    : generated::CPACSCurvePointListXYZ(parent)
    , m_kinksVec(*this, &CCPACSCurvePointListXYZ::BuildKinks)
{

}

void CCPACSCurvePointListXYZ::ReadCPACS(const TixiDocumentHandle &tixiHandle, const std::string &xpath)
{
    generated::CPACSCurvePointListXYZ::ReadCPACS(tixiHandle, xpath);

    m_kinksVec.clear();

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

const std::vector<double>& CCPACSCurvePointListXYZ::GetKinksAsVector() const
{
    return m_kinksVec.value();
}

void CCPACSCurvePointListXYZ::BuildKinks(std::vector<double> &kinks) const
{
    if (GetKinks()) {
        kinks = GetKinks()->AsVector();
    }
    else {
        kinks.empty();
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
