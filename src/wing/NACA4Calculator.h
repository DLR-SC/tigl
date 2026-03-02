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

//class gp_Vec2d; //schaun warum des nd geht
#include <gp_Vec2d.hxx>
#include <math/tiglmathfunctions.h>
//#include "MathFunc3d"
#include "CFunctionToBspline.h"

//docstrings dazu (richtiges format) (nur im header file), extension?

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
        NACA4Calculator(double max_camber = 0, double max_camber_position = 0, double max_profile_thickness = 12, double trailing_edge_thickness = 0);

        explicit NACA4Calculator(const ::std::string& naca_code, const double te_thickness);

        /**
         * @brief getter for the trailing edge thickness
         * 
         * @return double 
         */
        double get_trailing_edge_thickness() const; 

        /**
         * @brief Calculate the camberline
         * 
         * @param x 
         * @return double 
         */
        double camberline(double x) const; 


        /**
         * @brief Calculate the y coordinate for the upper curve
         * 
         * @param x 
         * @return gp_Vec2d 
         */
        gp_Vec2d upper_curve(double x) const; 


        /**
         * @brief Calculate the y coordinate for the lower curve
         * 
         * @param x 
         * @return gp_Vec2d 
         */
        gp_Vec2d lower_curve(double x) const; 


        /**
         * @brief Calculate the thickness of the profile
         * 
         * @param x 
         * @return double 
         */
        double profile_thickness(double x) const; 

        /**
         * @brief Construct a new trailing edge thickness object
         * 
         * @param y 
         * @return double
         */
        double trailing_edge_thickness_function(double y) const; 


        Handle(Geom_BSplineCurve) upper_bspline() const; 

        Handle(Geom_BSplineCurve) lower_bspline() const; 

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
        explicit NACA4UpperCurve( NACA4Calculator const& calculator);

        double valueX(double t) override;
        double valueY(double t) override;
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};

class NACA4LowerCurve : public MathFunc3d {
    public: 
        explicit NACA4LowerCurve( NACA4Calculator const& calculator);

        double valueX(double t) override;
        double valueY(double t) override;
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};




}