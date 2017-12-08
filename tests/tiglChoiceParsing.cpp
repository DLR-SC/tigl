/* 
* Copyright (C) RISC Software GmbH
*
* Created: 2017-06-08 Bernhard Manfred Gruber
* Changed: $Id$ 
*
* Version: $Revision$
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

/**
* @file
* @brief Tests for the generated parsing and validation of choices
*/

#include "test.h" // Brings in the GTest framework
#include "CCPACSWingRibsPositioning.h"

namespace {
    class tiglChoiceParsingTest : public ::testing::Test {};

    void ReadAndValidateMaterial(const std::string& cpacs, const bool expected) {
        TixiDocumentHandle handle;
        const ReturnCode ret = tixiImportFromString(cpacs.c_str(), &handle);
        ASSERT_EQ(ret, SUCCESS);

        tigl::CCPACSWingRibsPositioning rp(NULL);
        rp.ReadCPACS(handle, "/ribsPositioning");
        ASSERT_EQ(rp.ValidateChoices(), expected);
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningNoChoice) {
       ReadAndValidateMaterial(
           "<ribsPositioning>"
           "    <ribReference>trailingEdge</ribReference>"
           "    <ribStart>leadingEdge</ribStart>"
           "    <ribEnd>trailingEdge</ribEnd>"
           "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
           "    <ribRotation>"
           "        <z>16.0</z>"
           "    </ribRotation>"
            "</ribsPositioning>",
           false
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices111) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <elementStartUID>start</elementStartUID>" // choice 1
            "    <elementEndUID>start</elementEndUID>" // choice 1
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <numberOfRibs>42</numberOfRibs>" // choice 1
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices112) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <elementStartUID>start</elementStartUID>" // choice 1
            "    <elementEndUID>start</elementEndUID>" // choice 1
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <spacing>2.4</spacing>" // choice 2
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices121) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <elementStartUID>start</elementStartUID>" // choice 1
            "    <etaEnd>3.7</etaEnd>" // choice 2
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <numberOfRibs>42</numberOfRibs>" // choice 1
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices122) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <elementStartUID>start</elementStartUID>" // choice 1
            "    <etaEnd>3.7</etaEnd>" // choice 2
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <spacing>2.4</spacing>" // choice 2
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices211) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <etaStart>0.4</etaStart>" // choice 2
            "    <elementEndUID>start</elementEndUID>" // choice 1
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <numberOfRibs>42</numberOfRibs>" // choice 1
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices212) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <etaStart>0.4</etaStart>" // choice 2
            "    <elementEndUID>start</elementEndUID>" // choice 1
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <spacing>2.4</spacing>" // choice 2
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices221) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <etaStart>0.4</etaStart>" // choice 2
            "    <etaEnd>3.7</etaEnd>" // choice 2
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <numberOfRibs>42</numberOfRibs>" // choice 1
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }

    TEST_F(tiglChoiceParsingTest, ribsPositioningChoices222) {
        ReadAndValidateMaterial(
            "<ribsPositioning>"
            "    <ribReference>trailingEdge</ribReference>"
            "    <etaStart>0.4</etaStart>" // choice 2
            "    <etaEnd>3.7</etaEnd>" // choice 2
            "    <ribStart>leadingEdge</ribStart>"
            "    <ribEnd>trailingEdge</ribEnd>"
            "    <spacing>2.4</spacing>" // choice 2
            "    <ribCrossingBehaviour>cross</ribCrossingBehaviour>"
            "    <ribRotation>"
            "        <z>16.0</z>"
            "    </ribRotation>"
            "</ribsPositioning>",
            true
        );
    }
}
