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

/*

#pragma once

#include <gp_Vec2d.hxx>
#include <math/tiglmathfunctions.h>
#include "CFunctionToBspline.h"


namespace tigl{


class NACA5Calculator{

    public:
        
        TIGL_EXPORT NACA5Calculator(double max_camber_cl = 0, double max_camber_position = 0, double reflex = 0,double max_profile_thickness = 12, double trailing_edge_thickness = 0);

        explicit TIGL_EXPORT NACA5Calculator(const ::std::string& naca_code, const double te_thickness);

        
        TIGL_EXPORT double get_trailing_edge_thickness() const; 

        
        TIGL_EXPORT double camberline(double x) const; 

        
        TIGL_EXPORT gp_Vec2d upper_curve(double x) const; 

        
        TIGL_EXPORT gp_Vec2d lower_curve(double x) const; 

        
        TIGL_EXPORT double profile_thickness(double x) const; 

        
        TIGL_EXPORT double trailing_edge_thickness_function(double y) const; 

        TIGL_EXPORT Handle(Geom_BSplineCurve) upper_bspline() const; 

        TIGL_EXPORT Handle(Geom_BSplineCurve) lower_bspline() const; 

    private:
        
        double camberline_derivative(double x) const;

        
        gp_Vec2d normal(double x) const;


    private:
        double  max_camber, max_camber_position, max_profile_thickness, trailing_edge_thickness;
};

class NACA4UpperCurve : public MathFunc3d {
    public: 
        TIGL_EXPORT explicit NACA4UpperCurve( NACA4Calculator const& calculator);

        
        double valueX(double t) override;

        
        
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};

class NACA4LowerCurve : public MathFunc3d {
    public: 
        TIGL_EXPORT explicit NACA4LowerCurve( NACA4Calculator const& calculator);

        
        double valueX(double t) override;

        
        double valueY(double t) override;

       
        double valueZ(double t) override;

    private:
         NACA4Calculator const& calculator;
};
}

*/