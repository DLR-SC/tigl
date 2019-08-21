/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#include "CTiglStandardizer.h"

void tigl::CTiglStandardizer::StandardizeFuselage(tigl::CCPACSFuselage& fuselage, bool useSimpleDecomposition)
{

    std::vector<std::string> elementsUIDS = fuselage.GetOrderedConnectedElement();
    if (elementsUIDS.size() < 2) {
        CTiglError("CTiglStandardizer: Impossible to standardize a fuselage with less than 2 valid elements!");
    }

    // create a order list of cElements;
    std::vector<CTiglSectionElement*> elements;
    for (int i = 0; i < elementsUIDS.size(); i++) {
        elements.push_back(fuselage.fuselageHelper->GetCTiglElementOfFuselage(elementsUIDS.at(i)));
    }

    // Get the total transformation of each element
    std::vector<CTiglTransformation> totalTs = GetTotalTransformations(elements);

    // Set the transformation of the fuselage such that the nose center translation is performed by the fuselage transformation
    CTiglPoint noseCenter                        = fuselage.GetNoseCenter();
    CCPACSTransformation& fuselageTransformation = fuselage.GetTransformation();
    fuselageTransformation.setTranslation(noseCenter);
    fuselage.Invalidate();

    // create the std positioning structure
    StdandardizePositioningsStructure(fuselage.GetPositionings(CreateIfNotExistsTag()), elements);
    fuselage.Invalidate();

    // Reset the resulting transformation to a transformation equivalent to its original one.
    // Remark the ticks is that setting the element will keep the standardization
    SetTotalTransformations(elements, totalTs, useSimpleDecomposition);
}

void tigl::CTiglStandardizer::StandardizeWing(tigl::CCPACSWing& wing, bool useSimpleDecomposition)
{
    std::vector<std::string> elementsUIDS = wing.GetOrderedConnectedElement();
    if (elementsUIDS.size() < 2) {
        CTiglError("CTiglStandardizer: Impossible to standardize a wing with less than 2 valid elements!");
    }

    // create a order list of cElements;
    std::vector<CTiglSectionElement*> elements;
    for (int i = 0; i < elementsUIDS.size(); i++) {
        elements.push_back(wing.wingHelper->GetCTiglElementOfWing(elementsUIDS.at(i)));
    }

    // Get the total transformation of each element
    std::vector<CTiglTransformation> totalTs = GetTotalTransformations(elements);

    // Set the transformation of the fuselage such that the nose center translation is performed by the fuselage transforamtion
    CTiglPoint rootLE                        = wing.GetRootLEPosition();
    CCPACSTransformation& wingTransformation = wing.GetTransformation();
    wingTransformation.setTranslation(rootLE);
    wing.Invalidate();

    // create the std positioning structure
    StdandardizePositioningsStructure(wing.GetPositionings(CreateIfNotExistsTag()), elements);
    wing.Invalidate();

    // Reset the resulting transformation to a transformation equivalent to its original one.
    // Remark the ticks is that setting the element will keep the standardization
    SetTotalTransformations(elements, totalTs, useSimpleDecomposition);
}

void tigl::CTiglStandardizer::StdandardizePositioningsStructure(CCPACSPositionings& positionings,
                                                                std::vector<CTiglSectionElement*> elements)
{

    std::vector<unique_ptr<CCPACSPositioning>>& refPos = positionings.GetPositionings();
    while (refPos.size() != 0) {
        positionings.RemovePositioning(*(refPos.front()));
    }

    // create the std positioning structure
    std::string fromUID = "";
    std::string toUID;
    CTiglPoint dummyDelta(0, 1.11, 0);
    for (int i = 0; i < elements.size(); i++) {
        toUID = elements.at(i)->GetSectionUID();
        positionings.CreatePositioning(fromUID, toUID, dummyDelta);
        fromUID = toUID;
    }
}

void tigl::CTiglStandardizer::SetTotalTransformations(std::vector<tigl::CTiglSectionElement*> elements,
                                                      std::vector<tigl::CTiglTransformation> totalTs,
                                                      bool useSimpleDecomposition)
{

    for (int i = 0; i < elements.size(); i++) {
        elements.at(i)->SetTotalTransformation(totalTs.at(i), GLOBAL_COORDINATE_SYSTEM, useSimpleDecomposition);
    }
}

std::vector<tigl::CTiglTransformation>
tigl::CTiglStandardizer::GetTotalTransformations(std::vector<tigl::CTiglSectionElement*> elements)
{

    std::vector<tigl::CTiglTransformation> totalTs;
    for (int i = 0; i < elements.size(); i++) {
        totalTs.push_back(elements.at(i)->GetTotalTransformation(GLOBAL_COORDINATE_SYSTEM));
    }
    return totalTs;
}