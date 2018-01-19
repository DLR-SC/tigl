#include "CCPACSFuselageStructure.h"

namespace tigl
{
    CCPACSFuselageStructure::CCPACSFuselageStructure(CCPACSFuselage* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSFuselageStructure(parent, uidMgr) {}

    void CCPACSFuselageStructure::Invalidate() {
        if (m_stringers)
            m_stringers->Invalidate();
        if (m_pressureBulkheads)
            m_pressureBulkheads->Invalidate();
        if (m_cargoCrossBeamStruts)
            m_cargoCrossBeamStruts->Invalidate();
        if (m_longFloorBeams)
            m_longFloorBeams->Invalidate();
    }
}
