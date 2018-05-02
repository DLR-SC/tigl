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
#include <string>
#include "tigl_internal.h"
#include "CTiglError.h"
#include "CTiglRelativelyPositionedComponent.h"

namespace tigl
{
typedef std::map<const std::string, ITiglGeometricComponent*> ShapeContainerType;
typedef std::map<const std::string, CTiglRelativelyPositionedComponent*> RelativeComponentContainerType;

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

    // Checks if a UID already exists.
    TIGL_EXPORT bool HasGeometricComponent(const std::string& uid) const;

    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT ITiglGeometricComponent& GetGeometricComponent(const std::string& uid) const;

    // Returns the parent component for a component or a null pointer if there is no parent.
    TIGL_EXPORT CTiglRelativelyPositionedComponent* GetParentGeometricComponent(const std::string& uid) const;

    // Returns the container with all root components of the geometric topology that have children.
    TIGL_EXPORT const RelativeComponentContainerType& GetRootGeometricComponents() const;

    // Resolves all CTiglRelativelyPositionedComponent's parentUid to the actual objects
    TIGL_EXPORT void SetParentComponents();

    // Returns the container with all registered shapes
    TIGL_EXPORT const ShapeContainerType& GetShapeContainer() const;

    // Clears the uid store
    TIGL_EXPORT void Clear();

private:
    // Function to add a UID and a geometric component to the uid store.
    void AddGeometricComponent(const std::string& uid, ITiglGeometricComponent* componentPtr);

    void RegisterObject(const std::string& uid, void* object, const std::type_info& typeInfo);

    // Removes a component from the UID Manager
    bool TryRemoveGeometricComponent(const std::string& uid); // returns false on failure

    // Update internal UID manager data.
    void Update();

    // Builds the parent child relationships and finds the root components
    void BuildTree();

    // Returns a pointer to the geometric component for the given unique id.
    CTiglRelativelyPositionedComponent& GetRelativeComponent(const std::string& uid) const;

private:
    typedef std::map<std::string, TypedPtr> CPACSObjectMap;

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
    bool                                invalidated;                    ///< Internal state flag
};

} // end namespace tigl

#endif // CTIGLUIDMANAGER_H
