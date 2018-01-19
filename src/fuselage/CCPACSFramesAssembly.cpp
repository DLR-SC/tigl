#include "CCPACSFramesAssembly.h"

namespace tigl
{
    CCPACSFramesAssembly::CCPACSFramesAssembly(CCPACSFuselageStructure* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSFramesAssembly(parent, uidMgr) {}
}
