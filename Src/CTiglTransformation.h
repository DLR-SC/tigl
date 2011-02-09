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
* @brief  Implementation of TIGL transformations.
*/

#ifndef CTIGLTRANSFORMATION_H
#define CTIGLTRANSFORMATION_H

#include "CTiglPoint.h"
#include "gp_GTrsf.hxx"
#include "gp_Pnt.hxx"
#include "TopoDS.hxx"

namespace tigl {

	class CTiglTransformation
	{

	public:
		// Constructor
		CTiglTransformation(void);

		// Virtual Destructor
		virtual ~CTiglTransformation(void);

		// Converts degree to radian, utility function
		static double DegreeToRadian(double degree);

		// Converts radian to degree, utility function
		static double RadianToDegree(double radian);

		// Sets matrix to identity matrix
		void SetIdentity(void);

		// Sets a value of the transformation matrix by row/col
		void SetValue(int row, int col, double value);

		// Returns the current transformation as gp_GTrsf object
		gp_GTrsf Get_gp_GTrsf(void) const;

		// Post multiply this matrix with another matrix and stores 
		// the result in this matrix
		void PostMultiply(const CTiglTransformation& aTrans);

		// Pre multiply this matrix with another matrix and stores 
		// The result in this matrix
		void PreMultiply(const CTiglTransformation& aTrans);

		// Adds a translation to the matrix
		void AddTranslation(double tx, double ty, double tz);

		// Adds a scaling transformation to the matrix
		void AddScaling(double sx, double sy, double sz);

		// Adds a rotation around the x,y,z axis to the matrix
		void AddRotationX(double degreeX);
		void AddRotationY(double degreeY);
		void AddRotationZ(double degreeZ);

		// Adds projection on xy plane by setting the z coordinate to 0
		void AddProjectionOnXYPlane(void);

		// Adds projection on xz plane by setting the y coordinate to 0
		void AddProjectionOnXZPlane(void);

		// Adds projection on yz plane by setting the x coordinate to 0
		void AddProjectionOnYZPlane(void);

        // Adds mirroring at xy plane
        void AddMirroringAtXYPlane(void);

        // Adds mirroring at xz plane
        void AddMirroringAtXZPlane(void);

        // Adds mirroring at yz plane
        void AddMirroringAtYZPlane(void);

		// Transforms a shape with the current transformation matrix and
		// returns the transformed shape
		TopoDS_Shape Transform(const TopoDS_Shape& shape) const;

		// Transforms a point with the current transformation matrix and
		// returns the transformed point
		gp_Pnt Transform(const gp_Pnt& point) const;

		// Default copy constructor and assignment operator are correct
		// since memberwise copy is enough for this class.

	private:
		double m_matrix[4][4];
	
	};

} // end namespace tigl

#endif // CTIGLTRANSFORMATION_H
