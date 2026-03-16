/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
* @brief Header for Calculator for NACA wing profile coordinates
*/

#pragma once

#include <gp_Vec2d.hxx>
#include <math/tiglmathfunctions.h>
#include "CFunctionToBspline.h"


namespace tigl{


class NACA4Calculator{

    public:
        /**
        * @brief Construct a new NACA4Calculator object
        * 
        * @param max_camber 
        * @param max_camber_position 
        * @param max_profile_thickness 
        */
        TIGL_EXPORT NACA4Calculator(double max_camber = 0, double max_camber_position = 0, double max_profile_thickness = 12, double trailing_edge_thickness = 0);

        explicit TIGL_EXPORT NACA4Calculator(const ::std::string& naca_code, const double te_thickness);

        /**
         * @brief getter for the trailing edge thickness
         * 
         * @return double 
         */
        TIGL_EXPORT double get_trailing_edge_thickness() const; 

        /**
         * @brief Evaluate the camberline at parameter x, where x is between 0 and 1
         * 
         * @param x 
         * @return double 
         */
        TIGL_EXPORT double camberline(double x) const; 

        /**
         * @brief Evaluate the upper curve given a parameter x between 0 and 1.
         * 
         * @param x 
         * @return gp_Vec2d 
         */
        TIGL_EXPORT gp_Vec2d upper_curve(double x) const; 

        /**
         * @brief Calculate the y coordinate for the lower curve
         * 
         * @param x 
         * @return gp_Vec2d 
         */
        TIGL_EXPORT gp_Vec2d lower_curve(double x) const; 

        /**
         * @brief Calculate the local thickness of the profile given a parameter x with values between 0 and 1.
         * 
         * @param x 
         * @return double 
         */
        TIGL_EXPORT double profile_thickness(double x) const; 

        /**
         * @brief Construct a new trailing edge thickness object
         * 
         * @param y 
         * @return double
         */
        TIGL_EXPORT double trailing_edge_thickness_function(double y) const; 

        TIGL_EXPORT Handle(Geom_BSplineCurve) upper_bspline() const; 

        TIGL_EXPORT Handle(Geom_BSplineCurve) lower_bspline() const; 

    private:
        /**
         * @brief Calculate the derivative of the camberline
         * 
         * @param x 
         * @return double 
         */
        double camberline_derivative(double x) const;

        /**
         * @brief Calculate the normal by using the norm
         * 
         * @param x 
         * @return gp_Vec2d 
         */
        gp_Vec2d normal(double x) const;


    private:
        double  max_camber, max_camber_position, max_profile_thickness, trailing_edge_thickness;
};

class NACA4UpperCurve : public MathFunc3d {
    public: 
        TIGL_EXPORT explicit NACA4UpperCurve( NACA4Calculator const& calculator);

        /**
         * @brief Get the X coordinate of the upper curve
         * 
         * @param t 
         * @return double 
         */
        double valueX(double t) override;

        /**
         * @brief Get the Y coordinate of the upper curve
         * 
         * @param t 
         * @return double 
         */
        double valueY(double t) override;
        /**
         * @brief Get the Z coordinate of the upper curve
         * 
         * @param t 
         * @return double 
         */
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};

class NACA4LowerCurve : public MathFunc3d {
    public: 
        TIGL_EXPORT explicit NACA4LowerCurve( NACA4Calculator const& calculator);

        /**
         * @brief Get the X coordinate of the lower curve
         * 
         * @param t 
         * @return double 
         */
        double valueX(double t) override;

        /**
         * @brief Get the Y coordinate of the lower curve
         * 
         * @param t 
         * @return double 
         */
        double valueY(double t) override;

        /**
         * @brief Get the Z coordinate of the lower curve
         * 
         * @param t 
         * @return double 
         */
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};
}