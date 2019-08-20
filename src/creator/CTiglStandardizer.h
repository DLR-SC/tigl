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
     *  1) the nose position is set in the fuselage transformation
     *  2) the translation of section are set using positioning
     *  3) Each segment has exactly one positioning except for the nose section that has a positioning from
     *      the fuselage origin with a length of 0.
     *
     * @param fuselage
     */
    static void StandardizeFuselage(CCPACSFuselage& fuselage);
};
}

#endif //TIGL_CTIGLSTANDARDIZER_H
