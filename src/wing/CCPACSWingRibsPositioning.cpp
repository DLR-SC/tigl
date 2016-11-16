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
#include "CCPACSWingRibsPositioning.h"

#include "CCPACSWingCSStructure.h"
#include "CCPACSWingRibsDefinition.h"
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "TixiSaveExt.h"


std::string toString(tigl::CCPACSWingRibsPositioning::CrossingBehaviour crossingBehaviour)
{
    switch (crossingBehaviour) {
    case tigl::CCPACSWingRibsPositioning::CROSSING_CROSS:
        return "cross";
        break;
    case tigl::CCPACSWingRibsPositioning::CROSSING_END:
        return "end";
        break;
    default:
        throw tigl::CTiglError("Unknown CrossingBehaviour can not be converted to string!");
    }
}

tigl::CCPACSWingRibsPositioning::CrossingBehaviour ribCrossingFromString(const std::string& crossingBehaviour)
{
    if (crossingBehaviour == "cross") {
        return tigl::CCPACSWingRibsPositioning::CROSSING_CROSS;
    }
    else if (crossingBehaviour == "end") {
        return tigl::CCPACSWingRibsPositioning::CROSSING_END;
    }
    throw tigl::CTiglError("Unknown CrossingBehaviour " + crossingBehaviour + " found in CCPACSWingRibsPositioning!");
}


namespace tigl
{

CCPACSWingRibsPositioning::CCPACSWingRibsPositioning(CCPACSWingRibsDefinition& parent)
: parent(parent),
  ribRotation(parent)
{
    Cleanup();
}

CCPACSWingRibsPositioning::~CCPACSWingRibsPositioning()
{
    Cleanup();
}

void CCPACSWingRibsPositioning::Cleanup()
{
    ribReference.clear();
    startDefinitionType = ETA_START;
    etaStart = 0.;
    elementStartUID.clear();
    sparPositionStartUID.clear();
    endDefinitionType = ETA_END;
    etaEnd = 0.;
    elementEndUID.clear();
    sparPositionEndUID.clear();
    ribStart.clear();
    ribEnd.clear();
    ribCountDefinitionType = NUMBER_OF_RIBS;
    numberOfRibs = 0;
    spacing = 0.;
    ribCrossingBehaviour = CROSSING_CROSS;
    ribRotation.Cleanup();
}

void CCPACSWingRibsPositioning::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& ribsPositioningXPath)
{
    Cleanup();

    // Get subelement "ribReference"
    char* ptrRibReference = NULL;
    if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/ribReference").c_str(), &ptrRibReference) != SUCCESS) {
        LOG(ERROR) << "Missing ribReference";
        throw CTiglError("Error: Missing ribReference in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    ribReference = ptrRibReference;

    // Get start definition of rib
    const std::string etaStartElementPath = ribsPositioningXPath + "/etaStart";
    const std::string elementStartUIDElementPath = ribsPositioningXPath + "/elementStartUID";
    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    const std::string sparPositionStartUIDElementPath = ribsPositioningXPath + "/sparPositionStartUID";
    if (tixiCheckElement(tixiHandle, etaStartElementPath.c_str()) == SUCCESS) {                    // etaStart
        if (tixiGetDoubleElement(tixiHandle, etaStartElementPath.c_str(), &etaStart) != SUCCESS) {
            LOG(ERROR) << "Missing etaStart";
            throw CTiglError("Error: Missing etaStart in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        startDefinitionType = ETA_START;
    }
    else if (tixiCheckElement(tixiHandle, elementStartUIDElementPath.c_str()) == SUCCESS) {        // elementStartUID
        char* ptrElementUID = NULL;
        if (tixiGetTextElement(tixiHandle, elementStartUIDElementPath.c_str(), &ptrElementUID) != SUCCESS) {
            LOG(ERROR) << "Missing elementStartUID";
            throw CTiglError("Error: Missing elementStartUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        elementStartUID = ptrElementUID;
        startDefinitionType = ELEMENT_START;
    }
    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    else if (tixiCheckElement(tixiHandle, (ribsPositioningXPath + "/sparPositionStartUID").c_str()) == SUCCESS) {
        char* ptrSparPositionStartUID;
        if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/sparPositionStartUID").c_str(), &ptrSparPositionStartUID) != SUCCESS) {
            LOG(ERROR) << "Missing sparPositionStartUID";
            throw CTiglError("Error: Missing sparPositionStartUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        sparPositionStartUID = ptrSparPositionStartUID;
        startDefinitionType = SPARPOSITION_START;
    }
    else {
        LOG(ERROR) << "Missing etaStart or elementStartUID";
        throw CTiglError("Error: Missing etaStart or elementStartUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }

    // Get end definition of rib
    const std::string etaEndElementPath = ribsPositioningXPath + "/etaEnd";
    const std::string elementEndUIDElementPath = ribsPositioningXPath + "/elementEndUID";
    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    const std::string sparPositionEndUIDElementPath = ribsPositioningXPath + "/sparPositionEndUID";
    if (tixiCheckElement(tixiHandle, etaEndElementPath.c_str()) == SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, etaEndElementPath.c_str(), &etaEnd) != SUCCESS) {
            LOG(ERROR) << "Missing etaEnd";
            throw CTiglError("Error: Missing etaEnd in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        endDefinitionType = ETA_END;
    }
    else if (tixiCheckElement(tixiHandle, elementEndUIDElementPath.c_str()) == SUCCESS) {
        char* ptrElementUID = NULL;
        if (tixiGetTextElement(tixiHandle, elementEndUIDElementPath.c_str(), &ptrElementUID) != SUCCESS) {
            LOG(ERROR) << "Missing elementEndUID";
            throw CTiglError("Error: Missing elementEndUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        elementEndUID = ptrElementUID;
        endDefinitionType = ELEMENT_END;
    }
    // NOTE: definition of start/end of rib via spar position not conform with CPACS format (v2.3)
    else if (tixiCheckElement(tixiHandle, (ribsPositioningXPath + "/sparPositionEndUID").c_str()) == SUCCESS) {
        char* ptrSparPositionEndUID;
        if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/sparPositionEndUID").c_str(), &ptrSparPositionEndUID) != SUCCESS) {
            LOG(ERROR) << "Missing sparPositionEndUID";
            throw CTiglError("Error: Missing sparPositionEndUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
        }
        sparPositionEndUID = ptrSparPositionEndUID;
        endDefinitionType = SPARPOSITION_END;
    }
    else {
        LOG(ERROR) << "Missing etaEnd or elementEndUID";
        throw CTiglError("Error: Missing etaEnd or elementEndUID in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }

    // Get subelement "ribStart"
    char* ptrRibStart = NULL;
    if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/ribStart").c_str(), &ptrRibStart) != SUCCESS) {
        LOG(ERROR) << "Missing ribStart";
        throw CTiglError("Error: Missing ribStart in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    ribStart = ptrRibStart;

    // Get subelement "ribEnd"
    char* ptrRibEnd = NULL;
    if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/ribEnd").c_str(), &ptrRibEnd) != SUCCESS) {
        LOG(ERROR) << "Missing ribEnd";
        throw CTiglError("Error: Missing ribEnd in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    ribEnd = ptrRibEnd;

    // Get subelement "numberOfRibs" OR "spacing"
    const std::string numRibsElementPath = (ribsPositioningXPath + "/numberOfRibs").c_str();
    const std::string spacingElementPath = (ribsPositioningXPath + "/spacing").c_str();
    if (tixiCheckElement(tixiHandle, numRibsElementPath.c_str()) == SUCCESS) {
        if (tixiGetIntegerElement(tixiHandle, numRibsElementPath.c_str(), &numberOfRibs) != SUCCESS) {
            throw CTiglError("Error reading numberOfRibs in CCPACSWingRibsPositioning::ReadCPACS!");
        }
        ribCountDefinitionType = NUMBER_OF_RIBS;
    } else if (tixiCheckElement(tixiHandle, spacingElementPath.c_str()) == SUCCESS) {
        if (tixiGetDoubleElement(tixiHandle, spacingElementPath.c_str(), &spacing) != SUCCESS) {
            throw CTiglError("Error reading spacing in CCPACSWingRibsPositioning::ReadCPACS!");
        }
        ribCountDefinitionType = SPACING;
    } else {
        LOG(ERROR) << "Missing numberOfRibs OR spacing";
        throw CTiglError("Error: Missing numberOfRibs OR spacing in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }

    // Get subelement "ribCrossingBehaviour"
    char* ptrRibCrossing = NULL;
    if (tixiGetTextElement(tixiHandle, (ribsPositioningXPath + "/ribCrossingBehaviour").c_str(), &ptrRibCrossing) != SUCCESS) {
        LOG(ERROR) << "Missing ribCrossingBehaviour";
        throw CTiglError("Error: Missing ribCrossingBehaviour in CCPACSWingRibsPositioning::ReadCPACS!", TIGL_XML_ERROR);
    }
    ribCrossingBehaviour = ribCrossingFromString(ptrRibCrossing);

    // Get subelement ribRotation
    if (tixiCheckElement(tixiHandle, (ribsPositioningXPath + "/ribRotation").c_str()) != SUCCESS) {
        LOG(ERROR) << "Missing node ribRotation";
        throw CTiglError("Error: Missing node ribRotation in CCPACSWingRibsDefinition::ReadCPACS!", TIGL_XML_ERROR);
    }
    ribRotation.ReadCPACS(tixiHandle, ribsPositioningXPath + "/ribRotation");
}

void CCPACSWingRibsPositioning::WriteCPACS(TixiDocumentHandle tixiHandle, const std::string& ribsPositioningXPath) const
{
    TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "ribReference", ribReference.c_str());

    switch (startDefinitionType) {
    case ETA_START:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, ribsPositioningXPath.c_str(), "etaStart", etaStart, NULL);
        break;
    case ELEMENT_START:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "elementStartUID", elementStartUID.c_str());
        break;
    case SPARPOSITION_START:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "sparPositionStartUID", sparPositionStartUID.c_str());
        break;
    default:
        throw CTiglError("Unknown StartDefinitionType found in CCPACSWingRibsPositioning::WriteCPACS");
    }

    switch (endDefinitionType) {
    case ETA_END:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, ribsPositioningXPath.c_str(), "etaEnd", etaEnd, NULL);
        break;
    case ELEMENT_END:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "elementEndUID", elementEndUID.c_str());
        break;
    case SPARPOSITION_END:
        TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "sparPositionEndUID", sparPositionEndUID.c_str());
        break;
    default:
        throw CTiglError("Unknown EndDefinitionType found in CCPACSWingRibsPositioning::WriteCPACS");
    }

    TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "ribStart", ribStart.c_str());
    TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "ribEnd", ribEnd.c_str());

    switch (ribCountDefinitionType) {
    case NUMBER_OF_RIBS:
        TixiSaveExt::TixiSaveIntElement(tixiHandle, ribsPositioningXPath.c_str(), "numberOfRibs", numberOfRibs);
        break;
    case SPACING:
        TixiSaveExt::TixiSaveDoubleElement(tixiHandle, ribsPositioningXPath.c_str(), "spacing", spacing, NULL);
        break;
    default:
        throw CTiglError("Unknown RibCountDefinitionType found in CCPACSWingRibsPositioning::WriteCPACS");
    }

    TixiSaveExt::TixiSaveTextElement(tixiHandle, ribsPositioningXPath.c_str(), "ribCrossingBehaviour", toString(ribCrossingBehaviour).c_str());

    TixiSaveExt::TixiSaveElement(tixiHandle, ribsPositioningXPath.c_str(), "ribRotation");
    ribRotation.WriteCPACS(tixiHandle, ribsPositioningXPath + "/ribRotation");
}

const std::string& CCPACSWingRibsPositioning::GetRibReference() const
{
    return ribReference;
}

void CCPACSWingRibsPositioning::SetRibReference(const std::string& ref)
{
    ribReference = ref;
    // invalidate whole component segment structure, since cells could reference the ribs
    parent.GetStructure().Invalidate();
}

CCPACSWingRibsPositioning::StartDefinitionType CCPACSWingRibsPositioning::GetStartDefinitionType() const
{
    return startDefinitionType;
}

double CCPACSWingRibsPositioning::GetEtaStart() const
{
    if (startDefinitionType != ETA_START) {
        throw CTiglError("RibsPositioning is not defined via etaStart. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetEtaStart()");
    }
    return etaStart;
}

void CCPACSWingRibsPositioning::SetEtaStart(double value)
{
    etaStart = value;

    elementStartUID.clear();
    sparPositionStartUID.clear();
    startDefinitionType = ETA_START;

    // invalidate whole component segment structure, since cells could reference the ribs
    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetElementStartUID() const
{
    if (startDefinitionType != ELEMENT_START) {
        throw CTiglError("RibsPositioning is not defined via elementStartUID. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetElementStartUID()");
    }
    return elementStartUID;
}

void CCPACSWingRibsPositioning::SetElementStartUID(const std::string& uid)
{
    elementStartUID = uid;

    etaStart = 0.;
    sparPositionStartUID.clear();
    startDefinitionType = ELEMENT_START;

    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetSparPositionStartUID() const
{
    if (startDefinitionType != SPARPOSITION_START) {
        throw CTiglError("RibsPositioning is not defined via sparPositionStartUID. Please check StartDefinitionType first before calling CCPACSWingRibsPositioning::GetSparPositionStartUID()");
    }
    return sparPositionStartUID;
}

void CCPACSWingRibsPositioning::SetSparPositionStartUID(const std::string& uid)
{
    sparPositionStartUID = uid;

    etaStart = 0.;
    elementStartUID.clear();
    startDefinitionType = SPARPOSITION_START;

    parent.GetStructure().Invalidate();
}

CCPACSWingRibsPositioning::EndDefinitionType CCPACSWingRibsPositioning::GetEndDefinitionType() const
{
    return endDefinitionType;
}

double CCPACSWingRibsPositioning::GetEtaEnd() const
{
    if (endDefinitionType != ETA_END) {
        throw CTiglError("RibsPositioning is not defined via etaEnd. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetEtaEnd()");
    }
    return etaEnd;
}

void CCPACSWingRibsPositioning::SetEtaEnd(double value)
{
    etaEnd = value;

    elementEndUID.clear();
    sparPositionEndUID.clear();
    endDefinitionType = ETA_END;

    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetElementEndUID() const
{
    if (endDefinitionType != ELEMENT_END) {
        throw CTiglError("RibsPositioning is not defined via elementEndUID. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetElementEndUID()");
    }
    return elementEndUID;
}

void CCPACSWingRibsPositioning::SetElementEndUID(const std::string& uid)
{
    elementEndUID = uid;

    etaEnd = 0.;
    sparPositionEndUID.clear();
    endDefinitionType = ELEMENT_END;

    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetSparPositionEndUID() const
{
    if (endDefinitionType != SPARPOSITION_END) {
        throw CTiglError("RibsPositioning is not defined via sparPositionEndUID. Please check EndDefinitionType first before calling CCPACSWingRibsPositioning::GetSparPositionEndUID()");
    }
    return sparPositionEndUID;
}

void CCPACSWingRibsPositioning::SetSparPositionEndUID(const std::string& uid)
{
    sparPositionEndUID = uid;

    etaEnd = 0.;
    elementEndUID.clear();
    endDefinitionType = SPARPOSITION_END;

    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetRibStart() const
{
    return ribStart;
}

void CCPACSWingRibsPositioning::SetRibStart(const std::string& str)
{
    ribStart = str;
    parent.GetStructure().Invalidate();
}

const std::string& CCPACSWingRibsPositioning::GetRibEnd() const
{
    return ribEnd;
}

void CCPACSWingRibsPositioning::SetRibEnd(const std::string& str)
{
    ribEnd = str;
    parent.GetStructure().Invalidate();
}

CCPACSWingRibsPositioning::RibCountDefinitionType CCPACSWingRibsPositioning::GetRibCountDefinitionType() const
{
    return ribCountDefinitionType;
}

int CCPACSWingRibsPositioning::GetNumberOfRibs() const
{
    if (ribCountDefinitionType != NUMBER_OF_RIBS) {
        throw CTiglError("RibsPositioning is not defined via numberOfRibs. Please check RibCountDefinitionType first before calling CCPACSWingRibsPositioning::GetNumberOfRibs()");
    }
    return numberOfRibs;
}

void CCPACSWingRibsPositioning::SetNumberOfRibs(int numRibs)
{
    numberOfRibs = numRibs;

    spacing = 0.;
    ribCountDefinitionType = NUMBER_OF_RIBS;

    parent.GetStructure().Invalidate();
}

double CCPACSWingRibsPositioning::GetSpacing() const
{
    if (ribCountDefinitionType != SPACING) {
        throw CTiglError("RibsPositioning is not defined via spacing. Please check RibCountDefinitionType first before calling CCPACSWingRibsPositioning::GetSpacing()");
    }
    return spacing;
}

void CCPACSWingRibsPositioning::SetSpacing(double value)
{
    spacing = value;

    numberOfRibs = 0;
    ribCountDefinitionType = SPACING;

    parent.GetStructure().Invalidate();
}

CCPACSWingRibsPositioning::CrossingBehaviour CCPACSWingRibsPositioning::GetRibCrossingBehaviour() const
{
    return ribCrossingBehaviour;
}

void CCPACSWingRibsPositioning::SetRibCrossingBehaviour(CCPACSWingRibsPositioning::CrossingBehaviour behaviour)
{
    ribCrossingBehaviour = behaviour;
    parent.GetStructure().Invalidate();
}

const CCPACSWingRibRotation& CCPACSWingRibsPositioning::GetRibRotation() const
{
    return ribRotation;
}

CCPACSWingRibRotation& CCPACSWingRibsPositioning::GetRibRotation()
{
    return ribRotation;
}

} // end namespace tigl
