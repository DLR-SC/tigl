/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#ifndef CCPACSIMPORTEXPORT_H
#define CCPACSIMPORTEXPORT_H

#include "stringtools.h"
#include "CTiglError.h"
#include <string>

namespace tigl
{

enum ShapeGroupMode
{
    WHOLE_SHAPE,           /** Inserts the shape as it is into IGES/STEP. All faces will be named correctly but they will not be grouped by name */
    NAMED_COMPOUNDS,       /** Collects all faces with the same origin into compounds. All faces are named correctly */
    FACES                  /** Exports each face as its own group. The group name and the face name are identical    */
};

template <>
inline void from_string<ShapeGroupMode>(const std::string &s, ShapeGroupMode &t)
{
    std::string value = tigl::to_upper(s);
    if (value == "WHOLE_SHAPE") {
        t = WHOLE_SHAPE;
    }
    else if (value == "NAMED_COMPOUNDS") {
        t = NAMED_COMPOUNDS;
    }
    else if (value == "FACES") {
        t = FACES;
    }
    else {
        throw CTiglError("Cannot convert string to ShapeGroupMode");
    }
}


} // end namespace tigl

#endif // CCPACSIMPORTEXPORT_H
