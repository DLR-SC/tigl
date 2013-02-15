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
* @brief  Implementation of a TIGL point.
*/

#ifndef CTIGLPOINT_H
#define CTIGLPOINT_H

#include <climits>
#include "gp_Pnt.hxx"
#include <iostream>

namespace tigl {

	class CTiglPoint
	{

	public:
		// Constructor
		CTiglPoint(double xval = 0.0, double yval = 0.0, double zval = 0.0);

		// Copy constructor
		CTiglPoint(const CTiglPoint& aPoint);

		// Assignment operator
		CTiglPoint& operator=(const CTiglPoint& aPoint);

		// Addition of points
		CTiglPoint operator+(const CTiglPoint& aPoint) const;

		// Addition of a point
		CTiglPoint& operator+=(const CTiglPoint& aPoint);

		// Subtraction of points
		CTiglPoint operator-(const CTiglPoint& aPoint) const;

		// Subtraction of a point
		CTiglPoint& operator-=(const CTiglPoint& aPoint);

		// Scaling of points
		CTiglPoint operator*(double) const;

		// returns a'*a
		double norm2Sqr() const;

		// returns the length of the vector
		double norm2() const;

		// Virtual Destructor
		virtual ~CTiglPoint(void);

		// Convert a CTiglPoint to a OpenCascade gp_Pnt
		gp_Pnt Get_gp_Pnt(void) const;

		// Dump internal point data
		void Dump(std::ostream& aStream) const;

		// scalar product
		static double inner_prod(const CTiglPoint& aPoint, const CTiglPoint& bPoint);

		// cross product
		static CTiglPoint cross_prod(const CTiglPoint& a, const CTiglPoint& b);

	public:
		double x;
		double y;
		double z;
	};

} // end namespace tigl

#endif // CTIGLPOINT_H
