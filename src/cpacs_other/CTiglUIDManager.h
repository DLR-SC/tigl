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
        void* ptr;
        const std::type_info* type;
    };

public:
    // Constructor
    TIGL_EXPORT CTiglUIDManager();

    template<typename T>
    TIGL_EXPORT void RegisterObject(const std::string& uid, T& object) {
        if (uid.empty()) {
            throw CTiglError("Tried to register an empty uid for type " + std::string(typeid(T).name()));
        }

        // check existence
        const CPACSObjectMap::iterator it = cpacsObjects.find(uid);
        if (it != std::end(cpacsObjects)) {
            throw CTiglError("Tried to register uid " + uid + " for type " + std::string(typeid(T).name()) + " which is already registered to an instance of " + std::string(it->second.type->name()));
        }

        // insert
        cpacsObjects.insert(it, std::make_pair(uid, TypedPtr{
            &object,
            &typeid(T)
        }));
    }

    TIGL_EXPORT TypedPtr ResolveObject(const std::string& uid) const;

    template<typename T>
    TIGL_EXPORT T& ResolveObject(const std::string& uid) const {
        const TypedPtr object = ResolveObject(uid);

        // check type
        if (&typeid(T) != object.type) {
            throw CTiglError("Object with uid \"" + uid + "\" is not a " + std::string(typeid(T).name()) + " but a " + std::string(object.type->name()));
        }

        // cast and return
        return *static_cast<T* const>(object.ptr);
    }

    template<typename T>
    TIGL_EXPORT std::vector<T*> ResolveObjects() const {
        const std::type_info* ti = &typeid(T);
        std::vector<T*> objects;
        for (const auto& c : cpacsObjects)
            if (c.second.type == ti)
                objects.push_back(static_cast<T* const>(c.second.ptr));
        return objects;
    }

    TIGL_EXPORT void UnregisterObject(const std::string& uid);

    // Function to add a UID and a geometric component to the uid store.
    TIGL_EXPORT void AddGeometricComponent(const std::string& uid, ITiglGeometricComponent* componentPtr);

    // Checks if a UID already exists.
    TIGL_EXPORT bool HasGeometricComponent(const std::string& uid) const;

    // Returns a pointer to the geometric component for the given unique id.
    TIGL_EXPORT ITiglGeometricComponent& GetGeometricComponent(const std::string& uid) const;

    // Returns the parent component for a component or a null pointer if there is no parent.
    TIGL_EXPORT CTiglRelativelyPositionedComponent* GetParentGeometricComponent(const std::string& uid) const;

    // Returns the container with all root components of the geometric topology that have children.
    TIGL_EXPORT const RelativeComponentContainerType& GetRootGeometricComponents() const;

    // Returns the container with all registered shapes
    TIGL_EXPORT const ShapeContainerType& GetShapeContainer() const;

    // Clears the uid store
    TIGL_EXPORT void Clear();

protected:
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
