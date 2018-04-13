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

%include common.i

%include Geom_headers.i


%import geometry.i
%import core.i
%import Geom.i
%import TopoDS.i

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
%}

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";

// wrap optional classes
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


%include "generated/CPACSCargoDoorsAssembly.h"
%include "generated/CPACSPressureBulkheadAssembly.h"
%include "CCPACSPressureBulkheadAssembly.h"
%include "generated/CPACSLongFloorBeamsAssembly.h"
%include "CCPACSLongFloorBeamsAssembly.h"
%include "generated/CPACSCargoCrossBeamStrutsAssembly.h"
%include "CCPACSCargoCrossBeamStrutsAssembly.h"
%include "generated/CPACSCargoCrossBeamsAssembly.h"
%include "CCPACSCargoCrossBeamsAssembly.h"
%include "generated/CPACSFramesAssembly.h"
%include "CCPACSFramesAssembly.h"
%include "generated/CPACSStringersAssembly.h"
%include "CCPACSStringersAssembly.h"
%include "generated/CPACSSkinSegments.h"
%include "generated/CPACSSkin.h"
%include "generated/CPACSFuselageStructure.h"
%include "CCPACSFuselageStructure.h"
%include "generated/CPACSMaterialDefinition.h"
%include "CCPACSMaterialDefinition.h"
%include "generated/CPACSWingSkin.h"
%include "CPACSRotor_type.h"
%include "TiglRotorHubType.h"
%include "CTiglRelativelyPositionedComponent.h"
%include "CTiglAttachedRotorBlade.h"
%include "CPACSRotorHubHinge_type.h"

namespace tigl {
class CCPACSWingCSStructure;
class CCPACSWingSparSegment;
class CCPACSWingRibsPositioning;
}
%include "generated/CPACSRibRotation_ribRotationReference.h"
%include "generated/CPACSWingRibExplicitPositioning.h"
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
%include "CCPACSWingSparSegment.h"
%include "CCPACSWingSparPositions.h"
%include "CTiglWingStructureReference.h"
%include "generated/CPACSWingSpar.h"
%include "CCPACSWingSpars.h"
%include "generated/CPACSWingRibsDefinitions.h"
%include "CCPACSWingRibsDefinitions.h"
%include "generated/CPACSCellPositioningSpanwise.h"
%include "CCPACSWingCellPositionSpanwise.h"
%include "generated/CPACSCellPositioningChordwise.h"
%include "CCPACSWingCellPositionChordwise.h"
%include "generated/CPACSWingCell.h"
%include "CCPACSWingCell.h"
%include "generated/CPACSWingShell.h"
%include "CCPACSWingShell.h"
%template (WingComponentSegmentTemplate) tigl::CTiglAbstractSegment<tigl::CCPACSWingComponentSegment>;
%include "generated/CPACSComponentSegment.h"
%include "CCPACSWingComponentSegment.h"
%include "generated/CPACSWingComponentSegmentStructure.h"
%include "CCPACSWingCSStructure.h"
%include "generated/CPACSComponentSegments.h"
%include "CCPACSWingComponentSegments.h"
%include "generated/CPACSPositionings.h"
%include "CCPACSPositionings.h"
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
%include "CCPACSWingSegment.h"
%include "generated/CPACSFuselageSegment.h"
%template (FuselageSegmentTemplate) tigl::CTiglAbstractSegment<tigl::CCPACSFuselageSegment>;
%include "CCPACSFuselageSegment.h"
%include "generated/CPACSWingAirfoils.h"
%include "CCPACSWingProfiles.h"
%include "CTiglRelativelyPositionedComponent.h"
%include "generated/CPACSWing.h"
%include "CCPACSWing.h"
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

namespace tigl
{
class CTiglUIDManager
{
public:
    CTiglUIDManager();
    void AddGeometricComponent(const std::string& uid, ITiglGeometricComponent* componentPtr);
    void RemoveGeometricComponent(const std::string& uid);
    bool HasGeometricComponent(const std::string& uid) const;
    ITiglGeometricComponent& GetGeometricComponent(const std::string& uid) const;
    CTiglRelativelyPositionedComponent* GetParentGeometricComponent(const std::string& uid) const;
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
