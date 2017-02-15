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

%module (package="tigl") configuration

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
#include "CTiglAbstractPhysicalComponent.h"
#include "CTiglShapeCache.h"
#include "CTiglError.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSWingConnection.h"
#include "CCPACSFuselageConnection.h"
#include "CCPACSWingCell.h"
#include "CCPACSMaterial.h"
#include "CCPACSGenericSystem.h"
#include "CCPACSRotorBladeAttachment.h"
%}

%feature("autodoc", "3");

// rename file methods to python pep8 style
%rename("%(undercase)s", %$isfunction) "";


%include "PTiglWingProfileAlgo.h"
%include "CCPACSWingProfile.h"
%include "CCPACSFuselageProfile.h"
%include "CCPACSWingConnection.h"
%include "CCPACSFuselageConnection.h"
%include "CCPACSWingSectionElement.h"
%include "CCPACSWingPositionings.h"
%include "CCPACSFuselageSectionElement.h"
%include "CCPACSWingSection.h"
%include "CCPACSFuselageSection.h"
%include "CCPACSWingSections.h"
%include "CCPACSFuselageSections.h"
%include "CCPACSWingSectionElements.h"
%include "CCPACSFuselageSectionElements.h"
%include "CCPACSGuideCurveProfile.h"
%include "CCPACSGuideCurve.h"
%include "CCPACSWingSegment.h"
%include "CCPACSFuselageSegment.h"
%include "CCPACSWingProfiles.h"
%include "CCPACSWing.h"
%include "CCPACSWings.h"
%include "CCPACSRotorBlade.h"
%include "CCPACSRotorHinge.h"
%include "CCPACSRotorBladeAttachment.h"
%include "CCPACSRotorHub.h"
%include "CCPACSRotor.h"
%include "CCPACSRotors.h"
%include "CCPACSFuselageProfiles.h"
%include "CCPACSFuselage.h"
%include "CCPACSFuselages.h"
%include "CCPACSMaterial.h"
%include "CCPACSWingCell.h"
%include "CCPACSWingCells.h"
%include "CCPACSWingShell.h"
%include "CCPACSFarField.h"
%include "CCPACSExternalObject.h"
%include "CTiglUIDManager.h"
%include "CTiglShapeCache.h"
%include "CCPACSGenericSystem.h"
%include "CCPACSGenericSystems.h"
%include "CCPACSACSystems.h"
%include "CCPACSHeader.h"
%include "CCPACSConfiguration.h"
%include "CCPACSConfigurationManager.h"
%include "CTiglFusePlane.h"
