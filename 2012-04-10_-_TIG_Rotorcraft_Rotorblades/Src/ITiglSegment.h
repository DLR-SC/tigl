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
* @brief  Interface of CPACS wing/fuselage segment handling routines.
*/

#ifndef ITIGLSEGMENT_H
#define ITIGLSEGMENT_H

#include <string>


namespace tigl {

	class ITiglSegment
	{

	public:
		// Gets the loft between the two segment sections
		virtual TopoDS_Shape GetLoft(void) = 0;

        // Gets the uid of this segment
        virtual const std::string & GetUID(void) = 0;

	};  // end class ITiglSegment

} // end namespace tigl

#endif // ITIGLSEGMENT_H
