/*
* Copyright (C) 2016 Airbus Defence and Space
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

#ifndef TIGLETAXSIFUNCTIONS_H
#define TIGLETAXSIFUNCTIONS_H

#include <string>

#include <gp_Pnt.hxx>

#include "tigl_internal.h"
#include "EtaXsi.h"

namespace tigl
{

// forward declaration
class CCPACSWingComponentSegment;
class CCPACSWingRibsDefinition;
class CCPACSWingSparSegment;
class CTiglWingStructureReference;
class CTiglUIDManager;
class CCPACSEtaIsoLine;
class CCPACSXsiIsoLine;

/**
* @brief Returns the chordline point of the section element at the passed xsi value
*/
TIGL_EXPORT gp_Pnt getSectionElementChordlinePoint(const CCPACSWingComponentSegment& cs, const std::string& sectionElementUID, double xsi);

/**
* @brief Computes the xsi value of a spar at the passed eta position
*/
TIGL_EXPORT double computeSparXsiValue(const CTiglWingStructureReference& wsr, const CCPACSWingSparSegment& spar, double eta);

/**
* @brief Computes the eta value of a rib at the passed xsi position
*/
TIGL_EXPORT double computeRibEtaValue(const CTiglWingStructureReference& wsr, const CCPACSWingRibsDefinition& rib, int ribIndex, double xsi);

/**
* @brief Computes the eta/xsi value of the intersection point of a rib and a spar
*/
TIGL_EXPORT EtaXsi computeRibSparIntersectionEtaXsi(const CTiglWingStructureReference& wsr, const CCPACSWingRibsDefinition& rib, int ribIndex, const CCPACSWingSparSegment& spar);

/**
 * Following are routines, that transform e.g. segment eta/xsi coordinates into the CS system or into TED system
 */
TIGL_EXPORT double transformEtaToCSOrTed(const CCPACSEtaIsoLine& eta, const CTiglUIDManager& uidMgr);
TIGL_EXPORT double transformXsiToCSOrTed(const CCPACSXsiIsoLine& xsi, const CTiglUIDManager& uidMgr);
TIGL_EXPORT double transformEtaToCSOrTed(double eta, const std::string& referenceUid, const CTiglUIDManager& uidMgr);
TIGL_EXPORT double transformXsiToCSOrTed(double xsi, const std::string& referenceUid, const CTiglUIDManager& uidMgr);

TIGL_EXPORT EtaXsi transformEtaXsiToCSOrTed(EtaXsi etaXsi, const std::string& referenceUid, const CTiglUIDManager& uidMgr);

}


#endif // TIGLETAXSIFUNCTIONS_H
