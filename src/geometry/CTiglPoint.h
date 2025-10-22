/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "tigl_internal.h"
#include <climits>
#include "gp_XYZ.hxx"
#include "gp_Pnt.hxx"
#include "gp_XYZ.hxx"
#include <iostream>
#include "Precision.hxx"

namespace tigl 
{

class CTiglPoint
{

public:
    // Constructor
    TIGL_EXPORT CTiglPoint(double xval = 0.0, double yval = 0.0, double zval = 0.0);

    // Copy constructor
    TIGL_EXPORT CTiglPoint(const CTiglPoint& aPoint);

    TIGL_EXPORT CTiglPoint(const gp_XYZ& aPoint);

    // Assignment operator
    TIGL_EXPORT CTiglPoint& operator=(const CTiglPoint& aPoint);

    // Addition of points
    TIGL_EXPORT CTiglPoint operator+(const CTiglPoint& aPoint) const;

    // Addition of a point
    TIGL_EXPORT CTiglPoint& operator+=(const CTiglPoint& aPoint);

    // Subtraction of points
    TIGL_EXPORT CTiglPoint operator-(const CTiglPoint& aPoint) const;

    // Subtraction of a point
    TIGL_EXPORT CTiglPoint& operator-=(const CTiglPoint& aPoint);

    // Scaling of points
    TIGL_EXPORT CTiglPoint operator*(double) const;

    // Comparison of two points
    TIGL_EXPORT bool operator==(const CTiglPoint& aPoint) const;

    // Comparison of two points, return true if the distance between the two point is less than epsilon
    TIGL_EXPORT bool isNear(const CTiglPoint& aPoint, double epsilon = Precision::Confusion()) const;

    // returns a'*a
    TIGL_EXPORT double norm2Sqr() const;

    // returns the length of the vector
    TIGL_EXPORT double norm2() const;

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglPoint();

    // Convert a CTiglPoint to a OpenCascade gp_Pnt
    TIGL_EXPORT gp_Pnt Get_gp_Pnt() const;

    // Dump internal point data
    TIGL_EXPORT void Dump(std::ostream& aStream) const;

    // scalar product
    TIGL_EXPORT static double inner_prod(const CTiglPoint& aPoint, const CTiglPoint& bPoint);

    // cross product
    TIGL_EXPORT static CTiglPoint cross_prod(const CTiglPoint& a, const CTiglPoint& b);

    // scalar projection of a vector a onto a nonzero vector b
    TIGL_EXPORT static double scalar_projection(const CTiglPoint& a, const CTiglPoint& b);

    // vector projection of a vector a onto a nonzero vector b
    TIGL_EXPORT static CTiglPoint vector_projection(const CTiglPoint& a, const CTiglPoint& b);

    // square distance to another point
    TIGL_EXPORT double distance2(const CTiglPoint& point) const;

    // returns minimum and maximum component
    TIGL_EXPORT void getMinMax(double & min, double & max) const;

    // update the vector such that is norm2 is 1
    TIGL_EXPORT void normalize();

    double x;
    double y;
    double z;
};

} // end namespace tigl

#endif // CTIGLPOINT_H
