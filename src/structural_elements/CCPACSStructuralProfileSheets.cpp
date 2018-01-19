#include "CCPACSStructuralProfileSheets.h"

namespace tigl
{
    CCPACSStructuralProfileSheets::CCPACSStructuralProfileSheets(CCPACSStructuralProfile* parent, CTiglUIDManager* uidMgr)
        : generated::CPACSSheetList(parent, uidMgr) {}
}
