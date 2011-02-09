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
* @brief  Definition of the interface which describes a geometric component.
*/

#ifndef ITIGLGEOMETRICCOMPONENT_H
#define ITIGLGEOMETRICCOMPONENT_H

#include <map>
#include <list>
#include <string>

#include "tigl.h"
#include "CTiglTransformation.h"
#include "ITiglSegment.h"

#include "TopoDS_Shape.hxx"


namespace tigl {

	class ITiglGeometricComponent
	{

    public:
        // Container type to store a components children
        typedef std::list<ITiglGeometricComponent*> ChildContainerType;

    protected:
        // Define a std::map to store the indices of already fused segments
        typedef std::map<int, int> FusedElementsContainerType;

	public:
   		// Gets the loft of a geometric component
		virtual TopoDS_Shape & GetLoft(void) = 0;

        // Returns the parent unique id
        virtual std::string GetParentUID(void) = 0;

        // Adds a child to this geometric component.
        virtual void AddChild(ITiglGeometricComponent* componentPtr) = 0;

        // Returns a pointer to the list of children of a component.
        virtual ChildContainerType & GetChildren(void) = 0;

        // Get transformation object
        virtual CTiglTransformation GetTransformation(void) = 0;

        // number of segments
        virtual int GetSegmentCount(void) = 0;

		// Returns the segment for a given index
		virtual ITiglSegment & GetSegment(const int index) = 0;

		// Returns the Geometric type of this component, e.g. Wing or Fuselage
		virtual TiglGeometricComponentType GetComponentType(void) = 0;

		// Returns a unique Hashcode for a specific geometric component
		virtual int GetComponentHashCode(void) = 0;

    protected:
        // Resets the geometric component.
        virtual void Reset(void) = 0;

	};

} // end namespace tigl

#endif // ITIGLGEOMETRICCOMPONENT_H
