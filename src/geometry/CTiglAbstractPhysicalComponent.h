/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Definition of the interface which describes a geometric component.
*/

#ifndef CTIGLABSTRACTPHYISICALCOMPONENT_H
#define CTIGLABSTRACTPHYISICALCOMPONENT_H

#include <map>
#include <list>
#include <string>

#include "tigl.h"
#include "tigl_internal.h"
#include "tigl_config.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"

namespace tigl
{

class CTiglAbstractPhysicalComponent : public CTiglAbstractGeometricComponent
{
public:
    // Container type to store a components children
    typedef std::list<CTiglAbstractPhysicalComponent*> ChildContainerType;

    TIGL_EXPORT CTiglAbstractPhysicalComponent(TiglSymmetryAxis* symmetryAxis = NULL);
    TIGL_EXPORT CTiglAbstractPhysicalComponent(boost::optional<TiglSymmetryAxis>* symmetryAxis);
    TIGL_EXPORT CTiglAbstractPhysicalComponent(CCPACSTransformation* trans, TiglSymmetryAxis* symmetryAxis = NULL);
    TIGL_EXPORT CTiglAbstractPhysicalComponent(CCPACSTransformation* trans, boost::optional<TiglSymmetryAxis>* symmetryAxis);

    // Returns the parent unique id
    TIGL_EXPORT virtual std::string& GetParentUID();

    // Sets the parent uid.
    TIGL_EXPORT virtual void SetParentUID(const std::string& parentUID);

    // Adds a child to this geometric component.
    TIGL_EXPORT virtual void AddChild(CTiglAbstractPhysicalComponent* componentPtr);

    // Returns a pointer to the list of children of a component.
    TIGL_EXPORT virtual ChildContainerType GetChildren(bool recursive);

protected:
    // Define a std::map to store the indices of already fused segments
    typedef std::map<int, int> FusedElementsContainerType;

    // Resets the geometric component.
    virtual void Reset();

    ChildContainerType childContainer;
    std::string        parentUID;       /**< UID of the parent of this component */

};

} // end namespace tigl

#endif // CTIGLABSTRACTPHYISICALCOMPONENT_H
