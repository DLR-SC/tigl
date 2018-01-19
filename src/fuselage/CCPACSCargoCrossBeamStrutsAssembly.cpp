#include "CCPACSCargoCrossBeamStrutsAssembly.h"

#include "CTiglLogging.h"
#include "CTiglError.h"
#include "CCPACSCrossBeamStrutAssemblyPosition.h"

namespace tigl
{
    CCPACSCargoCrossBeamStrutsAssembly::CCPACSCargoCrossBeamStrutsAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSCargoCrossBeamStrutsAssembly(parent, uidMgr) {}

    void CCPACSCargoCrossBeamStrutsAssembly::Invalidate() {
        for (int i = 0; i < m_cargoCrossBeamStruts.size(); i++) {
            m_cargoCrossBeamStruts[i]->Invalidate();
        }
    }
}
