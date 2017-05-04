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
#include "CTiglLogging.h"
#include "to_string.h"

namespace tigl 
{

// Constructor
CTiglUIDManager::CTiglUIDManager()
    : invalidated(true), rootComponent(NULL) {}

namespace {
    void writeComponent(CTiglRelativelyPositionedComponent* c, int level = 0) {
        std::string indentation;
        for (int i = 0; i < level; i++)
            indentation += '\t';
        LOG(INFO) << indentation << c->GetUID() << std::endl;
        const CTiglRelativelyPositionedComponent::ChildContainerType& children = c->GetChildren(false);
        for (CTiglRelativelyPositionedComponent::ChildContainerType::const_iterator it = children.begin(); it != children.end(); ++it)
            writeComponent(*it, level + 1);
    }
}

// Update internal UID manager data.
void CTiglUIDManager::Update()
{
    if (!invalidated) {
        return;
    }
    
    BuildTree();
    invalidated = false;

    LOG(INFO) << "Relative component trees:" << std::endl;
    for (RelativeComponentContainerType::const_iterator it = rootComponents.begin(); it != rootComponents.end(); ++it)
        writeComponent(it->second);
}

// Function to add a UID and a geometric component to the uid store.
void CTiglUIDManager::AddGeometricComponent(const std::string& uid, ITiglGeometricComponent* componentPtr)
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::AddGeometricComponent", TIGL_XML_ERROR);
    }

    if (HasGeometricComponent(uid)) {
        throw CTiglError("Duplicate UID " + uid + " in CPACS file (CTiglUIDManager::AddGeometricComponent)", TIGL_XML_ERROR);
    }

    if (componentPtr == NULL) {
        throw CTiglError("Null pointer for component in CTiglUIDManager::AddGeometricComponent", TIGL_NULL_POINTER);
    }

    CTiglRelativelyPositionedComponent* tmp = dynamic_cast<CTiglRelativelyPositionedComponent*>(componentPtr);
    if (tmp && (componentPtr->GetComponentType() & TIGL_COMPONENT_PHYSICAL) ) {
        relativeComponents[uid] = tmp;
    }
    allShapes[uid] = componentPtr;
    invalidated = true;
}

// Checks if a UID already exists.
bool CTiglUIDManager::HasGeometricComponent(const std::string& uid) const
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::HasGeometricComponent", TIGL_XML_ERROR);
    }

    return (allShapes.find(uid) != allShapes.end());
}

// Returns a pointer to the geometric component for the given unique id.
ITiglGeometricComponent& CTiglUIDManager::GetGeometricComponent(const std::string& uid) const
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::GetGeometricComponent", TIGL_UID_ERROR);
    }

    if (!HasGeometricComponent(uid)) {
        throw CTiglError("UID " + std_to_string(uid) + " not found in CTiglUIDManager::GetGeometricComponent", TIGL_UID_ERROR);
    }

    return *allShapes.find(uid)->second;
}

// Returns a pointer to the geometric component for the given unique id.
CTiglRelativelyPositionedComponent& CTiglUIDManager::GetRelativeComponent(const std::string& uid) const
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::GetGeometricComponent", TIGL_XML_ERROR);
    }

    const RelativeComponentContainerType::const_iterator it = relativeComponents.find(uid);
    if (it == relativeComponents.end()) {
        throw CTiglError("UID '"+uid+"' not found in CTiglUIDManager::GetGeometricComponent", TIGL_XML_ERROR);
    }

    return *it->second;
}


// Clears the uid store
void CTiglUIDManager::Clear()
{
    relativeComponents.clear();
    allShapes.clear();
    rootComponents.clear();
    invalidated = true;
}

// Returns the parent component for a component or a null pointer
// if there is no parent.
CTiglRelativelyPositionedComponent* CTiglUIDManager::GetParentGeometricComponent(const std::string& uid) const
{
    CTiglRelativelyPositionedComponent& component = GetRelativeComponent(uid);
    const boost::optional<const std::string&> parentUID = component.GetParentUID();
    return parentUID ? NULL : &GetRelativeComponent(*parentUID);
}

// Returns the container with all root components of the geometric topology that have children.
const RelativeComponentContainerType& CTiglUIDManager::GetRootGeometricComponents() const
{
    const_cast<CTiglUIDManager&>(*this).Update(); // TODO(bgruber): hack to keep up logical constness, think about mutable members
    return rootComponents;
}

// Builds the parent child relationships.
void CTiglUIDManager::BuildTree()
{
    // clear all relations
    for (RelativeComponentContainerType::iterator it = relativeComponents.begin(); it != relativeComponents.end(); ++it) {
        it->second->ClearChildren();
    }

    // build relations
    for (RelativeComponentContainerType::iterator it = relativeComponents.begin(); it != relativeComponents.end(); ++it) {
        CTiglRelativelyPositionedComponent& c = *it->second;
        const boost::optional<const std::string&> parentUid = c.GetParentUID();
        if (parentUid) {
            CTiglRelativelyPositionedComponent& p = GetRelativeComponent(*parentUid);
            p.AddChild(c);
            c.SetParent(p);
        }
    }

    // find all components without a parent uid and find component with most children
    std::size_t count = 0;
    for (RelativeComponentContainerType::iterator it = relativeComponents.begin(); it != relativeComponents.end(); ++it) {
        CTiglRelativelyPositionedComponent* c = it->second;
        if (!c->GetParentUID()) {
            rootComponents[it->first] = c;
        }
        const std::size_t childCount = c->GetChildren(true).size();
        if (childCount > count) {
            count = childCount;
            rootComponent = c;
        }
    }
}

const ShapeContainerType& CTiglUIDManager::GetShapeContainer() const
{
    return allShapes;
}

} // end namespace tigl

