/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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

#ifndef TIGL_CTIGLWINGHELPER_H
#define TIGL_CTIGLWINGHELPER_H

#include "tigl.h"
#include "Cache.h"
#include "CTiglWingSectionElement.h"

#include <string>

namespace tigl
{

class CCPACSWing;

class CTiglWingHelper
{
public:
    CTiglWingHelper();

    CTiglWingHelper(CCPACSWing* wing);

    void SetWing(CCPACSWing* wing);

    // Returns true if the fuselage has a shape
    // This mean that a list the fuselage is composed by a valid segment and thus is composed by two elements
    bool HasShape() const;

    CTiglWingSectionElement* GetCTiglElementOfWing(const std::string& elementUID) const;

    // Returns the major direction of the wing (correspond to the span direction)
    // @Details: If a symmetry plan is set, the major direction is normal to the symmetry plan,
    // otherwise, an heuristic is used to find out the best span axis candidate.
    TiglAxis GetMajorDirection() const;

    // Returns the deep direction of the wing
    TiglAxis GetDeepDirection() const;

    TiglAxis GetThirdDirection() const;

    // Returns the uid of the element that is the most distant of the root in the major wing direction
    std::string GetTipUID() const;

    // Returns the uid of the root element (the element that start to build the wing)
    std::string GetRootUID() const;

    inline std::vector<std::string> GetElementUIDsInOrder() const
    {
        return elementUIDs;
    };

protected:
    // Init for the tipCElement cache
    void SetTipUid(std::string& cache) const;

    std::map<std::string, CTiglWingSectionElement*> BuildCTiglElementsMap();

private:
    CCPACSWing* wing;
    std::vector<std::string> elementUIDs;
    std::map<std::string, CTiglWingSectionElement*> cTiglElementsMap;
    Cache<std::string, CTiglWingHelper> tipUidCache;
};
}

#endif //TIGL_CTIGLWINGHELPER_H
