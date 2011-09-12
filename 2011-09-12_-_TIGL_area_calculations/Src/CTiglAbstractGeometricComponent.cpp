/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#include "CTiglAbstractGeometricComponent.h"
#include "CTiglError.h"

namespace tigl {

    // Constructor
    CTiglAbstractGeometricComponent::CTiglAbstractGeometricComponent(void)
        : childContainer(),
        mySymmetryAxis(TIGL_NO_SYMMETRY)
    {
    }

    // Destructor
    CTiglAbstractGeometricComponent::~CTiglAbstractGeometricComponent(void)
    {
    }

    // Adds a child to this geometric component.
    void CTiglAbstractGeometricComponent::AddChild(ITiglGeometricComponent* componentPtr)
    {
        if (componentPtr == 0) {
            throw CTiglError("Error: Null pointer for component in CTiglAbstractGeometricComponent::AddChild", TIGL_NULL_POINTER);
        }

        childContainer.push_back(componentPtr);
    }

    // Returns a pointer to the list of children of a component.
    ITiglGeometricComponent::ChildContainerType& CTiglAbstractGeometricComponent::GetChildren(void)
    {
        return childContainer;
    }

    // Resets the geometric component.
    void CTiglAbstractGeometricComponent::Reset(void)
    {
        childContainer.clear();
        SetUID("");
        SetParentUID("");
    }

    // Gets the component uid
    std::string CTiglAbstractGeometricComponent::GetUID(void)
    {
        return myUID;
    }

    // Sets the component uid
    void CTiglAbstractGeometricComponent::SetUID(const std::string& uid)
    {
        myUID = uid;
    }

    // Returns the parent unique id
    std::string CTiglAbstractGeometricComponent::GetParentUID(void)
    {
        return parentUID;
    }

    // Sets the parent uid.
    void CTiglAbstractGeometricComponent::SetParentUID(const std::string& parentUID)
    {
        this->parentUID = parentUID;
    }

    // Gets symmetry axis
    TiglSymmetryAxis CTiglAbstractGeometricComponent::GetSymmetryAxis(void)
    {
        return mySymmetryAxis;
    }

    // Gets symmetry axis
    void CTiglAbstractGeometricComponent::SetSymmetryAxis(const std::string& axis)
    {
        if (axis == "x-z-plane") {
            mySymmetryAxis = TIGL_X_Z_PLANE;
        } else if (axis == "x-y-plane") {
            mySymmetryAxis = TIGL_X_Y_PLANE;
        } else if (axis == "y-z-plane") {
            mySymmetryAxis = TIGL_Y_Z_PLANE;
        } else {
            mySymmetryAxis = TIGL_NO_SYMMETRY;
        }
    }

} // end namespace tigl
