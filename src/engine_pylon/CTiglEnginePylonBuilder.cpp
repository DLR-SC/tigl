#include "CTiglEnginePylonBuilder.h"

#include "CCPACSWingSegments.h"
#include "CCPACSWingSegment.h"
#include "CCPACSWingProfile.h"
#include "CTiglMakeLoft.h"
#include "CNamedShape.h"

namespace tigl
{


PNamedShape CTiglEnginePylonBuilder::BuildShape()
{
    const boost::optional<CCPACSWingSegments>& segments = m_pylon.GetSegments();

    if (!segments.is_initialized()) {
        throw CTiglError("No pylon geometry defined for pylon \"" + m_pylon.GetDefaultedUID() + "\".");
    }

    const CCPACSWingProfile& innerProfile = segments->GetSegment(1).GetInnerConnection().GetProfile();

    // we assume, that all profiles of one wing are either blunt or not
    // this is checked during cpacs loading of each wing segment
    bool hasBluntTE = innerProfile.HasBluntTE();

    CTiglMakeLoft lofter;
    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(true);

    for (int i=1; i <= segments->GetSegmentCount(); i++) {
        const TopoDS_Shape& startWire = segments->GetSegment(i).GetInnerWire();
        lofter.addProfiles(startWire);
    }

    TopoDS_Wire endWire = segments->GetSegment(segments->GetSegmentCount()).GetOuterWire();
    lofter.addProfiles(endWire);

    // TODO: add guide curves
    // lofter.addGuides(m_pylon.GetGuideCurveWires());

    TopoDS_Shape loftShape = lofter.Shape();
    std::string loftName = m_pylon.GetUID();
    PNamedShape loft(new CNamedShape(loftShape, loftName.c_str()));
    return loft;
}

CTiglEnginePylonBuilder::operator PNamedShape()
{
    return BuildShape();
}

}  // namespace tigl
