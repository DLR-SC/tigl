/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>

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
#include "typename.h"
#include "ITiglUIDRefObject.h"


namespace tigl
{

// Constructor
CTiglUIDManager::CTiglUIDManager()
    : rootComponent(NULL), invalidated(true)
{}

bool CTiglUIDManager::IsUIDRegistered(const std::string & uid) const
{
    return cpacsObjects.find(uid) != cpacsObjects.end();
}

bool CTiglUIDManager::IsUIDRegistered(const std::string& uid, const std::type_info& typeInfo) const
{
    CPACSObjectMap::const_iterator it = cpacsObjects.find(uid);
    return it != cpacsObjects.end() && it->second.type == &typeInfo;
}

void CTiglUIDManager::RegisterObject(const std::string& uid, void* object, const std::type_info& typeInfo)
{
    if (uid.empty()) {
        throw CTiglError("Tried to register an empty uid for type " + typeName(typeInfo));
    }

    // check existence
    const CPACSObjectMap::iterator it = cpacsObjects.find(uid);
    if (it != cpacsObjects.end()) {
        throw CTiglError("Tried to register uid " + uid + " for type " + typeName(typeInfo) + " which is already registered to an instance of " + std::string(it->second.type->name()));
    }

    // insert
    cpacsObjects.insert(it, std::make_pair(uid, TypedPtr(
                                               object,
                                               &typeInfo
                                           )));
}

void CTiglUIDManager::UpdateObjectUID(const std::string& oldUID, const std::string& newUID)
{
    if (oldUID.empty() || newUID.empty()) {
        throw CTiglError("Tried to update an empty uid");
    }
    // check existance
    CPACSObjectMap::iterator it = cpacsObjects.find(oldUID);
    if (it == cpacsObjects.end()) {
        throw CTiglError("Tried to update the uid of an object which was not registered, from uid " + oldUID + " to uid " + newUID);
    }
    // ensure that new uid does not exist
    const CPACSObjectMap::iterator it2 = cpacsObjects.find(newUID);
    if (it2 != cpacsObjects.end()) {
        throw CTiglError("Tried to update the uid of object " + oldUID + " to the new uid " + newUID + " which is already registered to an instance of " + std::string(it2->second.type->name()));
    }
    // insert entry with new UID
    cpacsObjects.insert(it2, std::make_pair(newUID, it->second));
    // erase old entry
    cpacsObjects.erase(it);

    TryUpdateGeometricComponentUID(oldUID, newUID);

    UpdateUIDReferences(oldUID, newUID);
}


void CTiglUIDManager::RegisterReference(const std::string& targetUid, ITiglUIDRefObject* source)
{
    if (!source || !source->GetNextUIDObject()) {
        LOG(ERROR) << "Tried to register a reference to uid " << targetUid << " from non-UID object (nullptr passed)";
        return;
    }

    // ignore references to empty string
    if (targetUid.empty()) {
        return;
    }
    // insert
    uidReferences[targetUid][source]++;
}

bool CTiglUIDManager::TryUnregisterReference(const std::string& targetUid, ITiglUIDRefObject* source)
{
    // find all registerd references to uid
    UIDReferenceMap::iterator it = uidReferences.find(targetUid);
    if (it == uidReferences.end()) {
        return false;
    }

    // find reference from object to uid
    UIDReferenceEntries& referencingObjects = it->second;
    UIDReferenceEntries::iterator refIt = referencingObjects.find(source);
    if (refIt != referencingObjects.end()) {
        int& refCount = refIt->second;
        refCount--;
        if (refCount <= 0) {
            referencingObjects.erase(refIt);
            if (referencingObjects.empty()) {
                uidReferences.erase(it);
            }
        }
        return true;
    }
    return false;
}

CTiglUIDManager::TypedPtr CTiglUIDManager::ResolveObject(const std::string& uid, const std::type_info& typeInfo) const
{
    const TypedPtr object = ResolveObject(uid);

    // check type
    if (&typeInfo != object.type) {
        throw CTiglError("Object with uid \"" + uid + "\" is not a " + typeName(typeInfo) + " but a " + typeName(*object.type), TIGL_UID_ERROR);
    }

    return object;
}

CTiglUIDManager::TypedPtr CTiglUIDManager::ResolveObject(const std::string& uid) const
{
    // check existence
    const CPACSObjectMap::const_iterator it = cpacsObjects.find(uid);
    if (it == cpacsObjects.end()) {
        throw CTiglError("No object is registered for uid \"" + uid + "\"", TIGL_UID_ERROR);
    }
    return it->second;
}


namespace {
    bool isChildOf(const tigl::CTiglUIDObject* obj, const std::string& parentUID)
    {
        const tigl::CTiglUIDObject* parentObj = obj->GetNextUIDParent();
        while (parentObj && parentObj->GetObjectUID() != parentUID) {
            parentObj = parentObj->GetNextUIDParent();
        }
        return (parentObj != nullptr);
    }
}

bool CTiglUIDManager::TryUnregisterObject(const std::string& uid)
{
    const CPACSObjectMap::iterator it = cpacsObjects.find(uid);
    if (it == cpacsObjects.end()) {
        return false;
    }
    cpacsObjects.erase(it);

    // remove all references to object
    if (uidReferences.find(uid) != uidReferences.end()) {
        uidReferences.erase(uid);
    }
    // remove all references from object
    for (UIDReferenceMap::iterator it = uidReferences.begin(); it != uidReferences.end();) {
        UIDReferenceEntries& entries = it->second;
        for (UIDReferenceEntries::iterator refIt = it->second.begin(); refIt != it->second.end();) {
            const CTiglUIDObject* refObj = refIt->first->GetNextUIDObject();
            if (refObj->GetObjectUID() == uid || isChildOf(refObj, uid)) {
                refIt = entries.erase(refIt);
            }
            else {
                ++refIt;
            }
        }
        if (entries.empty()) {
            it = uidReferences.erase(it);
        }
        else {
            ++it;
        }
    }

    // also remove the geometric component if it exists
    TryRemoveGeometricComponent(uid);

    return true;
}

void CTiglUIDManager::UnregisterObject(const std::string& uid)
{
    if (!TryUnregisterObject(uid)) {
        throw CTiglError("No object is registered for uid \"" + uid + "\"", TIGL_UID_ERROR);
    }
}

namespace
{
void writeComponent(CTiglRelativelyPositionedComponent* c, int level = 0)
{
    std::string indentation;
    for (int i = 0; i < level; i++) {
        indentation += '\t';
    }
    const std::string uid = c->GetDefaultedUID();
    LOG(INFO) << indentation << (uid.empty() ? "<no uid>" : uid) << std::endl;
    const CTiglRelativelyPositionedComponent::ChildContainerType& children = c->GetChildren(false);
    for (CTiglRelativelyPositionedComponent::ChildContainerType::const_iterator it = children.begin(); it != children.end(); ++it) {
        writeComponent(*it, level + 1);
    }
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
    for (RelativeComponentContainerType::const_iterator it = rootComponents.begin(); it != rootComponents.end(); ++it) {
        writeComponent(it->second);
    }
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
    if (tmp && (componentPtr->GetComponentIntent() & TIGL_INTENT_PHYSICAL) ) {
        relativeComponents[uid] = tmp;
    }
    allShapes[uid] = componentPtr;
    invalidated = true;
}

bool CTiglUIDManager::TryRemoveGeometricComponent(const std::string & uid)
{
    const ShapeContainerType::iterator it = allShapes.find(uid);
    if (it == allShapes.end()) {
        return false;
    }
    allShapes.erase(it);

    const RelativeComponentContainerType::iterator it2 = relativeComponents.find(uid);
    if (it2 != relativeComponents.end()) {
        relativeComponents.erase(it2);
    }

    return true;
}

bool CTiglUIDManager::TryUpdateGeometricComponentUID(const std::string& oldUID, const std::string& newUID)
{
    ShapeContainerType::iterator it = allShapes.find(oldUID);
    if (it == allShapes.end()) {
        return false;
    }
    // insert entry with new UID
    allShapes[newUID] = it->second;
    // erase old entry
    allShapes.erase(it);

    RelativeComponentContainerType::iterator it2 = relativeComponents.find(oldUID);
    if (it2 != relativeComponents.end()) {
        // insert entry with new UID
        relativeComponents[newUID] = it2->second;
        // erase old entry
        relativeComponents.erase(it2);
    }
    return true;
}

void CTiglUIDManager::UpdateUIDReferences(const std::string& oldUID, const std::string& newUID)
{
    // fix target uid
    auto it = uidReferences.find(oldUID);
    if (it != uidReferences.end()) {
        uidReferences[newUID] = it->second;
        // notify all targets about the change
        for (auto ref : it->second) {
            ref.first->NotifyUIDChange(oldUID, newUID);
        }
        uidReferences.erase(it);
    }
}

// Checks if a UID is referenced.
bool CTiglUIDManager::IsReferenced(const std::string& uid) const
{
    if (uid.empty()) {
        throw CTiglError("Empty UID in CTiglUIDManager::IsReferenced", TIGL_XML_ERROR);
    }
    return (uidReferences.find(uid) != uidReferences.end());
}

std::set<const CTiglUIDObject*> CTiglUIDManager::GetReferences(const std::string& uid) const
{
    std::set<const CTiglUIDObject*> references;
    if (IsReferenced(uid)) {
        // find pointers in list of registered uid objects
        for (const auto& ref : uidReferences.at(uid)) {
            references.insert(ref.first->GetNextUIDObject());
        }
    }
    return references;
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
        throw CTiglError("Empty UID in CTiglUIDManager::GetGeometricComponent", TIGL_UID_ERROR);
    }

    const RelativeComponentContainerType::const_iterator it = relativeComponents.find(uid);
    if (it == relativeComponents.end()) {
        throw CTiglError("UID '"+uid+"' not found in CTiglUIDManager::GetGeometricComponent", TIGL_UID_ERROR);
    }

    return *it->second;
}

// Clears the uid store
void CTiglUIDManager::Clear()
{
    relativeComponents.clear();
    allShapes.clear();
    rootComponents.clear();
    cpacsObjects.clear();
    invalidated = true;
}

// Returns the parent component for a component or a null pointer
// if there is no parent.
CTiglRelativelyPositionedComponent* CTiglUIDManager::GetParentGeometricComponent(const std::string& uid) const
{
    CTiglRelativelyPositionedComponent& component = GetRelativeComponent(uid);
    const boost::optional<const std::string&> parentUID = component.GetParentUID();
    return parentUID? &GetRelativeComponent(*parentUID) : NULL;
}

// Returns the container with all root components of the geometric topology that have children.
const RelativeComponentContainerType& CTiglUIDManager::GetRootGeometricComponents() const
{
    const_cast<CTiglUIDManager&>(*this).Update(); // TODO(bgruber): hack to keep up logical constness, think about mutable members
    return rootComponents;
}

void CTiglUIDManager::SetParentComponents()
{
    BuildTree();
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
            if (parentUid->empty()) {
                throw CTiglError("geometric component with uid " + c.GetDefaultedUID() + " has empty parentUid");
            }

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

std::string CTiglUIDManager::MakeUIDUnique(const std::string& baseUID) const
{
    std::string newUID = baseUID;
    int suffix         = 1;
    while (IsUIDRegistered(newUID)) {
        newUID = baseUID + "U" +std::to_string(suffix);
        suffix = suffix + 1;
    }
    return newUID;
}

} // end namespace tigl

