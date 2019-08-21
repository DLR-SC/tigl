/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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


#ifndef TIGL_CTIGLSTANDARDIZER_H
#define TIGL_CTIGLSTANDARDIZER_H

#include "CCPACSWing.h"
#include "CCPACSFuselage.h"

namespace tigl
{

class CTiglStandardizer
{
public:
    /**
     * Standardize the fuselage using the CPACSCreator standard.
     * This mean:
     *  1) The fuselage transformation store the nose position.
     *  2) The translation of sections are set using positionings.
     *  3) Each positioning correspond to a segment, except for the nose section that has a positioning from
     *      the fuselage origin (with a length of 0, if the origin of the profile correspond to its center).
     *
     * @param fuselage
     * @param useSimpleDecomposition, if set to true the standardization use only polar decomposition.
     * This mean that the cpacs file is more readable but there can be some simplification that change the global
     * shape of the fuselage.
     */
    static void StandardizeFuselage(CCPACSFuselage& fuselage, bool useSimpleDecomposition = false);

    /**
     * Standardize the wing using the CPACSCreator standard.
     * This mean:
     *  1) The wing transformation store the root leading point translation.
     *  2) The translation of sections are set using positionings.
     *  3) Each positioning correspond to a segment, except for the root section that has a positioning from
     *      the wing origin (with a length of 0, if the origin of the airfoil correspond to its leading edge).
     *
     * @param wing
     * @param useSimpleDecomposition, if set to true the standardization use only polar decomposition.
     * This mean that the cpacs file is more readable but there can be some simplification that change the global
     * shape of the wing.
     */
    static void StandardizeWing(CCPACSWing& wing, bool useSimpleDecomposition = false);



private:

    static void StdandardizePositioningsStructure(CCPACSPositionings& positionings, std::vector<CTiglSectionElement*> elements);

    static void SetTotalTransformations(std::vector<CTiglSectionElement*> elements, std::vector<CTiglTransformation>, bool useSimpleDecomposition = false);

    static std::vector<CTiglTransformation> GetTotalTransformations(std::vector<tigl::CTiglSectionElement *> elements);
};
}

#endif //TIGL_CTIGLSTANDARDIZER_H
