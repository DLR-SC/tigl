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

#include "CTiglUIDManager.h"
#include "CTiglError.h"

namespace tigl 
{

// Constructor
CTiglUIDManager::CTiglUIDManager(void) : 
    invalidated(true),
    rootComponent(0),
    rootComponentCnt(0)
{
}

// Destructor
CTiglUIDManager::~CTiglUIDManager(void)
{
    Clear();
}

// Update internal UID manager data.
void CTiglUIDManager::Update(void)
{
    if (!invalidated) {
        return;
    }
    
    BuildParentChildTree();
    FindRootComponents();
    invalidated = false;

    if (rootComponentCnt == 0) {
        throw CTiglError("Error: No root component found in CTiglUIDManager::FindRootComponents", TIGL_ERROR);
    }
    if (rootComponentCnt > 1) {
        throw CTiglError("Error: More than one root component found in CTiglUIDManager::FindRootComponents", TIGL_ERROR);
    }
}

// Function to add a UID and a geometric component to the uid store.
void CTiglUIDManager::AddUID(const std::string& uid, ITiglGeometricComponent* componentPtr)
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::AddUID", TIGL_XML_ERROR);
    }

    if (HasUID(uid)) {
        throw CTiglError("UID already exist in CTiglUIDManager::AddUID", TIGL_XML_ERROR);
    }

    if (componentPtr == 0) {
        throw CTiglError("Null pointer for component in CTiglUIDManager::AddUID", TIGL_NULL_POINTER);
    }

    CTiglAbstractPhysicalComponent* tmp = dynamic_cast<CTiglAbstractPhysicalComponent*>(componentPtr);
    if (tmp && (componentPtr->GetComponentType() & TIGL_COMPONENT_PHYSICAL) ) {
        physicalShapes[uid] = tmp;
    }
    allShapes[uid] = componentPtr;
    invalidated = true;
}

// Checks if a UID already exists.
bool CTiglUIDManager::HasUID(const std::string& uid) const
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::HasUID", TIGL_XML_ERROR);
    }

    return (allShapes.find(uid) != allShapes.end());
}

// Returns a pointer to the geometric component for the given unique id.
ITiglGeometricComponent* CTiglUIDManager::GetComponent(const std::string& uid)
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::GetComponent", TIGL_UID_ERROR);
    }

    if (!HasUID(uid)) {
        std::stringstream stream;
        stream << "UID " << uid << " not found in CTiglUIDManager::GetComponent";
        throw CTiglError(stream.str(), TIGL_UID_ERROR);
    }

    return allShapes[uid];
}

// Returns a pointer to the geometric component for the given unique id.
CTiglAbstractPhysicalComponent* CTiglUIDManager::GetPhysicalComponent(const std::string& uid)
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::GetComponent", TIGL_XML_ERROR);
    }

    if (physicalShapes.find(uid) == physicalShapes.end()) {
        throw CTiglError("UID not found in CTiglUIDManager::GetComponent", TIGL_XML_ERROR);
    }

    return physicalShapes[uid];
}


// Clears the uid store
void CTiglUIDManager::Clear(void) 
{
    physicalShapes.clear();
    allShapes.clear();
    allRootComponentsWithChildren.clear();
    rootComponent = 0;
    rootComponentCnt = 0;
    invalidated = true;
}

// Returns the parent component for a component or a null pointer
// if there is no parent.
CTiglAbstractPhysicalComponent* CTiglUIDManager::GetParentComponent(const std::string& uid)
{
    CTiglAbstractPhysicalComponent* component = GetPhysicalComponent(uid);
    std::string parentUID = component->GetParentUID();
    return (parentUID.empty() ? 0 : GetPhysicalComponent(parentUID));
}

// Returns the root component of the geometric topology.
CTiglAbstractPhysicalComponent* CTiglUIDManager::GetRootComponent(void)
{
    Update();
    return rootComponent;
}

// Returns the container with all root components of the geometric topology that have children.
const UIDStoreContainerType& CTiglUIDManager::GetAllRootComponentsWithChildren(void)
{
    Update();
    return allRootComponentsWithChildren;
}

// Finds and saves all root components and the main root component of the geometric topology.
void CTiglUIDManager::FindRootComponents(void)
{
    rootComponent = 0;
    rootComponentCnt = 0;
    int childCnt = 0;
    int maxChildCnt = -1;

    for (UIDStoreContainerType::iterator pIter = physicalShapes.begin(); pIter != physicalShapes.end(); ++pIter) {
        CTiglAbstractPhysicalComponent* component = pIter->second;
        if (component->GetParentUID().empty()) {
            // Select the component with the maximum number of children as root component if there are multiple components without parentUID in the dataset
            childCnt = component->GetChildren(true).size();
            if (childCnt > maxChildCnt) {
                maxChildCnt = childCnt;
                rootComponent = component;
            }
            if (childCnt > 0) {
	        allRootComponentsWithChildren[pIter->first] = component;
            }
            rootComponentCnt++;
        }
    }
}

// Builds the parent child relationships.
void CTiglUIDManager::BuildParentChildTree(void)
{
    UIDStoreContainerType::iterator pIter;

    for (pIter = physicalShapes.begin(); pIter != physicalShapes.end(); ++pIter) {
        CTiglAbstractPhysicalComponent* component = pIter->second;
        if (!component->GetParentUID().empty()) {
            CTiglAbstractPhysicalComponent* parent = GetPhysicalComponent(component->GetParentUID());
            parent->AddChild(component);
        }
    }
}

const ShapeContainerType& CTiglUIDManager::GetShapeContainer()
{
    return allShapes;
}

} // end namespace tigl

