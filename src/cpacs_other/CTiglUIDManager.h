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
* @brief  Implementation of the TIGL UID manager.
*/

#ifndef CTIGLUIDMANAGER_H
#define CTIGLUIDMANAGER_H

#include "tigl_internal.h"
#include "CTiglAbstractPhysicalComponent.h"
#include <map>
#include <string>

namespace tigl 
{

typedef std::map<const std::string, ITiglGeometricComponent*> ShapeContainerType;
typedef std::map<const std::string, CTiglAbstractPhysicalComponent*> UIDStoreContainerType;

class CTiglUIDManager
{
public:
    // Constructor
    TIGL_EXPORT CTiglUIDManager();

    // Function to add a UID and a geometric component to the uid store.
    TIGL_EXPORT void AddUID(const std::string& uid, ITiglGeometricComponent* componentPtr);

    // Checks if a UID already exists. 
    TIGL_EXPORT bool HasUID(const std::string& uid) const;

    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT ITiglGeometricComponent* GetComponent(const std::string& uid);

    // Returns the parent component for a component or a null pointer
    // if there is no parent.
    TIGL_EXPORT CTiglAbstractPhysicalComponent* GetParentComponent(const std::string& uid);

    // Returns the root component of the geometric topology.
    TIGL_EXPORT CTiglAbstractPhysicalComponent* GetRootComponent();

    // Returns the container with all root components of the geometric topology that have children.
    TIGL_EXPORT const UIDStoreContainerType& GetAllRootComponentsWithChildren();

    // Returns the contianer with all registered shapes
    TIGL_EXPORT const ShapeContainerType& GetShapeContainer();

    // Clears the uid store
    TIGL_EXPORT void Clear();

    // Sets the root component
    TIGL_EXPORT void SetRootComponent(CTiglAbstractPhysicalComponent* rootComponent);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglUIDManager();

protected:
    // Update internal UID manager data.
    void Update();

    // Returns the root component of the geometric topology.
    void FindRootComponents();

    // Builds the parent child relationships.
    void BuildParentChildTree();
    
    
    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT CTiglAbstractPhysicalComponent* GetPhysicalComponent(const std::string& uid);

private:

    // Copy constructor
    CTiglUIDManager(const CTiglUIDManager& );

    // Assignment operator
    void operator=(const CTiglUIDManager& );

    UIDStoreContainerType               physicalShapes;                 /**< All physical components of the configuration */
    ShapeContainerType                  allShapes;                      /**< All components of the configuration */
    UIDStoreContainerType               allRootComponentsWithChildren;  /**< All root components that have children */
    CTiglAbstractPhysicalComponent*     rootComponent;                  /**< Ptr to the root component of the component tree */
    int                                 rootComponentCnt;               /**< Number of root components found */
    bool                                invalidated;                    /**< Internal state flag */
};

} // end namespace tigl

#endif // CTIGLUIDMANAGER_H
