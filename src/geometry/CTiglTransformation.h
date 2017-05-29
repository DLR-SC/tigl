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
* @brief  Implementation of TIGL transformations.
*/

#ifndef CTIGLTRANSFORMATION_H
#define CTIGLTRANSFORMATION_H

#include "tigl_internal.h"
#include "CTiglPoint.h"
#include "gp_GTrsf.hxx"
#include "gp_Pnt.hxx"
#include "TopoDS.hxx"

namespace tigl
{

class CTiglTransformation
{

public:
    // Constructor
    TIGL_EXPORT CTiglTransformation();
    TIGL_EXPORT CTiglTransformation(const gp_GTrsf& ocMatrix);

    // Constructor for transformation based on gp_Trsf
    TIGL_EXPORT CTiglTransformation(const gp_Trsf& trans);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglTransformation();
    
    TIGL_EXPORT CTiglTransformation& operator=(const CTiglTransformation&);

    // Converts degree to radian, utility function
    TIGL_EXPORT static double DegreeToRadian(double degree);

    // Converts radian to degree, utility function
    TIGL_EXPORT static double RadianToDegree(double radian);

    // Sets matrix to identity matrix
    TIGL_EXPORT void SetIdentity();

    // Sets a value of the transformation matrix by row/col
    TIGL_EXPORT void SetValue(int row, int col, double value);

    // Returns the current transformation as gp_GTrsf object
    TIGL_EXPORT gp_GTrsf Get_gp_GTrsf() const;

    // Post multiply this matrix with another matrix and stores 
    // the result in this matrix
    TIGL_EXPORT void PostMultiply(const CTiglTransformation& aTrans);

    // Pre multiply this matrix with another matrix and stores 
    // The result in this matrix
    TIGL_EXPORT void PreMultiply(const CTiglTransformation& aTrans);

    // Adds a translation to the matrix
    TIGL_EXPORT void AddTranslation(double tx, double ty, double tz);

    // Adds a scaling transformation to the matrix
    TIGL_EXPORT void AddScaling(double sx, double sy, double sz);

    // Adds a rotation around the x,y,z axis to the matrix
    TIGL_EXPORT void AddRotationX(double degreeX);
    TIGL_EXPORT void AddRotationY(double degreeY);
    TIGL_EXPORT void AddRotationZ(double degreeZ);

    // Adds projection on xy plane by setting the z coordinate to 0
    TIGL_EXPORT void AddProjectionOnXYPlane();

    // Adds projection on xz plane by setting the y coordinate to 0
    TIGL_EXPORT void AddProjectionOnXZPlane();

    // Adds projection on yz plane by setting the x coordinate to 0
    TIGL_EXPORT void AddProjectionOnYZPlane();

    // Adds mirroring at xy plane
    TIGL_EXPORT void AddMirroringAtXYPlane();

    // Adds mirroring at xz plane
    TIGL_EXPORT void AddMirroringAtXZPlane();

    // Adds mirroring at yz plane
    TIGL_EXPORT void AddMirroringAtYZPlane();

    // Transforms a shape with the current transformation matrix and
    // returns the transformed shape
    TIGL_EXPORT TopoDS_Shape Transform(const TopoDS_Shape& shape) const;

    // Transforms a point with the current transformation matrix and
    // returns the transformed point
    TIGL_EXPORT gp_Pnt Transform(const gp_Pnt& point) const;
    
    // Returns the inverted Transformation
    TIGL_EXPORT CTiglTransformation Inverted() const;

    // Default copy constructor and assignment operator are correct
    // since memberwise copy is enough for this class.

    // for debug purposes
    TIGL_EXPORT friend std::ostream& operator<<(std::ostream& os, const CTiglTransformation& t);

    // Getter for matrix values
    TIGL_EXPORT double GetValue(int row, int col) const;

private:
    bool IsUniform() const;

    double m_matrix[4][4];
};

TIGL_EXPORT CTiglTransformation operator*(const CTiglTransformation& a, const CTiglTransformation& b);

} // end namespace tigl

#endif // CTIGLTRANSFORMATION_H
