#include "CCPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "CCPACSControlSurfaceBorderTrailingEdge.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSTrailingEdgeDevices.h"

namespace tigl
{

CCPACSControlSurfaceOuterShapeTrailingEdge::CCPACSControlSurfaceOuterShapeTrailingEdge(CCPACSTrailingEdgeDevice* parent)
    : generated::CPACSControlSurfaceOuterShapeTrailingEdge(parent)
{

}

void CCPACSControlSurfaceOuterShapeTrailingEdge::setUID(const std::string& uid)
{
    _uid = uid;
    //GetOuterBorder().SetUID(uid + "_OuterBorder");
    //GetInnerBorder().SetUID(uid + "_InnerBorder");
}



} // namespace tigl
