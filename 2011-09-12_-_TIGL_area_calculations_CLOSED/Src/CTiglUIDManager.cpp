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
* @brief  Implementation of the TIGL UID manager.
*/

#include "CTiglUIDManager.h"
#include "CTiglError.h"

namespace tigl {

    // Constructor
    CTiglUIDManager::CTiglUIDManager(void) : 
        invalidated(true),
        rootComponent(0)
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
        
        FindRootComponent();
        BuildParentChildTree();
        invalidated = false;
    }

    // Function to add a UID and a geometric component to the uid store.
    void CTiglUIDManager::AddUID(const std::string& uid, ITiglGeometricComponent* componentPtr)
    {
        if (uid.empty()) {
            throw CTiglError("Error: Empty UID in CTiglUIDManager::AddUID", TIGL_XML_ERROR);
        }

        if (HasUID(uid)) {
            throw CTiglError("Error: UID already exist in CTiglUIDManager::AddUID", TIGL_XML_ERROR);
        }

        if (componentPtr == 0) {
            throw CTiglError("Error: Null pointer for component in CTiglUIDManager::AddUID", TIGL_NULL_POINTER);
        }

        uidStore[uid] = componentPtr;
        invalidated = true;
    }

    // Checks if a UID already exists.
    bool CTiglUIDManager::HasUID(const std::string& uid) const
    {
        if (uid.empty()) {
            throw CTiglError("Error: Empty UID in CTiglUIDManager::HasUID", TIGL_XML_ERROR);
        }

        return (uidStore.find(uid) != uidStore.end());
    }

    // Returns a pointer to the geometric component for the given unique id.
    ITiglGeometricComponent* CTiglUIDManager::GetComponent(const std::string& uid)
    {
        if (uid.empty()) {
            throw CTiglError("Error: Empty UID in CTiglUIDManager::GetComponent", TIGL_XML_ERROR);
        }

        if (!HasUID(uid)) {
            throw CTiglError("Error: UID not found in CTiglUIDManager::GetComponent", TIGL_XML_ERROR);
        }

        return uidStore[uid];
    }


    // Clears the uid store
    void CTiglUIDManager::Clear(void) 
    {
        uidStore.clear();
        rootComponent = 0;
        invalidated = true;
    }

    // Returns the parent component for a component or a null pointer
    // if there is no parent.
    ITiglGeometricComponent* CTiglUIDManager::GetParentComponent(const std::string& uid)
    {
        ITiglGeometricComponent* component = GetComponent(uid);
        std::string parentUID = component->GetParentUID();

        // And now some Sackhaarcode :-D You know? Rotzevoll?
        return (parentUID.empty() ? 0 : GetComponent(parentUID));
    }

    // Returns the root component of the geometric topology.
    ITiglGeometricComponent* CTiglUIDManager::GetRootComponent(void)
    {
        Update();
        return rootComponent;
    }

    // Returns the root component of the geometric topology.
    void CTiglUIDManager::FindRootComponent(void)
    {
        rootComponent = 0;
        UIDStoreContainerType::iterator pIter;
        int parentCnt = 0;

        for (pIter = uidStore.begin(); pIter != uidStore.end(); pIter++) {
            ITiglGeometricComponent* component = pIter->second;
            if (component->GetParentUID().empty()) {
                if (parentCnt != 0) {
                    throw CTiglError("Error: More than one root component found in CTiglUIDManager::FindRootComponent", TIGL_ERROR);
                }
                parentCnt++;
                rootComponent = component;
            }
        }

        if (parentCnt == 0) {
            throw CTiglError("Error: No root component found in CTiglUIDManager::FindRootComponent", TIGL_ERROR);
        }

    }

    // Builds the parent child relationships.
    void CTiglUIDManager::BuildParentChildTree(void)
    {
        UIDStoreContainerType::iterator pIter;

        for (pIter = uidStore.begin(); pIter != uidStore.end(); pIter++) {
            ITiglGeometricComponent* component = pIter->second;
            if (!component->GetParentUID().empty()) {
                ITiglGeometricComponent* parent = GetComponent(component->GetParentUID());
                parent->AddChild(component);
            }
        }
    }

} // end namespace tigl

