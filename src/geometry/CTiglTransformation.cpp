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

#include "CTiglTransformation.h"
#include "CTiglError.h"
#include "tigl.h"
#include "BRepBuilderAPI_GTransform.hxx"
// [[CAS_AES]] added include for simple transformations
#include "BRepBuilderAPI_Transform.hxx"
#include "gp_XYZ.hxx"
#include "Standard_Version.hxx"

namespace tigl 
{

// Constructor
CTiglTransformation::CTiglTransformation(void)
{
    SetIdentity();
}

CTiglTransformation::CTiglTransformation(const gp_GTrsf& ocMatrix) 
{
    SetIdentity();
    
    // Vectorial part
    for (int i = 0; i < 3; ++i) {
        for (unsigned j = 0; j < 3; ++j) {
            m_matrix[i][j] = ocMatrix.Value(i+1,j+1);
        }
    }

    // Translation part
    m_matrix[0][3] = ocMatrix.Value(1,4);
    m_matrix[1][3] = ocMatrix.Value(2,4);
    m_matrix[2][3] = ocMatrix.Value(3,4);
}

// [[CAS_AES]] added constructor for transformation based on gp_Trsf
CTiglTransformation::CTiglTransformation(const gp_Trsf& trans)
{
    SetIdentity();

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            m_matrix[row][col] = trans.Value(row+1, col+1);
        }
    }
}

// Destructor
CTiglTransformation::~CTiglTransformation(void)
{
}

CTiglTransformation& CTiglTransformation::operator=(const CTiglTransformation& mat)
{
    for (unsigned int i = 0; i < 4; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            m_matrix[i][j] = mat.m_matrix[i][j];
        }
    }
    // [[CAS_AES]] performance optimization for simple transformations
    isSimple = mat.isSimple;
    return *this;
}

void CTiglTransformation::SetIdentity(void)
{
    // preset with 0
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            m_matrix[row][col] = 0.0;
        }
    }

    // set diagonal to 1
    for (int i = 0; i < 4; i++) {
        m_matrix[i][i] = 1.0;
    }
    // [[CAS_AES]] performance optimization for simple transformations
    isSimple = true;
}

// Sets a value of the transformation matrix by row/col
void CTiglTransformation::SetValue(int row, int col, double value)
{
    if (row < 0 || row > 3 || col < 0 || col > 3) {
        throw CTiglError("Error: Invalid row or column index in CTiglTransformation::SetValue", TIGL_INDEX_ERROR);
    }

    m_matrix[row][col] = value;
    // [[CAS_AES]] not possible to identify if the new transformation is simple or not
    isSimple = false;
}

// Post multiply this matrix with another matrix and stores 
// the result in this matrix
void CTiglTransformation::PostMultiply(const CTiglTransformation& aTrans)
{
    double tmp_matrix[4][4];
    int row;

    for (row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            tmp_matrix[row][col] = 0.0;
            for (int j = 0; j < 4; j++) {
                tmp_matrix[row][col] += m_matrix[row][j] * aTrans.m_matrix[j][col];
            }
        }
    }

    // Store results back
    for (row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            m_matrix[row][col] = tmp_matrix[row][col];
        }
    }
    // [[CAS_AES]] new transformation is simple when both were simple
    isSimple = isSimple && aTrans.isSimple;
}

// Pre multiply this matrix with another matrix and stores 
// the result in this matrix
void CTiglTransformation::PreMultiply(const CTiglTransformation& aTrans)
{
    double tmp_matrix[4][4];
    int row;

    for (row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            tmp_matrix[row][col] = 0.0;
            for (int j = 0; j < 4; j++) {
                tmp_matrix[row][col] += aTrans.m_matrix[row][j] * m_matrix[j][col];
            }
        }
    }

    // store results back
    for (row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            m_matrix[row][col] = tmp_matrix[row][col];
        }
    }
    // [[CAS_AES]] new transformation is simple when both were simple
    isSimple = isSimple && aTrans.isSimple;
}

// Returns the current transformation as gp_GTrsf object
gp_GTrsf CTiglTransformation::Get_gp_GTrsf(void) const
{
    gp_GTrsf ocMatrix;

    // Vectorial part
    ocMatrix.SetValue(1, 1, m_matrix[0][0]);
    ocMatrix.SetValue(1, 2, m_matrix[0][1]);
    ocMatrix.SetValue(1, 3, m_matrix[0][2]);

    ocMatrix.SetValue(2, 1, m_matrix[1][0]);
    ocMatrix.SetValue(2, 2, m_matrix[1][1]);
    ocMatrix.SetValue(2, 3, m_matrix[1][2]);

    ocMatrix.SetValue(3, 1, m_matrix[2][0]);
    ocMatrix.SetValue(3, 2, m_matrix[2][1]);
    ocMatrix.SetValue(3, 3, m_matrix[2][2]);

    // Translation part
    ocMatrix.SetValue(1, 4, m_matrix[0][3]);
    ocMatrix.SetValue(2, 4, m_matrix[1][3]);
    ocMatrix.SetValue(3, 4, m_matrix[2][3]);

    return ocMatrix;
}

// [[CAS_AES]] Returns the current transformation as gp_Trsf object
gp_Trsf CTiglTransformation::Get_gp_Trsf(void) const
{
    gp_Trsf ocMatrix;

    // Vectorial part
// SUPPORT for old OpenCASCADE Version
#if OCC_VERSION_MAJOR <= 6 && OCC_VERSION_MINOR <= 7
    ocMatrix.SetValues(m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3], 
                        m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3],
                        m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3],
                        Precision::Confusion(), Precision::Confusion());
#else
    ocMatrix.SetValues(m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3], 
                        m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3],
                        m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3]);
#endif

    return ocMatrix;
}

// Converts degree to radian, utility function
double CTiglTransformation::DegreeToRadian(double degree)
{
    return (degree * 1.74532925199433E-02);
}

// Converts radian to degree, utility function
double CTiglTransformation::RadianToDegree(double radian)
{
    return (radian * 57.2957795130823);
}

// Adds a translation to this transformation. Translation part
// is stored in the last column of the transformation matrix.
void CTiglTransformation::AddTranslation(double tx, double ty, double tz)
{
    // Matrix is:
    //
    // (      1       0       0       tx )
    // (      0       1       0       ty )
    // (      0       0       1       tz )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 3, tx);
    trans.SetValue(1, 3, ty);
    trans.SetValue(2, 3, tz);
    // [[CAS_AES]]
    trans.isSimple = true;

    PreMultiply(trans);
}

void CTiglTransformation::AddScaling(double sx, double sy, double sz)
{
    // Matrix is:
    //
    // (     sx       0       0        0 )
    // (      0      sy       0        0 )
    // (      0       0       sz       0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, sx);
    trans.SetValue(1, 1, sy);
    trans.SetValue(2, 2, sz);
    // [[CAS_AES]]
    // quickfix for bug #240, simple scale seems to cause drastic 
    // performance issues in GeomALGO_Splitter on high scale values, so 
    // transformation is only set to simple for small scale values
    if ((sx == sy) && (sx == sz) && fabs(sx) <= 10) {
        trans.isSimple = true;
    }
    // [[CAS_AES]]
    // quickfix for bug #359: scale of zero seems to destroy shape when gp_Trsf 
    // is used
    if (sx == 0 && sy == 0 && sz == 0) {
        trans.isSimple = false;
    }

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationX(double degreeX)
{
    double radianX = DegreeToRadian(degreeX);
    double sinVal  = sin(radianX);
    double cosVal  = cos(radianX);

    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0  cosVal -sinVal        0 )
    // (      0  sinVal  cosVal        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(1, 1, cosVal);
    trans.SetValue(1, 2, -sinVal);
    trans.SetValue(2, 1, sinVal);
    trans.SetValue(2, 2, cosVal);
    // [[CAS_AES]]
    trans.isSimple = true;

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationY(double degreeY)
{
    double radianY = DegreeToRadian(degreeY);
    double sinVal  = sin(radianY);
    double cosVal  = cos(radianY);

    // Matrix is:
    //
    // ( cosVal       0  sinVal        0 )
    // (      0       1       0        0 )
    // (-sinVal       0  cosVal        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, cosVal);
    trans.SetValue(0, 2, sinVal);
    trans.SetValue(2, 0, -sinVal);
    trans.SetValue(2, 2, cosVal);
    // [[CAS_AES]]
    trans.isSimple = true;

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationZ(double degreeZ)
{
    double radianZ = DegreeToRadian(degreeZ);
    double sinVal  = sin(radianZ);
    double cosVal  = cos(radianZ);

    // Matrix is:
    //
    // ( cosVal -sinVal       0        0 )
    // ( sinVal  cosVal       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, cosVal);
    trans.SetValue(0, 1, -sinVal);
    trans.SetValue(1, 0, sinVal);
    trans.SetValue(1, 1, cosVal);
    // [[CAS_AES]]
    trans.isSimple = true;

    PreMultiply(trans);
}

// Adds projection an xy plane by setting the z coordinate to 0
void CTiglTransformation::AddProjectionOnXYPlane(void)
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       0        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(2, 2, 0.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Adds projection an xz plane by setting the y coordinate to 0
void CTiglTransformation::AddProjectionOnXZPlane(void)
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       0       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(1, 1, 0.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Adds projection an yz plane by setting the x coordinate to 0
void CTiglTransformation::AddProjectionOnYZPlane(void)
{
    // Matrix is:
    //
    // (      0       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, 0.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Adds mirroring at xy plane
void CTiglTransformation::AddMirroringAtXYPlane(void)
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       -1       0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(2, 2, -1.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Adds mirroring at xz plane
void CTiglTransformation::AddMirroringAtXZPlane(void)
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       -1      0        0 )
    // (      0       0       0        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(1, 1, -1.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Adds mirroring at yz plane
void CTiglTransformation::AddMirroringAtYZPlane(void)
{
    // Matrix is:
    //
    // (      -1      0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, -1.0);
    // [[CAS_AES]]
    trans.isSimple = false;

    PreMultiply(trans);
}

// Transforms a shape with the current transformation matrix and
// returns the transformed shape
TopoDS_Shape CTiglTransformation::Transform(const TopoDS_Shape& shape) const
{
    // [[CAS_AES]] added special handling for simple transformations (performance optimization)
    if (isSimple) {
        const BRepBuilderAPI_Transform brepBuilderTransform(shape, Get_gp_Trsf(), Standard_True);
        const TopoDS_Shape& transformedShape = brepBuilderTransform.Shape();
        return transformedShape;
    } else {
        const BRepBuilderAPI_GTransform brepBuilderGTransform(shape, Get_gp_GTrsf(), Standard_True);
        const TopoDS_Shape& transformedShape = brepBuilderGTransform.Shape();
        return transformedShape;
    }
}

// Transforms a point with the current transformation matrix and
// returns the transformed point
gp_Pnt CTiglTransformation::Transform(const gp_Pnt& point) const
{
    // [[CAS_AES]] added special handling for simple transformations (performance optimization)
    if (isSimple) {
        gp_XYZ transformed(point.X(), point.Y(), point.Z());
        Get_gp_Trsf().Transforms(transformed);
        return gp_Pnt(transformed.X(), transformed.Y(), transformed.Z());
    } else {
        gp_XYZ transformed(point.X(), point.Y(), point.Z());
        Get_gp_GTrsf().Transforms(transformed);
        return gp_Pnt(transformed.X(), transformed.Y(), transformed.Z());
    }
}

void CTiglTransformation::printTransformMatrix()
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            cout << m_matrix[i][j] << "\t";
        }
        cout << endl;
    }
}

CTiglTransformation CTiglTransformation::Inverted() const 
{
    // [[CAS_AES]] added support for simple transformations
    if (isSimple) {
        return Get_gp_Trsf().Inverted();
    } else {
        return Get_gp_GTrsf().Inverted();
    }
}

// [[CAS_AES]] added getter for matrix values
double CTiglTransformation::GetValue(int row, int col) const {
    if (row < 0 || row > 3 || col < 0 || col > 3)
        throw CTiglError("Error: Invalid row or column index in CTiglTransformation::GetValue", TIGL_INDEX_ERROR);

    return m_matrix[row][col];
}

} // end namespace tigl
