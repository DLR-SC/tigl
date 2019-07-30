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


#ifndef TIGL_SECTIONCREATORINTERFACE_H
#define TIGL_SECTIONCREATORINTERFACE_H

#include "tigl_internal.h"
#include <string>
#include <vector>

namespace tigl
{

/**
* This class defines the interface (I) of a cpacs object that is capable to create a connected element.
* By connected element we mean a element that is correctly connected to another element by a segment.
* For the moment this interface is implemented by the the wing and fuselage classes.
*
*/
class CreateConnectedElementI
{

public:

    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "after" the start element.
     * If there is already a element after the start element, we split the existing segment and insert the new element
     * between the the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT virtual void CreateNewConnectedElementAfter(std::string startElementUID) = 0;

    /**
     * Create a new section, a new element and connect the element to the "startElement".
     * The new element is placed "Before" the start element.
     * If there is already a element before the start element, we split the existing segment and insert the new element
     * between the the two elements.
     *
     * @param startElementUID
     */
    TIGL_EXPORT virtual void CreateNewConnectedElementBefore(std::string startElementUID) = 0;

    /**
      *Create a new section, a new element and place the new element between the startElement and the endElement.
     * @remark The startElement and endElement must be connected by a segment.
     * @param startElementUID
     * @param endElementUID
     */
    TIGL_EXPORT virtual void CreateNewConnectedElementBetween(std::string startElementUID, std::string endElementUID) = 0;


    /**
     * Return the current connected element uids in order for this component (wing or fuselage)
     * @return
     */
    TIGL_EXPORT virtual  std::vector<std::string> GetOrderedConnectedElement() = 0;

};
}

#endif //TIGL_SECTIONCREATORINTERFACE_H
