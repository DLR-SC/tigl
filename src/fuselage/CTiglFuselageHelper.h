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

#ifndef TIGL_CTIGLFUSELAGEHELPER_H
#define TIGL_CTIGLFUSELAGEHELPER_H

#include "CTiglFuselageSectionElement.h"

#include <string>
#include <vector>
#include <map>

namespace tigl
{

class CCPACSFuselage;

/**
 * This class help the CCPACSFuselage class.
 * It's main goal is to keep the CCPACSFuselage class clean, such that the CPACSFuselage has its interface simple
 *
 */
class CTiglFuselageHelper
{

public:
    CTiglFuselageHelper();

    CTiglFuselageHelper(CCPACSFuselage* fuselage);

    void SetFuselage(CCPACSFuselage* fuselage);

    // Returns true if the fuselage has a shape
    // This mean that a list the fuselage is composed by a valid segment and thus is composed by two elements
    bool HasShape() const;

    // return the element uid that is considered as the noise element
    std::string GetNoseUID() const;

    // return the element uid that is considered as the tail element
    std::string GetTailUID() const;

    /**
     * Return the uid of the elements connected by some segments.
     * @remark if a element in the fuselage is not connected, then it will not appears in this list.
     * @return
     */
    inline std::vector<std::string> GetElementUIDsInOrder() const
    {
        return elementUIDs;
    };

    CTiglFuselageSectionElement* GetCTiglElementOfFuselage(std::string elementUID) const;

protected:
    std::map<std::string, CTiglFuselageSectionElement*> BuildCTiglElementsMap();

private:
    CCPACSFuselage* fuselage;
    std::vector<std::string> elementUIDs; // element that compose the fuselage in order
    std::map<std::string, CTiglFuselageSectionElement*> cTiglElementsMap;
};
}

#endif //TIGL_CTIGLFUSELAGEHELPER_H
