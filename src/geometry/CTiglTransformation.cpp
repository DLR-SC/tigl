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
* @brief  Implementation of TIGL transformations.
*/

#include "CTiglTransformation.h"
#include "CTiglError.h"
#include "tigl.h"
#include "BRepBuilderAPI_GTransform.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "gp_XYZ.hxx"
#include "Standard_Version.hxx"

#include "tiglmathfunctions.h"
#include "tiglcommonfunctions.h"

namespace tigl 
{

// Constructor
CTiglTransformation::CTiglTransformation()
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

// Constructor for transformation based on gp_Trsf
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
CTiglTransformation::~CTiglTransformation()
{
}

CTiglTransformation& CTiglTransformation::operator=(const CTiglTransformation& mat)
{
    for (unsigned int i = 0; i < 4; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            m_matrix[i][j] = mat.m_matrix[i][j];
        }
    }
    return *this;
}

void CTiglTransformation::SetIdentity()
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
}

// Sets a value of the transformation matrix by row/col
void CTiglTransformation::SetValue(int row, int col, double value)
{
    if (row < 0 || row > 3 || col < 0 || col > 3) {
        throw CTiglError("Invalid row or column index in CTiglTransformation::SetValue", TIGL_INDEX_ERROR);
    }

    m_matrix[row][col] = value;
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
}

// Returns the current transformation as gp_GTrsf object
gp_GTrsf CTiglTransformation::Get_gp_GTrsf() const
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

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationX(double degreeX)
{
    double radianX = Radians(degreeX);
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

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationY(double degreeY)
{
    double radianY = Radians(degreeY);
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

    PreMultiply(trans);
}

void CTiglTransformation::AddRotationZ(double degreeZ)
{
    double radianZ = Radians(degreeZ);
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

    PreMultiply(trans);
}

// Adds a rotation in intrinsic x-y'-z'' Euler convention to the matrix
void CTiglTransformation::AddRotationIntrinsicXYZ(double phi, double theta, double psi)
{
    // intrinsic x-y'-z'' corresponds to extrinsic z-y-x, i.e. Rx*Ry*Rz:
    AddRotationZ(psi);
    AddRotationY(theta);
    AddRotationX(phi);
}

// Adds projection an xy plane by setting the z coordinate to 0
void CTiglTransformation::AddProjectionOnXYPlane()
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       0        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(2, 2, 0.0);

    PreMultiply(trans);
}

// Adds projection an xz plane by setting the y coordinate to 0
void CTiglTransformation::AddProjectionOnXZPlane()
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       0       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(1, 1, 0.0);

    PreMultiply(trans);
}

// Adds projection an yz plane by setting the x coordinate to 0
void CTiglTransformation::AddProjectionOnYZPlane()
{
    // Matrix is:
    //
    // (      0       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, 0.0);

    PreMultiply(trans);
}

// Adds mirroring at xy plane
void CTiglTransformation::AddMirroringAtXYPlane()
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       -1       0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(2, 2, -1.0);

    PreMultiply(trans);
}

// Adds mirroring at xz plane
void CTiglTransformation::AddMirroringAtXZPlane()
{
    // Matrix is:
    //
    // (      1       0       0        0 )
    // (      0       -1      0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(1, 1, -1.0);

    PreMultiply(trans);
}

// Adds mirroring at yz plane
void CTiglTransformation::AddMirroringAtYZPlane()
{
    // Matrix is:
    //
    // (      -1      0       0        0 )
    // (      0       1       0        0 )
    // (      0       0       1        0 )
    // (      0       0       0        1 )

    CTiglTransformation trans;
    trans.SetValue(0, 0, -1.0);

    PreMultiply(trans);
}

// Transforms a shape with the current transformation matrix and
// returns the transformed shape
TopoDS_Shape CTiglTransformation::Transform(const TopoDS_Shape& shape) const
{

    if (IsUniform()) {
        gp_Trsf t;
        t.SetValues(m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3],
                    m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3],
                    m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3]
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,8,0)
                );
#else
                ,1e-10, 1e-10);
#endif
        BRepBuilderAPI_Transform trafo(shape, t, Standard_True);
        return trafo.Shape();
    }
    else {
        BRepBuilderAPI_GTransform brepBuilderGTransform(shape, Get_gp_GTrsf(), Standard_True);
        const TopoDS_Shape& transformedShape = brepBuilderGTransform.Shape();
        return transformedShape;
    }
}

// Transforms a point with the current transformation matrix and
// returns the transformed point
gp_Pnt CTiglTransformation::Transform(const gp_Pnt& point) const
{
    gp_XYZ transformed(point.X(), point.Y(), point.Z());
    Get_gp_GTrsf().Transforms(transformed);
    return gp_Pnt(transformed.X(), transformed.Y(), transformed.Z());
}

bool CTiglTransformation::IsUniform() const
{
    // The following code is copied from gp_Trsf

    gp_XYZ col1(m_matrix[0][0], m_matrix[1][0], m_matrix[2][0]);
    gp_XYZ col2(m_matrix[0][1], m_matrix[1][1], m_matrix[2][1]);
    gp_XYZ col3(m_matrix[0][2], m_matrix[1][2], m_matrix[2][2]);

    // compute the determinant
    gp_Mat M(col1, col2, col3);
    Standard_Real s = M.Determinant();

    if (fabs(s) < Precision::Confusion()) {
        return false;
    }

    if (s > 0) {
        s = Pow(s, 1. / 3.);
    }
    else {
        s = -Pow(-s, 1. / 3.);
    }
    M.Divide(s);

    // check if the matrix is a rotation matrix
    // i.e. check if M^T * M = I
    gp_Mat TM(M);
    TM.Transpose();
    TM.Multiply(M);

    // don t trust the initial values !
    gp_Mat anIdentity;
    anIdentity.SetIdentity();
    TM.Subtract(anIdentity);

    double v = 0;
    v = TM.Value(1, 1);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(1, 2);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(1, 3);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(2, 1);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(2, 2);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(2, 3);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(3, 1);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(3, 2);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    v = TM.Value(3, 3);
    if (fabs(v) > Precision::Confusion()) {
        return false;
    }

    return true;
}

CTiglTransformation CTiglTransformation::Inverted() const
{
    return Get_gp_GTrsf().Inverted();
}

void CTiglTransformation::Decompose(double scale[3], double rotation[3], double translation[3]) const
{
    // compute polar decomposition of upper 3x3 part
    tiglMatrix A(1, 3, 1, 3);
    tiglMatrix P(1, 3, 1, 3);
    tiglMatrix U(1, 3, 1, 3);
    for( int i=0; i<3; ++i) {
        for ( int j=0; j<3; ++j) {
            A(i+1,j+1)= GetValue(i,j);
        }
    }
    PolarDecomposition(A, U, P);

    // scale is diagonal of P
    scale[0] = P(1,1);
    scale[1] = P(2,2);
    scale[2] = P(3,3);

    // check for shearing
    double aveAbsOffDiag = (fabs(P(1,2)) + fabs(P(1,3)) + fabs(P(2,3)))/3;
    if (aveAbsOffDiag > Precision::Confusion() ) {
        LOG(WARNING) << "CTiglTransformation::Decompose: The Transformation contains a Shearing, that will be discarded in the decomposition!";
    }

    // calculate intrinsic Euler angles from rotation matrix U
    //
    // This implementation is based on http://www.gregslabaugh.net/publications/euler.pdf, where the same argumentation
    // is used for intrinsic x,y',z'' angles and the rotatation matrix
    //
    //                |                        cos(y)*cos(z) |               -        cos(y)*cos(z) |         sin(y) |
    // U = Rx*Ry*Rz = | cos(x)*sin(z) + sin(x)*sin(y)*cos(z) | cos(x)*cos(z) - sin(x)*sin(y)*sin(z) | -sin(x)*cos(y) |
    //                | sin(x)*sin(z) - cos(x)*sin(y)*cos(z) | sin(x)*cos(z) + cos(x)*sin(y)*sin*z( |  cos(x)*cos(y) |
    //
    // rather than extrinsic angles and the Rotation matrix mentioned in that pdf.

    if( fabs( fabs(U(1, 3)) - 1) > 1e-10 ){
        rotation[1] = asin(U(1, 3));
        double cosTheta = cos(rotation[1]);
        rotation[0] = -atan2(U(2,3)/cosTheta, U(3,3)/cosTheta);
        rotation[2] = -atan2(U(1,2)/cosTheta, U(1,1)/cosTheta);
    }
    else {
        rotation[0] = 0;
        if ( fabs(U(1,3) + 1) > 1e-10 ) {
            rotation[2] = -rotation[0] - atan2(U(2,1), U(2,2));
            rotation[1] = -M_PI/2;
        }
        else{
            rotation[2] = -rotation[0] + atan2(U(2,1), U(2,2));
            rotation[1] = M_PI/2;
        }
    }

    rotation[0] = Degrees(rotation[0]);
    rotation[1] = Degrees(rotation[1]);
    rotation[2] = Degrees(rotation[2]);

    // translation is last column of transformation
    translation[0] = GetValue(0,3);
    translation[1] = GetValue(1,3);
    translation[2] = GetValue(2,3);

}

bool CTiglTransformation::DecomposeTRSRS(CTiglPoint& scaling1, CTiglPoint& rotation1, CTiglPoint& scaling2,
                                         CTiglPoint& rotation2, CTiglPoint& translation) const
{

    // The theory behind this decomposition can be found at:
    // http://research.cs.wisc.edu/graphics/Courses/838-s2002/Papers/polar-decomp.pdf

    // Basically we do:
    // M = T*U*P            where UP is the polar decompostion
    // M = T*U*V*D*Vt       we decompose P using a Jacobi decomposition
    // M = T*U*V*N*D*VT*N   we force U*V and VT to be proper rotation  using the fact that an improper rotation U
    //                      can be factorized in U = QN, where Q is a proper rotation and N =+/-I
    // M = T*R2*N*D*R1*N
    // M = T*R2*S2*R1*S1    we store N in the scaling matrices

    tiglMatrix A(1, 3, 1, 3);
    tiglMatrix P(1, 3, 1, 3);
    tiglMatrix U(1, 3, 1, 3);

    tiglMatrix D2(1, 3, 1, 3);
    tiglMatrix V(1, 3, 1, 3);

    tiglMatrix R2(1, 3, 1, 3);
    tiglMatrix R1(1, 3, 1, 3);
    tiglMatrix D1(1, 3, 1, 3);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            A(i + 1, j + 1) = GetValue(i, j);
        }
    }
    PolarDecomposition(A, U, P);

    // check if P is a proper diagonal (P is symmetric so we can check only the upper part)
    double aveAbsOffDiag = (fabs(P(1, 2)) + fabs(P(1, 3)) + fabs(P(2, 3))) / 3;

    // If S is all ready a proper scaling  we do no decompose P
    if (aveAbsOffDiag < Precision::Confusion()) {
        if (U.Determinant() < 0) {
            R2 = U * -1;
            D2 = P * -1;
        } else {
            R2 = U;
            D2 = P;
        }
        R1.Init(0);
        R1(1, 1) = 1.0;
        R1(2, 2) = 1.0;
        R1(3, 3) = 1.0;
        D1.Init(0);
        D1(1, 1) = 1.0;
        D1(2, 2) = 1.0;
        D1(3, 3) = 1.0;
    }
    else { // Otherwise we use decompose P using the Jacobi method

        DiagonalizeMatrixByJacobi(P, D2, V);
        R2 = U * V;
        if (R2.Determinant() < 0) {
            R2 = R2 * -1;
            D2 = D2 * -1;
        }

        R1 = V.Transposed();

        D1.Init(0.0);
        D1(1, 1) = 1.0;
        D1(2, 2) = 1.0;
        D1(3, 3) = 1.0;

        if (R1.Determinant() < 0) {
            R1 = R1 * -1;
            D1 = D1 * -1;
        }
    }

    // Set the return values;

    // translation is last column of transformation
    translation.x = GetValue(0, 3);
    translation.y = GetValue(1, 3);
    translation.z = GetValue(2, 3);

    rotation2 = RotMatrixToIntrinsicXYZVector(R2);

    scaling2.x = D2(1, 1);
    scaling2.y = D2(2, 2);
    scaling2.z = D2(3, 3);

    rotation1 = RotMatrixToIntrinsicXYZVector(R1);

    scaling1.x = D1(1, 1);
    scaling1.y = D1(2, 2);
    scaling1.z = D1(3, 3);

//
//    CTiglTransformation verify;
//    verify.AddScaling(scaling1.x,scaling1.y,scaling1.z);
//    verify.AddRotationIntrinsicXYZ(rotation1.x,rotation1.y,rotation1.z);
//    verify.AddScaling(scaling2.x,scaling2.y,scaling2.z);
//    verify.AddRotationIntrinsicXYZ(rotation2.x,rotation2.y,rotation2.z);
//    verify.AddTranslation(translation.x,translation.y,translation.z);
//    if( !this->IsNear(verify, 0.001)) {
//        throw CTiglError("DecomposeTRSRS is not equivalent to the original transformation! ");
//    }


}

// Getter for matrix values
double CTiglTransformation::GetValue(int row, int col) const
{
    if (row < 0 || row > 3 || col < 0 || col > 3) {
        throw CTiglError("Invalid row or column index in CTiglTransformation::GetValue", TIGL_INDEX_ERROR);
    }

    return m_matrix[row][col];
}

tigl::CTiglTransformation tigl::CTiglTransformation::GetRotationToAlignAToB(tigl::CTiglPoint vectorA,
                                                                            tigl::CTiglPoint vectorB)
{

    vectorA.normalize();
    vectorB.normalize();

    CTiglTransformation Rot;

    if (vectorA.isNear((vectorB))) {
        return Rot;
    }
    if (vectorA.isNear((vectorB * -1))) {
            CTiglPoint ortho = FindOrthogonalVectorToDirection(vectorA);
            return CTiglTransformation::GetRotationFromAxisRotation(ortho,180);
    }

    CTiglPoint cross = CTiglPoint::cross_prod(vectorA, vectorB);
    double cos       = CTiglPoint::inner_prod(vectorA, vectorB);
    double s         = 1.0 / (1.0 + cos);

    CTiglTransformation V;
    V.SetIdentity();
    V.SetValue(0, 0, 0);
    V.SetValue(0, 1, -cross.z);
    V.SetValue(0, 2, cross.y);
    V.SetValue(1, 0, cross.z);
    V.SetValue(1, 1, 0);
    V.SetValue(1, 2, -cross.x);
    V.SetValue(2, 0, -cross.y);
    V.SetValue(2, 1, cross.x);
    V.SetValue(2, 2, 0);
    V.SetValue(3, 3, 0);

    CTiglTransformation V2 = V * V;

    Rot = ((Rot + V) + (s * V2));

    return Rot;
}


tigl::CTiglTransformation tigl::CTiglTransformation::GetRotationFromAxisRotation( tigl::CTiglPoint a, double angle )
{

    CTiglTransformation R;
    a.normalize();
    double c = cos(Radians(angle));
    double s = sin(Radians(angle));
    R.SetValue(0, 0, c + (pow(a.x,2)*(1-c)) );
    R.SetValue(0, 1, a.x * a.y * (1-c) - (a.z * s) );
    R.SetValue(0, 2, a.x * a.z * (1-c) + (a.y * s));

    R.SetValue(1, 0, a.y * a.x *(1-c) + (a.z *s ));
    R.SetValue(1, 1, c + (pow(a.y,2) * (1-c) )) ;
    R.SetValue(1, 2, a.y * a.z*(1-c) - (a.x *s));

    R.SetValue(2, 0, a.z*a.x*(1-c) - (a.y * s));
    R.SetValue(2, 1, a.z*a.y*(1-c) + (a.x*s));
    R.SetValue(2, 2, c + (pow(a.z,2)*(1-c)));

    return R;
}


std::ostream& operator<<(std::ostream& os, const CTiglTransformation& t)
{
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            os << t.m_matrix[i][j] << "\t";
        }
        os << endl;
    }
    return os;
}

CTiglTransformation operator*(const CTiglTransformation& a, const CTiglTransformation& b)
{
    CTiglTransformation result = b;
    result.PreMultiply(a);
    return result;
}

CTiglPoint operator*(const CTiglTransformation& m, const CTiglPoint& p)
{

    double augmented_pnt[4] = {p.x, p.y, p.z, 1.0};
    double res_matrix[4]    = {
        0.0,
        0.0,
        0.0,
        0.0,
    };

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            res_matrix[row] += (m.GetValue(row, col) * augmented_pnt[col]);
        }
    }

    CTiglPoint result(res_matrix[0], res_matrix[1], res_matrix[2]);
    return result;
}

CTiglTransformation operator+(const CTiglTransformation& a, const CTiglTransformation& b)
{
    CTiglTransformation result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.SetValue(row, col, a.GetValue(row, col) + b.GetValue(row, col));
        }
    }
    return result;
}

CTiglTransformation operator*( double s, const CTiglTransformation& a)
{
    CTiglTransformation result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.SetValue(row, col, s * a.GetValue(row, col));
        }
    }
    return result;
}

bool CTiglTransformation::HasZeroScaling() const
{
    return (isNear(GetValue(0,0),0)|| isNear(GetValue(1,1),0) || isNear(GetValue(2,2),0));
}

bool CTiglTransformation::IsNear(const tigl::CTiglTransformation &other, double epsilon) const
{
    bool result = true;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result = result && fabs( GetValue(row, col) - other.GetValue(row, col)) <= epsilon;
        }
    }
    return result;
}

CTiglPoint CTiglTransformation::GetTranslation()
{
    // translation is last column of transformation
    CTiglPoint translation;
    translation.x = GetValue(0,3);
    translation.y = GetValue(1,3);
    translation.z = GetValue(2,3);
    return translation;
}

} // end namespace tigl
