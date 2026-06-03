#include "DrawOptionsActions.h"
#include <QDebug>

// Forward declaration for example
void onColorChosen();


// WING actions
const std::vector<DrawOptionAction>& getWingDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Show Wing", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawWing(uid);} },
        { "Show Fused wing", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawFusedWing(uid);} },
        { "Show Wing Shells", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingShells(uid); } },
        { "Show Wing Component segment", [](TIGLCreatorDocument* doc, const QString& uid){  doc->drawWingComponentSegment(uid); } },
        { "Show Wing Structure", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingStructure(uid);} },
        { "Show Wing triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingTriangulation(uid); } },
        { "Show Wing Profiles", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingProfiles(); } },
        { "Show Wing Guide curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingGuideCurves(uid);} },
        { "Show Wing Flaps", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingFlaps(uid);} },
        { "Show Wing Sample points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingSamplePoints(uid);} },
        { "Show Wing Overlay profile points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingOverlayProfilePoints(uid); } },
        { "Show Wing Component segment points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawWingComponentSegmentPoints(uid);} }
    };
    return actions;
}

// FUSELAGE actions
const std::vector<DrawOptionAction>& getFuselageDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Show Fuselage", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselage(uid);} },
        { "Show Fuselage Profiles", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageProfiles(); } },
        { "Show Fuselage Guide Curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageGuideCurves(uid); } },
        { "Show Fuselage triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageTriangulation(uid); } },
        { "Show Fuselage Sample points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageSamplePoints(uid); } },
        { "Show Fuselage Sample points at angle", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFuselageSamplePointsAngle(uid); } },
        { "Show Fused fuselage", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedFuselage(uid); } }
    };
    return actions;
}

// PLANE actions
const std::vector<DrawOptionAction>& getPlaneDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Show the complete aircraft", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfiguration(); } },
        { "Show the complete aircraft with duct cutouts", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawConfigurationWithDuctCutouts(); } },
        { "Show he complete aircraft fused (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraft(); } },
        { "Show fused aircraft triangulation (slow)", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedAircraftTriangulation(); } },
        { "Show intersection line", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawIntersectionLine(); } },
        { "Draw Far Field", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFarField(); } },
        { "Draw Systems", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawSystems();} },
        { "Draw any Component", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawComponent(); } },
        { "Draw Control Point Net", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawControlPointNet(); } }
    };
    return actions;
}

// ROTORBLADE actions
const std::vector<DrawOptionAction>& getRotorBladeDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Show Rotor Blade", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBlade(uid); } },
        { "Show Rotor Blade Guide curves", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeGuideCurves(); } },
        { "Show Rotor Blade overlay profile points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeOverlayProfilePoints(uid); } },
        { "Show Rotor Blade Triangulation", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeTriangulation(uid); } },
        { "Show Sample Rotor Blade points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeSamplePoints(uid); } },
        { "Show Fused Rotor Blade", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawFusedRotorBlade(uid); } },
        { "Show Rotor Blade Component segment", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeComponentSegment(uid); } },
        { "Show Rotor Blade Component segment points", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeComponentSegmentPoints(uid); } },
        { "Show Rotor Blade Shells", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorBladeShells(uid); } },
        { "Show all Rotors, Wings and Fuselages", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawAllFuselagesAndWingsSurfacePoints(); } }
    };
    return actions;
}

// ROTOR actions
const std::vector<DrawOptionAction>& getRotorDrawOptionsActions() {
    static std::vector<DrawOptionAction> actions = {
        { "Show Rotor", [](TIGLCreatorDocument* doc, const QString& uid){ doc->drawRotorByUID(uid);} },
        { "Show Rotor Disk", [](TIGLCreatorDocument* doc, const QString& uid){doc->drawRotorDisk(uid);} },
        { "Show Rotor Properties", [](TIGLCreatorDocument* doc, const QString& uid){ doc->showRotorProperties(uid);} }
    };
    return actions;
}
