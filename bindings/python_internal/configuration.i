/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-11-22 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

%module (package="tigl3") configuration

%include <typemaps.i>
%include <factory.i>
%include common.i
%include math_headers.i

%import geometry.i
%import core.i

%{
// All includes that are required for the wrapper compilation
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CTiglFusePlane.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselageSection.h"
#include "CCPACSExternalObject.h"
#include "CTiglShapeCache.h"
#include "CTiglError.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselageSegment.h"
#include "CTiglWingConnection.h"
#include "CTiglFuselageConnection.h"
#include "CCPACSWingCell.h"
#include "CCPACSMaterialDefinition.h"
#include "CCPACSGenericSystem.h"
#include "CCPACSRotorBladeAttachment.h"
#include "generated/CPACSWingElement.h"
#include "CCPACSWingSectionElement.h"
#include "CCPACSWingSection.h"
#include "generated/CPACSGuideCurveProfileGeometry.h"
#include "CCPACSGuideCurveProfile.h"
#include "generated/CPACSSparSegment.h"
#include "CCPACSWingSparSegment.h"
#include "CCPACSWingRibsDefinition.h"
#include "CCPACSSkinSegment.h"
#include "CTiglWingChordface.h"
#include "CCPACSCrossBeamAssemblyPosition.h"
#include "CCPACSCrossBeamStrutAssemblyPosition.h"
#include "generated/CPACSCrossBeamStrutAssemblyPosition.h"
#include "CCPACSDoorAssemblyPosition.h"
#include "CCPACSLongFloorBeam.h"
#include "CCPACSLongFloorBeamPosition.h"
#include "CCPACSPressureBulkheadAssemblyPosition.h"
#include <TopoDS_TEdge.hxx>
#include <TopoDS_HShape.hxx>
#include <TopLoc_IndexedMapNodeOfIndexedMapOfLocation.hxx>
#include <TopLoc_StdMapNodeOfMapOfLocation.hxx>
#include "generated/CPACSControlSurfaceOuterShapeTrailingEdge.h"
#include "generated/CPACSControlSurfacePath.h"
#include "generated/CPACSCutOutControlPoints.h"
#include "generated/CPACSControlSurfaceWingCutOut.h"
#include "generated/CPACSControlSurfaceSkinCutOutBorder.h"
#include "generated/CPACSControlSurfaceTracks.h"
#include "CCPACSControlSurfaceTrackType.h"
#include "generated/CPACSTrailingEdgeDevice.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSEnginePylons.h"
#include "CCPACSEnginePylon.h"
%}

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

// wrap optional classes
%boost_optional(tigl::generated::CPACSRibRotation_ribRotationReference)
%boost_optional(tigl::CCPACSWingCSStructure)
%boost_optional(tigl::generated::CPACSSkinSegments)
%boost_optional(tigl::generated::CPACSSkin)
%boost_optional(tigl::CCPACSStringersAssembly)
%boost_optional(tigl::CCPACSFramesAssembly)
%boost_optional(tigl::CCPACSCargoCrossBeamsAssembly)
%boost_optional(tigl::CCPACSCargoCrossBeamStrutsAssembly)
%boost_optional(tigl::CCPACSLongFloorBeamsAssembly)
%boost_optional(tigl::CCPACSPressureBulkheadAssembly)
%boost_optional(tigl::generated::CPACSCargoDoorsAssembly)
%boost_optional(tigl::ECPACSRibRotation_ribRotationReference)
%boost_optional(tigl::generated::CPACSWingRibCell)
%boost_optional(tigl::generated::CPACSCap)
%boost_optional(tigl::CCPACSWingRibsPositioning)
%boost_optional(tigl::generated::CPACSWingRibExplicitPositioning)
%boost_optional(tigl::generated::CPACSWeb)
%boost_optional(tigl::generated::CPACSSparCells)
%boost_optional(tigl::CCPACSGuideCurves)
%boost_optional(tigl::CCPACSPositionings)
%boost_optional(tigl::CCPACSWingComponentSegments)
%boost_optional(tigl::CCPACSWingRibsDefinitions)
%boost_optional(tigl::CCPACSWingSpars)
%boost_optional(tigl::generated::CPACSGuideCurve_continuity)
%boost_optional(tigl::CCPACSWingProfileCST)
%boost_optional(tigl::CTiglTransformation)
%boost_optional(tigl::CCPACSRotorHinges)
%boost_optional(tigl::generated::CPACSRotorHub)
%boost_optional(tigl::TiglRotorHubType)
%boost_optional(tigl::generated::CPACSRotor_type)
%boost_optional(tigl::CCPACSFuselageStructure)
%boost_optional(tigl::generated::CPACSLinkToFileType_format)
%boost_optional(tigl::CCPACSWingCells)
%boost_optional(tigl::generated::CPACSAlignmentCrossBeam)
%boost_optional(tigl::generated::CPACSDoorAssemblyPosition_doorType)
%boost_optional(tigl::CCPACSWingSections)
%boost_optional(tigl::CCPACSWingSegments)
%boost_optional(tigl::CCPACSPositionings)


// ---------------- Other ------------------------------//
%boost_optional(tigl::generated::CPACSEtaIsoLine)
%boost_optional(tigl::CCPACSMaterialDefinition)
%include "generated/CPACSXsiIsoLine.h"
%include "generated/CPACSEtaIsoLine.h"

%include "generated/CPACSMaterialDefinition.h"
%include "CCPACSMaterialDefinition.h"

// ---------------- Control surfaces ------------------ //
%boost_optional(tigl::generated::CPACSControlSurfaceTrackType_trackSubType)
%boost_optional(tigl::generated::CPACSTrackFairing)
%boost_optional(tigl::generated::CPACSTrackStrut2)
%boost_optional(tigl::generated::CPACSTrackStrut1)
%boost_optional(tigl::generated::CPACSTrackCar)
%boost_optional(tigl::generated::CPACSTrackStructure)
%boost_optional(tigl::generated::CPACSTrackActuator)
%boost_optional(tigl::generated::CPACSControlSurfaceWingCutOut)
%boost_optional(tigl::generated::CPACSControlSurfaceContours)
%boost_optional(tigl::generated::CPACSCutOutProfiles)
%boost_optional(tigl::generated::CPACSContourReference)
%boost_optional(tigl::generated::CPACSLeadingEdgeShape)
%boost_optional(tigl::generated::CPACSLeadingEdgeHollow)
%boost_optional(tigl::generated::CPACSCutOutControlPoints)
%boost_optional(tigl::CCPACSControlSurfaceSkinCutOutBorder)
%boost_optional(tigl::CCPACSTrailingEdgeDevices)
%boost_optional(tigl::CCPACSControlSurfaces)
%boost_optional(tigl::CPACSControlSurfaceWingCutOut)
%boost_optional(tigl::generated::CPACSControlSurfaceTracks)
%boost_optional(tigl::generated::CPACSCutOutControlPoints)

namespace tigl
{
    class CCPACSWingCSStructure;
}
%include "generated/CPACSTrackFairing.h"
%include "generated/CPACSTrackStrut2.h"
%include "generated/CPACSTrackStrut1.h"
%include "generated/CPACSTrackCar.h"
%include "generated/CPACSTrackStructure.h"
%include "generated/CPACSTrackActuator.h"
%include "generated/CPACSControlSurfaceTrackType_trackSubType.h"
%include "generated/CPACSControlSurfaceTrackType_trackType.h"
%include "generated/CPACSControlSurfaceTrackType.h"
%include "CCPACSControlSurfaceTrackType.h"
%include "generated/CPACSOuterCutOutProfile.h"
%include "generated/CPACSCutOutProfiles.h"
%include "generated/CPACSControlSurfaceSkinCutOut.h"
%include "CCPACSControlSurfaceSkinCutout.h"
%include "generated/CPACSControlSurfaceTracks.h"
%include "generated/CPACSControlSurfaceContours.h"
%include "generated/CPACSContourReference.h"
%include "generated/CPACSLeadingEdgeShape.h"
%include "generated/CPACSLeadingEdgeHollow.h"
%include "generated/CPACSCutOutControlPoint.h"
%include "generated/CPACSCutOutControlPoints.h"
%include "generated/CPACSControlSurfaceSteps.h"
%include "CCPACSControlSurfaceSteps.h"
%include "generated/CPACSControlSurfaceHingePoint.h"
%include "CCPACSControlSurfaceHingePoint.h"
%include "generated/CPACSControlSurfaceSkinCutOutBorder.h"
%include "CCPACSControlSurfaceSkinCutOutBorder.h"
%include "generated/CPACSCutOutControlPoints.h"
%include "generated/CPACSControlSurfacePath.h"
%include "generated/CPACSControlSurfaceBorderTrailingEdge.h"
%include "CCPACSControlSurfaceBorderTrailingEdge.h"
%include "generated/CPACSControlSurfaceOuterShapeTrailingEdge.h"
%include "generated/CPACSControlSurfacePath.h"
%include "generated/CPACSControlSurfaceTracks.h"
%include "CPACSControlSurfaceWingCutOut.h"
%include "generated/CPACSTrailingEdgeDevice.h"
%include "CCPACSTrailingEdgeDevice.h"
%include "generated/CPACSTrailingEdgeDevices.h"
%include "CCPACSTrailingEdgeDevices.h"
%include "generated/CPACSControlSurfaces.h"
%include "CCPACSControlSurfaces.h"

// --------------- Fuselage structure -----------------//

%apply double *OUTPUT { double* eta, double* xsi };
%ignore tigl::CTiglStringerFrameBorderedObject;
%include "CTiglStringerFrameBorderedObject.h"
%include "generated/CPACSSkinSegment.h"
%include "CCPACSSkinSegment.h"
%include "generated/CPACSCargoDoorsAssembly.h"
%include "generated/CPACSPressureBulkheadAssembly.h"
%include "CCPACSPressureBulkheadAssembly.h"
%include "generated/CPACSLongFloorBeamsAssembly.h"
%include "CCPACSLongFloorBeamsAssembly.h"
%include "generated/CPACSCargoCrossBeamStrutsAssembly.h"
%include "CCPACSCargoCrossBeamStrutsAssembly.h"
%include "generated/CPACSCargoCrossBeamsAssembly.h"
%include "CCPACSCargoCrossBeamsAssembly.h"
%include "generated/CPACSAlignmentCrossBeam.h"
%include "generated/CPACSCrossBeamAssemblyPosition.h"
%include "CCPACSCrossBeamAssemblyPosition.h"
%include "generated/CPACSCrossBeamStrutAssemblyPosition.h"
%include "CCPACSCrossBeamStrutAssemblyPosition.h"
%include "generated/CPACSPressureBulkheadAssemblyPosition.h"
%include "CCPACSPressureBulkheadAssemblyPosition.h"
%include "generated/CPACSFramesAssembly.h"
%include "CCPACSFramesAssembly.h"
%include "generated/CPACSStringersAssembly.h"
%include "CCPACSStringersAssembly.h"
%include "generated/CPACSSkinSegments.h"
%include "generated/CPACSSkin.h"
%include "generated/CPACSFuselageStructure.h"
%include "CCPACSFuselageStructure.h"


%include "generated/CPACSWingSkin.h"
%include "CPACSRotor_type.h"
%include "TiglRotorHubType.h"
%include "CTiglRelativelyPositionedComponent.h"
%include "CTiglAttachedRotorBlade.h"
%include "CPACSRotorHubHinge_type.h"

// -------------------- Wing Structure -------------------//

namespace tigl {
class CCPACSWingCSStructure;
class CCPACSWingSparSegment;
class CCPACSWingRibsPositioning;
}
%include "generated/CPACSRibRotation_ribRotationReference.h"
%include "generated/CPACSWingRibExplicitPositioning.h"
%include "generated/CPACSRibRotation.h"
%include "CCPACSWingRibRotation.h"
%include "generated/CPACSRibCrossingBehaviour.h"
%include "generated/CPACSWingRibsPositioning.h"
%include "CCPACSWingRibsPositioning.h"
%include "generated/CPACSCap.h"
%include "generated/CPACSWingRibCell.h"
%include "generated/CPACSWingRibCrossSection.h"
%include "CCPACSWingRibCrossSection.h"
%include "generated/CPACSWingRibsDefinition.h"
%include "CCPACSWingRibsDefinition.h"
%include "generated/CPACSSparPositionUIDs.h"
%include "CCPACSWingSparPositionUIDs.h"
%include "generated/CPACSSparCells.h"
%include "generated/CPACSWeb.h"
%include "generated/CPACSSparCrossSection.h"
%include "generated/CPACSSparPositions.h"
%include "generated/CPACSSparSegments.h"
%include "generated/CPACSSparSegment.h"
%include "CCPACSWingSparSegments.h"

%apply double &OUTPUT { double& eta, double& xsi };
%include "CCPACSWingSparSegment.h"
%include "CCPACSWingSparPositions.h"
%include "CTiglWingStructureReference.h"
%clear double& eta, double& xsi;

%include "generated/CPACSWingSpar.h"
%include "CCPACSWingSpars.h"
%include "generated/CPACSWingRibsDefinitions.h"
%include "CCPACSWingRibsDefinitions.h"
%include "generated/CPACSCellPositioningSpanwise.h"
%include "CCPACSWingCellPositionSpanwise.h"
%include "generated/CPACSCellPositioningChordwise.h"
%include "CCPACSWingCellPositionChordwise.h"
%include "EtaXsi.h"
%include "generated/CPACSWingCell.h"
%include "CCPACSWingCell.h"
%include "generated/CPACSWingShell.h"
%include "CCPACSWingShell.h"
%template (WingComponentSegmentTemplate) tigl::CTiglAbstractSegment<tigl::CCPACSWingComponentSegment>;
%include "generated/CPACSComponentSegment.h"

%apply double &OUTPUT { double& eta, double& xsi, double& errorDistance};
%include "CCPACSWingComponentSegment.h"
%clear double& eta, double& xsi, double& errorDistance;

%include "generated/CPACSWingComponentSegmentStructure.h"
%include "CCPACSWingCSStructure.h"

// -------------------- Wing -------------------//

%include "generated/CPACSComponentSegments.h"
%include "CCPACSWingComponentSegments.h"
%include "generated/CPACSPositionings.h"
%include "CCPACSPositionings.h"
// We have to rename the enums since they collide with those from tigl.h
%rename(GuideCurve_C0) tigl::generated::C0;
%rename(GuideCurve_C1_from_previous) tigl::generated::C1_from_previous;
%rename(GuideCurve_C2_from_previous) tigl::generated::C2_from_previous;
%rename(GuideCurve_C1_to_previous) tigl::generated::C1_to_previous;
%rename(GuideCurve_C2_to_previous) tigl::generated::C2_to_previous;
%include "CPACSGuideCurve_continuity.h"
%include "generated/CPACSGuideCurve.h"
%include "generated/CPACSGuideCurves.h"
%include "CCPACSGuideCurves.h"
%include "generated/CPACSCst2D.h"
%include "ITiglWingProfileAlgo.h"
%include "CCPACSWingProfileCST.h"
%include "PTiglWingProfileAlgo.h"
%include "generated/CPACSFuselageElements.h"
%include "CCPACSFuselageSectionElements.h"
%include "generated/CPACSProfileGeometry.h"
%include "CCPACSWingProfile.h"
%include "CCPACSFuselageProfile.h"
%include "CTiglWingConnection.h"
%include "CTiglFuselageConnection.h"
%include "generated/CPACSWingElements.h"
%include "CCPACSWingSectionElements.h"
%include "generated/CPACSWingElement.h"
%include "CCPACSWingSectionElement.h"
%include "generated/CPACSFuselageElement.h"
%include "CCPACSFuselageSectionElement.h"
%include "generated/CPACSWingSection.h"
%include "CCPACSWingSection.h"
%include "generated/CPACSFuselageSection.h"
%include "CCPACSFuselageSection.h"
%include "generated/CPACSWingSections.h"
%include "CCPACSWingSections.h"
%include "generated/CPACSFuselageSections.h"
%include "CCPACSFuselageSections.h"
%include "generated/CPACSWingElements.h"
%include "CCPACSWingSectionElements.h"
%include "generated/CPACSFuselageElements.h"
%include "CCPACSFuselageSectionElements.h"
%include "generated/CPACSGuideCurveProfileGeometry.h"
%include "CCPACSGuideCurveProfile.h"
%include "generated/CPACSWingSegment.h"

%template (WingSegmentTemplate) tigl::CTiglAbstractSegment<tigl::CCPACSWingSegment>;
%apply double &OUTPUT { double& eta, double& xsi };
%include "CCPACSWingSegment.h"
%clear double& eta, double& xsi;

%include "generated/CPACSFuselageSegment.h"
%template (FuselageSegmentTemplate) tigl::CTiglAbstractSegment<tigl::CCPACSFuselageSegment>;
%include "CCPACSFuselageSegment.h"
%include "generated/CPACSWingAirfoils.h"
%include "CCPACSWingProfiles.h"
%include "CTiglRelativelyPositionedComponent.h"
%include "generated/CPACSWing.h"

%apply double& OUTPUT { double& mac_chord, double& mac_x, double& mac_y, double& mac_z };
%apply bool& OUTPUT { double& onTop };
%include "CCPACSWing.h"
%clear double &mac_chord, double &mac_x, double &mac_y, double &mac_z;
%clear bool& onTop;

%include "generated/CPACSWings.h"
%include "CCPACSWings.h"
%include "generated/CPACSRotorHubHinge.h"
%include "CCPACSRotorHinge.h"
%include "generated/CPACSRotorBladeAttachment.h"
%include "CCPACSRotorBladeAttachment.h"
%include "generated/CPACSRotorHub.h"
%include "CCPACSRotorHub.h"
%include "generated/CPACSRotor.h"
%include "CCPACSRotor.h"
%include "generated/CPACSRotors.h"
%include "CCPACSRotors.h"
%include "generated/CPACSFuselageProfiles.h"
%include "CCPACSFuselageProfiles.h"
%include "generated/CPACSFuselage.h"
%include "CCPACSFuselage.h"
%include "generated/CPACSFuselages.h"
%include "CCPACSFuselages.h"
%include "TiglFarFieldType.h"
%include "generated/CPACSCellPositioningSpanwise.h"
%include "generated/CPACSCellPositioningChordwise.h"
%include "CCPACSWingCellPositionSpanwise.h"
%include "CCPACSWingCellPositionChordwise.h"
%include "generated/CPACSWingCell.h"
%include "CCPACSWingCell.h"
%include "generated/CPACSWingCells.h"
%include "CCPACSWingCells.h"
%include "generated/CPACSWingShell.h"
%include "CCPACSWingShell.h"
%include "generated/CPACSFarField.h"
%include "CCPACSFarField.h"
%include "generated/CPACSLinkToFileType_format.h"
%include "generated/CPACSLinkToFile.h"
%include "generated/CPACSGenericGeometricComponent.h"
%include "CCPACSExternalObject.h"
%include "CTiglWingSegmentList.h"
%include "CTiglWingChordface.h"

// ---- Engine Pylon -- //
%include "generated/CPACSEnginePylon.h"
%include "CCPACSEnginePylon.h"
%include "generated/CPACSEnginePylons.h"
%include "CCPACSEnginePylons.h"

// CTiglUIDManager::GetGeometricComponent returns the interface type ITiglGeometricComponent
// In the target language, we want to get the concrete type back
%factory(tigl::ITiglGeometricComponent& tigl::CTiglUIDManager::GetGeometricComponent,
         tigl::CCPACSFuselage,
         tigl::CCPACSFuselageSegment,
         tigl::CCPACSWing,
         tigl::CCPACSWingSegment,
         tigl::CCPACSWingComponentSegment,
         tigl::CTiglWingChordface,
         tigl::CCPACSExternalObject,
         tigl::CCPACSGenericSystem,
         tigl::CCPACSRotor,
         tigl::CTiglAttachedRotorBlade,
         tigl::CCPACSCrossBeamAssemblyPosition,
         tigl::CCPACSCrossBeamStrutAssemblyPosition,
         tigl::CCPACSDoorAssemblyPosition,
         tigl::CCPACSLongFloorBeam,
         tigl::CCPACSPressureBulkheadAssemblyPosition,
         tigl::CCPACSWingCell,
         tigl::CCPACSWingRibsDefinition,
         tigl::CCPACSWingSparSegment,
         tigl::CCPACSEnginePylon
);

namespace tigl
{
typedef std::map<const std::string, ITiglGeometricComponent*> ShapeContainerType;
typedef std::map<const std::string, CTiglRelativelyPositionedComponent*> RelativeComponentContainerType;

class CTiglUIDManager
{
public:
    CTiglUIDManager();
    bool IsUIDRegistered(const std::string& uid) const;
    bool TryUnregisterObject(const std::string& uid);
    void UnregisterObject(const std::string& uid);
    bool HasGeometricComponent(const std::string& uid) const;
    ITiglGeometricComponent& GetGeometricComponent(const std::string& uid) const;
    CTiglRelativelyPositionedComponent* GetParentGeometricComponent(const std::string& uid) const;
    const RelativeComponentContainerType& GetRootGeometricComponents() const;
    void SetParentComponents();
    const ShapeContainerType& GetShapeContainer() const;
    void Clear();
};
} // namespace tigl

%include "CTiglShapeCache.h"
%include "CCPACSGenericSystem.h"
%include "CCPACSGenericSystems.h"
%include "CCPACSACSystems.h"
%include "generated/CPACSRotorAirfoils.h"
%include "CCPACSRotorProfiles.h"
%include "CCPACSConfiguration.h"
%include "CCPACSConfigurationManager.h"
%include "CTiglFusePlane.h"
%include "generated/CPACSDoorAssemblyPosition_doorType.h"
%include "generated/CPACSDoorAssemblyPosition.h"
%include "CCPACSDoorAssemblyPosition.h"
%include "generated/CPACSLongFloorBeam.h"
%include "CCPACSLongFloorBeam.h"

%clear double* eta, double* xsi;
