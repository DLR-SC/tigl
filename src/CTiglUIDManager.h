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

#include "CTiglAbstractPhysicalComponent.h"
#include <map>
#include <string>

namespace tigl {

    class CTiglUIDManager
    {

    private:
        typedef std::map<const std::string, CTiglAbstractPhysicalComponent*> UIDStoreContainerType;

    public:
        // Constructor
        CTiglUIDManager(void);

        // Function to add a UID and a geometric component to the uid store.
        void AddUID(const std::string& uid, CTiglAbstractPhysicalComponent* componentPtr);

        // Checks if a UID already exists. 
        bool HasUID(const std::string& uid) const;

        // Returns a pointer to the geometric component for the given unique id.
        CTiglAbstractPhysicalComponent* GetComponent(const std::string& uid);

        // Returns the parent component for a component or a null pointer
        // if there is no parent.
        CTiglAbstractPhysicalComponent* GetParentComponent(const std::string& uid);

        // Returns the root component of the geometric topology.
        CTiglAbstractPhysicalComponent* GetRootComponent(void);

        // Clears the uid store
        void Clear(void);

        // Virtual Destructor
        virtual ~CTiglUIDManager(void);

    protected:
           // Update internal UID manager data.
        void Update(void);

        // Returns the root component of the geometric topology.
        void FindRootComponent(void);

        // Builds the parent child relationships.
        void BuildParentChildTree(void);

    private:
        // Copy constructor
        CTiglUIDManager(const CTiglUIDManager& );

        // Assignment operator
        void operator=(const CTiglUIDManager& );

    private:
        UIDStoreContainerType           uidStore;
        bool                            invalidated;          /**< Internal state flag */
        CTiglAbstractPhysicalComponent* rootComponent;        /**< Ptr to the root component of the component tree */

    };

} // end namespace tigl

#endif // CTIGLUIDMANAGER_H
