
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

/** 
#include "gp_Vec2d.hxx"
#include "tiglmathfunctions.h"
#include "NACA5Calculator.h"
#include "CFunctionToBspline.h"

namespace tigl{
        
        NACA5Calculator::NACA5Calculator(double max_camber_cl, double max_camber_position, double reflex, double max_profile_thickness, double trailing_edge_thickness)
         : max_camber_cl(max_camber_cl/100)
         , max_camber_position(max_camber_position/20)
         , reflex(reflex)
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness(trailing_edge_thickness/2) 
        {
            
        }

        NACA5Calculator::NACA5Calculator(::std::string const& naca_code , const double te_thickness)
            : NACA5Calculator(static_cast<double>(naca_code[0] - '0'),
                              static_cast<double>(naca_code[1] - '0'),
                              static_cast<double>(naca_code[2] - '0'),
                              static_cast<double>(std::stoi(naca_code.substr(3,2))),
                              te_thickness)
        {}

        double NACA5Calculator::get_trailing_edge_thickness() const
        {
            return trailing_edge_thickness;
        }

        double NACA5Calculator::k1(double max_camber_cl, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber_cl*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex = static_cast<int>(reflex);
            double k1;
            //alles von chapter 8 table 8-6
            meanline_designation = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex);
            switch(meanline_designation){
                case "210":
                     k1 =361.400;
                case "220":
                     k1 =51.640;
                case "230":
                     k1 = 15.957;
                case "240":
                    k1 = 6.643;
                case "250":
                    k1 = 3.230;
                case "211":
                    throw ::std::logic_error("error in NACA5Calculator::k1: this profile does not provide a constant for k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                case "221":
                    k1 = 51.99;
                case "231":
                    k1 = 15.793;
                case "241":
                    k1 = 6.520;
                case "251":
                    k1 = 3.191;
                }
            return k1; 
        }

        double NACA5Calculator::m(double max_camber_cl, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber_cl*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex = static_cast<int>(reflex);
            double m;
            //alles von chapter 8 table 8-6
            meanline_designation = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex);
            switch(meanline_designation){
                case "210":
                     m = 0.0580;
                case "220":
                     m = 0.1260;
                case "230":
                     m = 0.2025;
                case "240":
                    m = 0.2900;
                case "250":
                    m = 0.3910;
                case "211":
                    throw ::std::logic_error("error in NACA5Calculator::m: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                case "221":
                    m = 0.1300;
                case "231":
                    m = 0.2170;
                case "241":
                    m = 0.3180;
                case "251":
                    m = 0.4410;
                }
            return m; 
        }
/*
        double NACA5Calculator::k2(double max_camber_cl, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber_cl*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex = static_cast<int>(reflex);
            double k2;
            //alles von chapter 8 table 8-6
            meanline_designation = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex);
            switch(meanline_designation){
                case "210":
                     throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2.");
                case "220":
                     throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2.");
                case "230":
                     throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2.");
                case "240":
                    throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2.");
                case "250":
                    throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2.");
                case "211":
                    throw ::std::logic_error("error in NACA5Calculator::k2: this profile does not provide a constant for k2."); 
                case "221":
                    k2 = 7.64;
                case "231":
                    k2 = 67.70;
                case "241":
                    k2 = 303.0;
                case "251":
                    k2 = 1355;
                }
            return k2; 
        }
            */
/** 
        double NACA5Calculator::camberline(double x) const{ //geändert
        
            double s = this->max_camber_cl;
            double p = this->max_camber_position;
            double q = this->reflex;
            double k1 = k1(s, p, q)
            double m = m(s, p, q)
            double frack1k2 = (3*(m-p)*(m-p)-m*m*m)/((1-m)*(1-m)*(1-m))
            if(q = 0){
            if(p == 0){
                return 0;
            }
            if(0 <= x && x <= p){
                return (k1/6)(x*x*x-3*m*x*x+m*m*(3-m)*x); 
            }
            else if(x > p){ 
                return ((k1*m*m*m)/6)*(1-x);
            }
            }
            else if(q == 1){
            if(p == 0){
                return 0;
            }
            if(0 <= x && x <= p){
                return (k1/6)(((x-m)*(x-m)*(x-m))-(frack1k2*((1-m)*(1-m)*(1-m))*x)- (m*m*m*x+m*m*m)); 
            }
            else if(x > p){ 
                return (k1/6)((frack1k2)*(x-m)*(x-m)*(x-m)-(frack1k2)*(1-m)*(1-m)*(1-m)*x-m*m*m*x+m*m*m);
            }
            }
            else{
                throw ::std::logic_error("error in NACA5Calculator::camberline: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA5Calculator::upper_curve(double x) const{ //gleich
            double yt = profile_thickness(x); 
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            if(yt < 0){
                throw ::std::logic_error("error in NACA4Calculator::upper_curve: the profile_thickness must be positive or 0.");
            }
            return point + yt*normal(x);
        }
        
        gp_Vec2d NACA5Calculator::lower_curve(double x) const{ //gleich
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            if(yt < 0){
                throw ::std::logic_error("error in NACA4Calculator::upper_curve: the profile_thickness must be positive or 0.");
            }
            return point - yt*normal(x);
        }

        double NACA5Calculator::profile_thickness(double x) const{ //bleibt gleich
            double t = this->max_profile_thickness; 
            double e = trailing_edge_thickness_function(trailing_edge_thickness);
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - e*pow(x,4));
        }

        double NACA5Calculator::camberline_derivative(double x) const{ //c'(x) geändert
            double s = this->max_camber_cl;
            double p = this->max_camber_position;
            double q = this->reflex;
            double k1 = k1(s, p, q)
            double m = m(s, p, q)
            double frack1k2 = (3*(m-p)*(m-p)-m*m*m)/((1-m)*(1-m)*(1-m))
            if(p == 0){
                return 0;
            }
            if(q == 0){
                if(0 <= x && x <= p){
                    return (k1/6)(3*x*x-6*m*x+m*m*(3-m));
                }
                else if(x > p){
                    return -((k1*m*m*m)/6);
                }
            }
            if(q==1){
                if(0 <= x && x <= p){
                    return (k1/6)*(3*(x-m)*(x-m)-frack1k2*(1-m)*(1-m)*(1-m)-m*m*m);
                }
                else if(x > p){
                    return (k1/6)(3*frack1k2*(x-m)*(x-m)-frack1k2*(1-m)*(1-m)*(1-m)-m*m*m);
                }
            }
            else{
                throw ::std::logic_error("error in NACA4Calculator::camberline_derivative: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA5Calculator::normal(double x) const{ //bleibt gleich
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }

        double NACA5Calculator::trailing_edge_thickness_function(double y) const{ //bleibt gleich
            double t = max_profile_thickness;
            if(y < 0){
                throw ::std::logic_error("error in NACA4Calculator::trailing_edge_thickness_function: trailing_edge_thickness must be bigger than 0.");
            }
            return -(y/(5*t)) + (0.2969 - 0.1260 - 0.3516 + 0.2843); 
        }

        Handle(Geom_BSplineCurve) NACA5Calculator::upper_bspline() const{

            NACA4UpperCurve upperCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(upperCurve, umin, umax, degree, tolerance, maxDepth); 
            return converter.Curve();
        }

        Handle(Geom_BSplineCurve) NACA5Calculator::lower_bspline() const{
            NACA4LowerCurve lowerCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(lowerCurve, umin, umax, degree, tolerance, maxDepth);
            return converter.Curve();
        }

        NACA4UpperCurve::NACA5UpperCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double NACA5UpperCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.X();
        }
        double NACA5UpperCurve::valueY(double t)  {
            return 0.0;
        }
        double NACA5UpperCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.Y();
        }

        NACA5LowerCurve::NACA5LowerCurve( NACA5Calculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double NACA5LowerCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.X();
        }

        double NACA5LowerCurve::valueY(double t)  {
            return 0.0;
        }

        double NACA5LowerCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.Y();
        }

} //namespace tigl
**/
