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
* @brief  Implementation of a TIGL point.
*/

#include "CTiglPoint.h"

namespace tigl {

	// Constructor
	CTiglPoint::CTiglPoint(double xval, double yval, double zval)
		: x(xval)
		, y(yval)
		, z(zval)
	{
	}

	// Copy constructor
	CTiglPoint::CTiglPoint(const CTiglPoint& aPoint)
	{
		x = aPoint.x;
		y = aPoint.y;
		z = aPoint.z;
	}

	// Assignment operator
	CTiglPoint& CTiglPoint::operator=(const CTiglPoint& aPoint)
	{
		if (&aPoint != this) {
			x = aPoint.x;
			y = aPoint.y;
			z = aPoint.z;
		}
		return *this;
	}

	// Destructor
	CTiglPoint::~CTiglPoint(void)
	{
	}

	// Convert a CTiglPoint to a OpenCascade gp_Pnt
	gp_Pnt CTiglPoint::Get_gp_Pnt(void) const
	{
		return gp_Pnt(x, y, z);
	}

	// Dump internal point data
	void CTiglPoint::Dump(std::ostream& aStream) const 
	{
		aStream << "CTiglPoint: (" << x << ", " << y << ", " << z << ")";
	}

} // end namespace tigl
