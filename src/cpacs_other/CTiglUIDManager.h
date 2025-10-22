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

#ifndef CTIGLUIDMANAGER_H
#define CTIGLUIDMANAGER_H

#include <typeinfo>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include "tigl_internal.h"
#include "CTiglError.h"
#include "CTiglRelativelyPositionedComponent.h"
#include "CTiglUIDObject.h"


namespace tigl
{
typedef std::map<const std::string, ITiglGeometricComponent*> ShapeContainerType;
typedef std::map<const std::string, CTiglRelativelyPositionedComponent*> RelativeComponentContainerType;

class ITiglUIDRefObject;

class CTiglUIDManager
{
public:
    struct TypedPtr {
        TypedPtr(void* ptr, const std::type_info* type)
            : ptr(ptr), type(type) {}

        void* ptr;
        const std::type_info* type;
    };

public:
    // Constructor
    TIGL_EXPORT CTiglUIDManager();

    TIGL_EXPORT bool IsUIDRegistered(const std::string& uid) const;
    TIGL_EXPORT bool IsUIDRegistered(const std::string& uid, const std::type_info& typeInfo) const;

    template <typename T>
    bool IsUIDRegistered(const std::string& uid) const
    {
        return IsUIDRegistered(uid, typeid(T));
    }

    template <typename T>
    bool IsType(const std::string& uid) const
    {
        return ResolveObject(uid).type == &typeid(T);
    }

    template<typename T>
    void RegisterObject(const std::string& uid, T& object)
    {
        RegisterObject(uid, &object, typeid(object)); // typeid(T) may yield a base class of object
        if (ITiglGeometricComponent* p = dynamic_cast<ITiglGeometricComponent*>(&object)) {
            AddGeometricComponent(uid, p);
        }
    }

    TIGL_EXPORT TypedPtr ResolveObject(const std::string& uid) const;
    TIGL_EXPORT TypedPtr ResolveObject(const std::string& uid, const std::type_info& typeInfo) const;

    template<typename T>
    T& ResolveObject(const std::string& uid) const
    {
        return *static_cast<T* const>(ResolveObject(uid, typeid(T)).ptr);
    }

    template<typename T>
    std::vector<T*> ResolveObjects() const
    {
        const std::type_info* ti = &typeid(T);
        std::vector<T*> objects;
        for (CPACSObjectMap::const_iterator it = cpacsObjects.begin(); it != cpacsObjects.end(); ++it)
            if (it->second.type == ti) {
                objects.push_back(static_cast<T* const>(it->second.ptr));
            }
        return objects;
    }

    TIGL_EXPORT bool TryUnregisterObject(const std::string& uid); // returns false on failure
    TIGL_EXPORT void UnregisterObject(const std::string& uid); // throws on failure

    TIGL_EXPORT void UpdateObjectUID(const std::string& oldUID, const std::string& newUID);

    template <typename T>
    void RegisterReference(const std::string& uid, T& object)
    {
        RegisterReference(uid, &object);
    }

    template <typename T>
    bool TryUnregisterReference(const std::string& uid, T& object)
    {
        return TryUnregisterReference(uid, &object);
    }

    // Checks if a UID is referenced
    TIGL_EXPORT bool IsReferenced(const std::string& uid) const;

    // Returns all objects referencing the passed uid
    TIGL_EXPORT std::set<const CTiglUIDObject*> GetReferences(const std::string& uid) const;

    // Checks if a UID already exists.
    TIGL_EXPORT bool HasGeometricComponent(const std::string& uid) const;

    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT ITiglGeometricComponent& GetGeometricComponent(const std::string& uid) const;

    // Returns the parent component for a component or a null pointer if there is no parent.
    TIGL_EXPORT CTiglRelativelyPositionedComponent* GetParentGeometricComponent(const std::string& uid) const;

    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT CTiglRelativelyPositionedComponent& GetRelativeComponent(const std::string& uid) const;

    // Returns the container with all root components of the geometric topology that have children.
    TIGL_EXPORT const RelativeComponentContainerType& GetRootGeometricComponents() const;

    // Resolves all CTiglRelativelyPositionedComponent's parentUid to the actual objects
    TIGL_EXPORT void SetParentComponents();

    // Returns the container with all registered shapes
    TIGL_EXPORT const ShapeContainerType& GetShapeContainer() const;

    // Clears the uid store
    TIGL_EXPORT void Clear();

    // Create a unique uid base on the the given UID.
    // If the given UID is not present in the file, the function returns the given UID without any change.
    // Otherwise, the function adds a prefix to make the UID unique and returns it
    TIGL_EXPORT std::string MakeUIDUnique(const std::string& baseUID) const;

private:
    // Function to add a UID and a geometric component to the uid store.
    void AddGeometricComponent(const std::string& uid, ITiglGeometricComponent* componentPtr);

    void RegisterObject(const std::string& uid, void* object, const std::type_info& typeInfo);

    void RegisterReference(const std::string& targetUid, ITiglUIDRefObject* source);

    bool TryUnregisterReference(const std::string& targetUid, ITiglUIDRefObject* source);

    // Removes a component from the UID Manager
    bool TryRemoveGeometricComponent(const std::string& uid); // returns false on failure

    // Updates the uid of a component from the UID Manager
    bool TryUpdateGeometricComponentUID(const std::string& oldUID, const std::string& newUID);

    // Updates all uid references on UID change
    void UpdateUIDReferences(const std::string& oldUID, const std::string& newUID);

    // Update internal UID manager data.
    void Update();

    // Builds the parent child relationships and finds the root components
    void BuildTree();


private:
    typedef std::map<std::string, TypedPtr> CPACSObjectMap;
    typedef std::unordered_map<ITiglUIDRefObject*, int> UIDReferenceEntries;
    typedef std::map<std::string, UIDReferenceEntries> UIDReferenceMap;

private:
    // Copy constructor
    CTiglUIDManager(const CTiglUIDManager& );

    // Assignment operator
    void operator=(const CTiglUIDManager& );

    RelativeComponentContainerType      relativeComponents;             ///< All relative components of the configuration
    ShapeContainerType                  allShapes;                      ///< All components of the configuration
    CTiglRelativelyPositionedComponent* rootComponent;                  ///< Root component injected by configuration
    RelativeComponentContainerType      rootComponents;                 ///< All root components that have children
    CPACSObjectMap                      cpacsObjects;                   ///< All objects in CPACS which have a UID
    UIDReferenceMap                     uidReferences;                  ///< All references to a specific UID
    bool                                invalidated;                    ///< Internal state flag
};

} // end namespace tigl

#endif // CTIGLUIDMANAGER_H
