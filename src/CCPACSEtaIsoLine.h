#pragma once

#include "generated/CPACSEtaIsoLine.h"

namespace tigl
{

class CCPACSEtaIsoLine : public generated::CPACSEtaIsoLine
{
public:
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSWingCellPositionSpanwise* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceAirfoil* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceBorderTrailingEdge* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceSkinCutOutBorder* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSControlSurfaceTrackType* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSCutOutProfile* parent);
    TIGL_EXPORT CCPACSEtaIsoLine(CCPACSSparCell* parent);

    TIGL_EXPORT virtual void SetEta(const double& value);
    TIGL_EXPORT virtual void SetReferenceUID(const std::string& value);

    //TIGL_EXPORT double ComputeCSOrTEDEta() const;

private:
    void InvalidateParent();
};
}
