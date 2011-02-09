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

#ifndef CTIGLABSTRACTGEOMETRICCOMPONENT_H
#define CTIGLABSTRACTGEOMETRICCOMPONENT_H

#include <list>
#include <string>

#include "ITiglGeometricComponent.h"


namespace tigl {

    class CTiglAbstractGeometricComponent : public ITiglGeometricComponent
	{

	public:
        // Constructor
        CTiglAbstractGeometricComponent(void);

        // Virtual Destructor
		virtual ~CTiglAbstractGeometricComponent(void);

   		// Gets the loft of a geometric component
		virtual TopoDS_Shape& GetLoft(void) = 0;

        // Gets the component uid
        virtual std::string GetUID(void);

        // Sets the component uid
        virtual void SetUID(const std::string& uid);

        // Returns the parent unique id
        virtual std::string GetParentUID(void);

        // Sets the parent uid.
        virtual void SetParentUID(const std::string& parentUID);

        // Adds a child to this geometric component.
        virtual void AddChild(ITiglGeometricComponent* componentPtr);

        // Returns a pointer to the list of children of a component.
        virtual ChildContainerType& GetChildren(void);

		// Gets symmetry axis
		virtual TiglSymmetryAxis GetSymmetryAxis(void);

		// Sets symmetry axis
		virtual void SetSymmetryAxis(const std::string& axis);

    protected:
        // Resets the geometric component.
        virtual void Reset(void);

    private:
		// Copy constructor
		CTiglAbstractGeometricComponent(const CTiglAbstractGeometricComponent& ) { /* Do nothing */ }

		// Assignment operator
		void operator=(const CTiglAbstractGeometricComponent& ) { /* Do nothing */ }

    private:
        ChildContainerType childContainer;
        std::string        myUID;           /**< UID of this component               */
        std::string        parentUID;       /**< UID of the parent of this component */
        TiglSymmetryAxis   mySymmetryAxis;  /**< SymmetryAxis of this component      */

	};

} // end namespace tigl

#endif // CTIGLABSTRACTGEOMETRICCOMPONENT_H
