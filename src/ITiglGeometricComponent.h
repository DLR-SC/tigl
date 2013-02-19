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

#ifndef ITIGLGEOMETRICCOMPONENT_H
#define ITIGLGEOMETRICCOMPONENT_H

#include <map>
#include <list>
#include <string>

#include "tigl.h"
#include "CTiglTransformation.h"

#include "TDF_Label.hxx"

class TopoDS_Shape;


namespace tigl {

	class ITiglGeometricComponent
	{

	public:
   		// Gets the loft of a geometric component
		virtual TopoDS_Shape & GetLoft(void) = 0;

        // Returns the unique id of this component
        virtual const std::string& GetUID(void) = 0;

        // Sets the unique id of this component
        virtual void SetUID(const std::string&) = 0;

        // Get transformation object
        virtual CTiglTransformation GetTransformation(void) = 0;
		
		// Set transformation object
        virtual void Translate(CTiglPoint trans) = 0;

        // Get component translation
        virtual CTiglPoint GetTranslation(void) = 0;

		// Returns the Geometric type of this component, e.g. Wing or Fuselage
		virtual TiglGeometricComponentType GetComponentType(void) = 0;

        // builds data structure for a TDocStd_Application
        // mostly used for export
        virtual void ExportDataStructure(TDF_Label &rootLabel) = 0;

    protected:
        // Resets the geometric component.
        virtual void Reset(void) = 0;

	};

} // end namespace tigl

#endif // ITIGLGEOMETRICCOMPONENT_H
