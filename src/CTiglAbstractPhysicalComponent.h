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
* @brief  Definition of the interface which describes a geometric component.
*/

#ifndef CTIGLABSTRACTPHYISICALCOMPONENT_H
#define CTIGLABSTRACTPHYISICALCOMPONENT_H

#include <map>
#include <list>
#include <string>

#include "tigl.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"

#include "TDF_Label.hxx"
#include "Handle_XCAFDoc_ShapeTool.hxx"


namespace tigl {

    class CTiglAbstractPhysicalComponent : public CTiglAbstractGeometricComponent
	{

    public:
        // Container type to store a components children
        typedef std::list<CTiglAbstractPhysicalComponent*> ChildContainerType;

    protected:
        // Define a std::map to store the indices of already fused segments
        typedef std::map<int, int> FusedElementsContainerType;

    public:
        CTiglAbstractPhysicalComponent();

        // Returns the parent unique id
        virtual std::string& GetParentUID(void);

        // Sets the parent uid.
        virtual void SetParentUID(const std::string& parentUID);

        // Adds a child to this geometric component.
        virtual void AddChild(CTiglAbstractPhysicalComponent* componentPtr);

        // Returns a pointer to the list of children of a component.
        virtual ChildContainerType& GetChildren(void);

        // number of segments
        virtual int GetSegmentCount(void) const = 0;

        // builds data structure for a TDocStd_Application
        // mostly used for export
        virtual TDF_Label ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label) = 0;

		// Returns the segment for a given index
        virtual class CTiglAbstractSegment & GetSegment(const int index) = 0;

        virtual void SetSymmetryAxis(const std::string& axis);

    protected:
        // Resets the geometric component.
        virtual void Reset(void);

        ChildContainerType childContainer;
        std::string        parentUID;       /**< UID of the parent of this component */

	};

} // end namespace tigl

#endif // CTIGLABSTRACTPHYISICALCOMPONENT_H
