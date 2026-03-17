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
* @brief Calculator for NACA wing profile coordinates
*/

#include "gp_Vec2d.hxx"
#include "tiglmathfunctions.h"
#include "NACA4Calculator.h"
#include "CFunctionToBspline.h"

namespace tigl{
        
        NACA4Calculator::NACA4Calculator(double max_camber, double max_camber_position, double max_profile_thickness, double trailing_edge_thickness)
         : max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness_half(trailing_edge_thickness/2) 
        {
            if(this->max_camber > 1 || this->max_camber < 0){
                throw ::std::logic_error("error in NACA4Calculator max_camber after normalization must be between 0 and 1.");
            }
            if(this->max_camber_position > 1 || this->max_camber_position < 0){
                throw ::std::logic_error("error in NACA4Calculator max_camber_position after normalization must be between 0 and 1.");
            }
            if(this->max_profile_thickness > 1 || this->max_profile_thickness < 0){
                throw ::std::logic_error("error in NACA4Calculator max_profile_thickness after normalization must be between 0 and 1.");
            }
        }

        NACA4Calculator::NACA4Calculator(::std::string const& naca_code , const double te_thickness)
            : NACA4Calculator(0.0,
                              0.0,
                              0.0,
                              te_thickness)
        {
            if (naca_code.size() != 4){
                throw std::invalid_argument("error in NACA4Calculator: naca_code is not four digits long");
            }
            try{
                double m = static_cast<double>(naca_code[0] - '0');
                double p = static_cast<double>(naca_code[1] - '0');
                double t = static_cast<double>(std::stoi(naca_code.substr(2,2)));

                *this = NACA4Calculator(m, p, t, te_thickness);
            }
            catch(...){
                throw std::invalid_argument("error in NACA4Calculator: the naca_code format is not correct, it must to contain four digits and nothing else");
            }

        }

        double NACA4Calculator::get_trailing_edge_thickness() const
        {
            return trailing_edge_thickness_half;
        }

        double NACA4Calculator::camberline(double x) const{
        
            double m = this->max_camber;
            double p = this->max_camber_position;
            if(p == 0){
                return 0;
            }
            if(x < 0 || x > 1){
                throw ::std::logic_error("error in NACA4Calculator::camberline: x must be between 0 and 1.");
            }
            else if(0 <= x && x <= p){
                return (2*p*x - x*x)*m/(p*p);; 
            }
            else if(x > p){ 
                return (1 - 2*p + 2*p*x - x*x)*m/((1-p)*(1-p));;
            }
        }

        gp_Vec2d NACA4Calculator::upper_curve(double x) const{
            double yt = profile_thickness(x); 
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point + yt*normal(x);
        }
        
        gp_Vec2d NACA4Calculator::lower_curve(double x) const{ 
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point - yt*normal(x);
        }

        double NACA4Calculator::profile_thickness(double x) const{ 
            double t = this->max_profile_thickness; 
            double e = -(trailing_edge_thickness_half/(5*t)) + (0.2969 - 0.1260 - 0.3516 + 0.2843);
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - e*pow(x,4));
        }

        double NACA4Calculator::camberline_derivative(double x) const{ //c'(x)
            double m = this->max_camber;
            double p = this->max_camber_position;

            if(p == 0){
                return 0;
            }
            if(0 <= x && x <= p){
                return (2*p - 2*x)*m/(p*p);
            }
            else if(x > p){
                return ( 2*p - 2*x)*m/((1-p)*(1-p));;
            }
            else{
                throw ::std::logic_error("error in NACA4Calculator::camberline_derivative: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA4Calculator::normal(double x) const{
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;

        }

        Handle(Geom_BSplineCurve) NACA4Calculator::upper_bspline() const{

            NACA4UpperCurve upperCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(upperCurve, umin, umax, degree, tolerance, maxDepth); 
            return converter.Curve();
        }

        Handle(Geom_BSplineCurve) NACA4Calculator::lower_bspline() const{
            NACA4LowerCurve lowerCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(lowerCurve, umin, umax, degree, tolerance, maxDepth);
            return converter.Curve();
        }

        NACA4UpperCurve::NACA4UpperCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double NACA4UpperCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.X();
        }
        double NACA4UpperCurve::valueY(double t)  {
            return 0.0;
        }
        double NACA4UpperCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.Y();
        }

        NACA4LowerCurve::NACA4LowerCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double NACA4LowerCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.X();
        }

        double NACA4LowerCurve::valueY(double t)  {
            return 0.0;
        }

        double NACA4LowerCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.Y();
        }

} //namespace tigl


