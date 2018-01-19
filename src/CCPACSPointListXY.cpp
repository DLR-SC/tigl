#include "CCPACSPointListXY.h"

#include "CTiglLogging.h"

#include "CCPACSPointXY.h"

namespace tigl {
    CCPACSPointListXY::CCPACSPointListXY(CTiglUIDManager* uidMgr)
        : generated::CPACSPointListXY(uidMgr) {}

    const CCPACSPointXY& CCPACSPointListXY::GetPoint(const std::string& uid) const {
        for (int i = 0; i < m_points.size(); i++) {
            if (m_points[i]->GetUID() == uid) {
                return *m_points[i];
            }
        }

        LOG(ERROR) << "Invalid structural profile point uid: '" << uid << "'";
        throw CTiglError("Invalid uid in CCPACSStructuralProfilePoints::GetPoint");
    }
}
