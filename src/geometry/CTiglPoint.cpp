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

#include "CTiglPoint.h"
#include "CTiglLogging.h"

namespace tigl 
{

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

CTiglPoint::CTiglPoint(const gp_XYZ& aPoint)
{
    x = aPoint.X();
    y = aPoint.Y();
    z = aPoint.Z();
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

CTiglPoint CTiglPoint::operator+(const CTiglPoint& aPoint) const
{
    CTiglPoint p;
    p.x = x + aPoint.x;
    p.y = y + aPoint.y;
    p.z = z + aPoint.z;
    return p;
}

CTiglPoint& CTiglPoint::operator+=(const CTiglPoint& aPoint) 
{
    x += aPoint.x;
    y += aPoint.y;
    z += aPoint.z;
    return *this;
}

CTiglPoint CTiglPoint::operator-(const CTiglPoint& aPoint) const
{
    CTiglPoint p;
    p.x = x - aPoint.x;
    p.y = y - aPoint.y;
    p.z = z - aPoint.z;
    return p;
}

CTiglPoint& CTiglPoint::operator-=(const CTiglPoint& aPoint) 
{
    x -= aPoint.x;
    y -= aPoint.y;
    z -= aPoint.z;
    return *this;
}

CTiglPoint CTiglPoint::operator*(double s) const
{
    CTiglPoint p;
    p.x = x*s;
    p.y = y*s;
    p.z = z*s;
    return p;
}

double CTiglPoint::norm2Sqr() const 
{
    return x*x+y*y+z*z;
}

double CTiglPoint::norm2() const 
{
    return sqrt(x*x + y*y + z*z);
}

// Destructor
CTiglPoint::~CTiglPoint()
{
}

// Convert a CTiglPoint to a OpenCascade gp_Pnt
gp_Pnt CTiglPoint::Get_gp_Pnt() const
{
    return gp_Pnt(x, y, z);
}

// Dump internal point data
void CTiglPoint::Dump(std::ostream& aStream) const 
{
    aStream << "CTiglPoint: (" << x << ", " << y << ", " << z << ")";
}

double CTiglPoint::inner_prod(const CTiglPoint& a, const CTiglPoint& b) 
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

CTiglPoint CTiglPoint::cross_prod(const CTiglPoint& a, const CTiglPoint& b) 
{
    CTiglPoint c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

// scalar projection of a vector a onto a nonzero vector b
double CTiglPoint::scalar_projection(const CTiglPoint& a, const CTiglPoint& b)
{
    return CTiglPoint::inner_prod(a,b)/b.norm2();
}

// vector projection of a vector a onto a nonzero vector b
CTiglPoint CTiglPoint::vector_projection(const CTiglPoint& a, const CTiglPoint& b)
{
    return b * (CTiglPoint::inner_prod(a,b)/b.norm2Sqr());
}

double CTiglPoint::distance2(const CTiglPoint &p) const 
{
    return (x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) + (z-p.z)*(z-p.z);
}

void CTiglPoint::getMinMax(double & min, double & max) const 
{
    min = x;
    if (y < min) {
        min = y;
    }
    if (z < min) {
        min = z;
    }
    max = x;
    if (y > max) {
        max = y;
    }
    if (z > max) {
        max = z;
    }
}

bool CTiglPoint::operator==(const CTiglPoint& aPoint) const
{
    if ( this->isNear(aPoint) ) {
        return true;
    }
    return false;
}

bool CTiglPoint::isNear(const CTiglPoint& aPoint, double epsilon) const
{
    double d2 = distance2(aPoint);
    double d = sqrt(d2);
    if( d < epsilon) {
        return true;
    }
    return false;

}

void CTiglPoint::normalize()
{
    if (norm2() == 0) {
        LOG(WARNING)
            << "CTiglPoint::normalize: The norm is 0, so it's impossible to normalize, 0 length vector is returned.";
        return;
    }
    double s = 1.0 / norm2();
    x        = s * x;
    y        = s * y;
    z        = s * z;
}

} // end namespace tigl
