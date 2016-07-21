#include "CCPACSPoint.h"

namespace tigl {
	CTiglPoint CCPACSPoint::AsPoint() const {
		CTiglPoint point;
		// TODO: in case m_x, m_y ot m_z is not valid, we can set the corresponding coord to zero, thus AsPoint() never throws
		point.x = m_x.get();
		point.y = m_y.get();
		point.z = m_z.get();
		return point;
	}

	void CCPACSPoint::SetAsPoint(const CTiglPoint& point) {
		m_x = point.x;
		m_y = point.y;
		m_z = point.z;
	}
}
