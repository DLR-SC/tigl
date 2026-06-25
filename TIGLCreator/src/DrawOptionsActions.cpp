#include "DrawOptionsActions.h"

#include "CCPACSWing.h"
#include "CCPACSWingComponentSegment.h"
#include "CTiglError.h"

#include <QDebug>


void onColorChosen();

namespace
{

bool hasControlSurfaces(TIGLCreatorDocument* doc, const QString& uid)
{
    if (!doc || doc->getCpacsHandle() <= 0) {
        return false;
    }

    try {
        auto& config = doc->GetConfiguration();
        tigl::CTiglUIDManager& uidMgr = config.GetUIDManager();
        // As the uid can also be an aircraft uid we need to check if the uid is anything else than a wing. 
        // The fucntion is designed to work for wings and aircrafts and therefore has to check both cases.
        if (uid.isEmpty() || !uidMgr.IsType<tigl::CCPACSWing>(uid.toStdString())) {
            for (int wingIndex = 1; wingIndex <= config.GetWingCount(); ++wingIndex) {
                tigl::CCPACSWing& wing = config.GetWing(wingIndex);
                for (int segmentIndex = 1; segmentIndex <= wing.GetComponentSegmentCount(); ++segmentIndex) {
                    tigl::CCPACSWingComponentSegment& componentSegment = wing.GetComponentSegment(segmentIndex);
                    const auto& controlSurfaces = componentSegment.GetControlSurfaces();
                    if (controlSurfaces && controlSurfaces->ControlSurfaceCount() > 0) {
                        return true;
                    }
                }
            }
        }
        else {
            tigl::CCPACSWing& wing = config.GetWing(uid.toStdString());
            for (int segmentIndex = 1; segmentIndex <= wing.GetComponentSegmentCount(); ++segmentIndex) {
                tigl::CCPACSWingComponentSegment& componentSegment = wing.GetComponentSegment(segmentIndex);
                const auto& controlSurfaces = componentSegment.GetControlSurfaces();
                if (controlSurfaces && controlSurfaces->ControlSurfaceCount() > 0) {
                    return true;
                }
            }
        }
    }
    catch (const tigl::CTiglError&) {
        return false;
    }

    return false;
}

bool hasStructure(TIGLCreatorDocument* doc, const QString& uid)
{
    if (!doc || doc->getCpacsHandle() <= 0) {
        return false;
    }
     try {
        auto& config = doc->GetConfiguration();
        tigl::CTiglUIDManager& uidMgr = config.GetUIDManager();
        if (uid.isEmpty()) {
            for (int wingIndex = 1; wingIndex <= config.GetWingCount(); ++wingIndex) {
                tigl::CCPACSWing& wing = config.GetWing(wingIndex);
                for (int segmentIndex = 1; segmentIndex <= wing.GetComponentSegmentCount(); ++segmentIndex) {
                    tigl::CCPACSWingComponentSegment& segment = wing.GetComponentSegment(segmentIndex);
                    tigl::CCPACSWingComponentSegment* cs = static_cast<tigl::CCPACSWingComponentSegment*>(&segment);
                    if (cs && cs->GetStructure()) {
                        return true;
                    }
                }
            }
        }
        else {
            tigl::CCPACSWing& wing = doc->GetConfiguration().GetWing(uid.toStdString());
            for (int segmentIndex = 1; segmentIndex <= wing.GetComponentSegmentCount(); ++segmentIndex) {
                tigl::CCPACSWingComponentSegment& segment = wing.GetComponentSegment(segmentIndex);
                    tigl::CCPACSWingComponentSegment* cs = static_cast<tigl::CCPACSWingComponentSegment*>(&segment);
                    if (cs && cs->GetStructure()) {
                        return true;
                    }
                }
        }
    }
      catch (const tigl::CTiglError&) {
        return false;
    }


    return false;
}

}
// WING actions
const std::vector<DrawOptionAction>& getWingDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Wing", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawWing(uid);} },
        { "Fused wing", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawFusedWing(uid);} },
        { "Wing Shells", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingShells(uid); } },
        { "Wing Component segment", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawWingComponentSegment(uid); } },
        { "Wing Structure", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingStructure(uid);} ,[](TIGLCreatorDocument* doc, const QString& uid) {
            return hasStructure(doc, uid); }},
        { "Wing triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingTriangulation(uid); } },
        { "Wing Profiles", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingProfiles(); } },
        { "Wing Guide curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingGuideCurves(uid);} },
        { "Wing Flaps", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingFlaps(uid);},[](TIGLCreatorDocument* doc, const QString& uid) {
            return hasControlSurfaces(doc, uid); }},
        { "Wing Sample points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingSamplePoints(uid);} },
        { "Wing Overlay profile points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingOverlayProfilePoints(uid); } },
        { "Wing Component segment points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingComponentSegmentPoints(uid);} }
    };
    return actions;
}

// FUSELAGE actions
const std::vector<DrawOptionAction>& getFuselageDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Fuselage", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselage(uid);} },
        { "Fuselage Profiles", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageProfiles(); } },
        { "Fuselage Guide Curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageGuideCurves(uid); } },
        { "Fuselage triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageTriangulation(uid); } },
        { "Fuselage Sample points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageSamplePoints(uid); } },
        { "Fuselage Sample points at angle", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageSamplePointsAngle(uid); } },
        { "Fused fuselage", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedFuselage(uid); } }
    };
    return actions;
}

// PLANE actions
const std::vector<DrawOptionAction>& getPlaneDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Complete aircraft", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfiguration(); } },
        { "Complete aircraft with duct cutouts", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfigurationWithDuctCutouts(); } },
        { "Complete aircraft fused (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraft(); } },
        { "Fused aircraft triangulation (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraftTriangulation(); } },
        { "Intersection line", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawIntersectionLine(); } },
        { "Far Field", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFarField(); }, [](TIGLCreatorDocument* doc, const QString&) {
            return doc && doc->getCpacsHandle() > 0 && doc->GetConfiguration().GetFarField().GetType() != tigl::NONE;}},
        { "Systems", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawSystems();}, [](TIGLCreatorDocument* doc, const QString&) {
            return doc && doc->getCpacsHandle() > 0 && doc->GetConfiguration().GetGenericSystemCount() > 0;}},
        { "Draw any Component", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawComponent(); } },
        { "Control Point Net", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawControlPointNet(); } },
        { "Wing Flaps (Overlay)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFlapsOverlay();}, [](TIGLCreatorDocument* doc, const QString& uid) {
            return hasControlSurfaces(doc, uid); }},

    };
    return actions;
}

const std::vector<DrawOptionAction>& getPlaneDisplayOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Complete aircraft", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfiguration(); } },
        { "Complete aircraft with duct cutouts", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfigurationWithDuctCutouts(); } },
        { "Complete aircraft fused (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraft(); } },
        { "Fused aircraft triangulation (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraftTriangulation(); } },
        { "Systems", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawSystems();}, [](TIGLCreatorDocument* doc, const QString&) {
            return doc && doc->getCpacsHandle() > 0 && doc->GetConfiguration().GetGenericSystemCount() > 0;}},
        { "Wing Flaps (Overlay)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFlapsOverlay();}, [](TIGLCreatorDocument* doc, const QString& uid) {
            return hasControlSurfaces(doc, uid); }},
    };
    return actions;
}

// ROTORBLADE actions
const std::vector<DrawOptionAction>& getRotorBladeDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Rotor Blade", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBlade(uid); } },
        { "Rotor Blade Guide curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeGuideCurves(); } },
        { "Rotor Blade overlay profile points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeOverlayProfilePoints(uid); } },
        { "Rotor Blade Triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeTriangulation(uid); } },
        { "Sample Rotor Blade points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeSamplePoints(uid); } },
        { "Fused Rotor Blade", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedRotorBlade(uid); } },
        { "Rotor Blade Component segment", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeComponentSegment(uid); } },
        { "Rotor Blade Component segment points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeComponentSegmentPoints(uid); } },
        { "Rotor Blade Shells", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeShells(uid); } },
        { "All Rotors, Wings and Fuselages", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawAllFuselagesAndWingsSurfacePoints(); } }
    };
    return actions;
}

// ROTOR actions
const std::vector<DrawOptionAction>& getRotorDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Rotor", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorByUID(uid);} },
        { "Rotor Disk", [](TIGLCreatorDocument* doc, const QString& uid){doc->drawRotorDisk(uid);} },
        { "Rotor Properties", [](TIGLCreatorDocument* doc, const QString& uid){ doc->showRotorProperties(uid);} }
    };
    return actions;
}
